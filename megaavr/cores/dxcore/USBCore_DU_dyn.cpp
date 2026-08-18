/**
 * USBCore_DU_dyn.cpp - PluggableUSB dynamic-endpoint machinery for the
 *                      AVR DU native-USB stack.
 *
 * Split out of USBCore_DU.cpp so that CDC-only sketches do not carry the
 * dynamic-EP staging buffers (s_dyn_ep_buf & friends, ~330 B of RAM).
 *
 * LINK MECHANICS: nothing in the always-linked stack references this
 * translation unit. It is extracted from core.a only when a PluggableUSB
 * module (HID, MIDIUSB, ...) references epBuffer() / PluggableUSB_ /
 * USB_Send() etc. Once extracted, the strong usbcore_* definitions here
 * override the weak no-op defaults in USBCore_DU.cpp (a strong definition
 * in an extracted archive member always wins over a weak one; what an
 * archive member can NOT do is force its own extraction - the references
 * from the PluggableUSB module do that for us, unlike the variant-keep
 * case which needs -Wl,-u).
 *
 * All code below is moved verbatim from USBCore_DU.cpp; behavior for
 * sketches that use PluggableUSB modules is unchanged.
 */
#include <avr/io.h>
#if defined(USB0)

#include <Arduino.h>
#include <string.h>
#include <avr/pgmspace.h>

extern "C" {
  #include "usb_core.h"
  #include "usb_descriptors.h"
}
#include "USBCore_DU.h"
#include "usb_ep_types.h"

#include "api/USBAPI.h"
#include "api/PluggableUSB.h"

/* Constants shared with USBCore_DU.cpp (kept in lockstep). */
#define USBCORE_NUM_EP        USB_NUM_EP        /* EP0..EP{N-1}; follows the
                                                 * USB_EP_SLOTS knob (8/16) in
                                                 * usb_descriptors.h, so the
                                                 * bridge and CTRLA.MAXEP can
                                                 * never disagree.           */
#define USBCORE_DYN_EP_BASE   4                 /* first plugged EP          */
#define USBCORE_DYN_EP_COUNT  (USBCORE_NUM_EP - USBCORE_DYN_EP_BASE)
#define USBCORE_CDC_LAST_EP   3                 /* CDC owns EP1..EP3       */
#define USBCORE_CDC_NUM_IF    2                 /* CDC owns IF0..IF1       */
#define USBCORE_DYN_EP_BUF    USB_EP_SIZE       /* per-dynamic-EP staging  */

/* ============================================================
 *  EP-type storage queried by PluggableUSB::plug()
 *
 *  The framework writes `node->endpointType[i]` as `unsigned int`
 *  to *(unsigned int *)epBuffer(lastEp), so we hand back a pointer
 *  to a uint16_t per slot. CDC slots (1..3) are pre-filled with
 *  EP_TYPE_INTERRUPT_IN / EP_TYPE_BULK_OUT / EP_TYPE_BULK_IN so a
 *  scan over the whole table can apply CDC and dynamic EPs uniformly
 *  if ever needed.
 * ============================================================ */
static uint8_t s_ep_types[USBCORE_NUM_EP] = {
    EP_TYPE_CONTROL,         /* EP0 control            */
    EP_TYPE_INTERRUPT_IN,    /* EP1 CDC notify         */
    EP_TYPE_BULK_OUT,        /* EP2 CDC data RX        */
    EP_TYPE_BULK_IN,         /* EP3 CDC data TX        */
    0, 0, 0, 0               /* EP4..EP7 dynamic       */
};

void *epBuffer(unsigned int n) {
    if (n >= USBCORE_NUM_EP) return NULL;
    return &s_ep_types[n];
}

/* ============================================================
 *  PluggableUSB_ constructor  (the API only declares it)
 *
 *  CDC occupies IF0..1 and EP1..3, so plugged modules start at
 *  lastIf = 2, lastEp = 4. totalEP is the per-class member that
 *  plug() compares against (lastEp + numEndpoints > totalEP rejects).
 * ============================================================ */
PluggableUSB_::PluggableUSB_() :
    lastIf(USBCORE_CDC_NUM_IF),
    lastEp(USBCORE_CDC_LAST_EP + 1),
    rootNode(NULL),
    totalEP(USBCORE_NUM_EP - 1)   /* highest usable EP number (7 or 15) */
{
}

/* ============================================================
 *  USBSetup conversion (our packed usb_setup_t -> framework type)
 * ============================================================ */
static inline USBSetup as_usbsetup(const usb_setup_t *s) {
    USBSetup u;
    u.bmRequestType = s->bmRequestType;
    u.bRequest      = s->bRequest;
    u.wValueL       = (uint8_t)(s->wValue & 0xFF);
    u.wValueH       = (uint8_t)((s->wValue >> 8) & 0xFF);
    u.wIndex        = s->wIndex;
    u.wLength       = s->wLength;
    return u;
}

/* ============================================================
 *  Control-OUT data stage staging (plugged / HID host->device)
 *
 *  Flow (see usb_standard.c usb_handle_class_request):
 *    SETUP(host->device, wLength>0, non-CDC IF)
 *      -> usbcore_ctrl_out_begin() saves the SETUP and returns the length
 *         to arm; usb_standard.c arms EP0 OUT at usbcore_ctrl_out_buf().
 *    EP0 OUT data lands -> handle_ep0_out_complete()
 *      -> usb_class_data_out_complete() -> usbcore_ctrl_out_dispatch()
 *         re-runs the owning module's setup(), which reads the staged
 *         bytes synchronously via USB_RecvControl().
 *  Only one control transfer is in flight at a time, so a single set of
 *  static state is sufficient (and is mutually exclusive with the CDC
 *  SET_LINE_CODING path).
 * ============================================================ */
static usb_setup_t s_ctrl_out_setup;             /* SETUP to re-dispatch        */
static uint8_t     s_ctrl_out_buf[USB_EP_SIZE];  /* EP0 OUT landing buffer      */
static uint16_t    s_ctrl_out_len     = 0;       /* bytes the host will send    */
static uint16_t    s_ctrl_out_pos     = 0;       /* USB_RecvControl read cursor */
static bool        s_ctrl_out_pending = false;

uint16_t usbcore_ctrl_out_begin(const usb_setup_t *s) {
    s_ctrl_out_setup = *s;
    uint16_t n = s->wLength;
    if (n > sizeof(s_ctrl_out_buf)) n = sizeof(s_ctrl_out_buf); /* single-packet cap */
    s_ctrl_out_len     = n;
    s_ctrl_out_pos     = 0;
    s_ctrl_out_pending = true;
    return n;
}

uint8_t *usbcore_ctrl_out_buf(void)     { return s_ctrl_out_buf; }
bool     usbcore_ctrl_out_pending(void) { return s_ctrl_out_pending; }

void usbcore_ctrl_out_dispatch(void) {
    /* EP0 OUT has filled s_ctrl_out_buf. Hand it to the owning module: its
     * setup() calls USB_RecvControl() below, which now returns the staged
     * bytes. The status-stage ZLP is issued by handle_ep0_out_complete()
     * after we return. */
    s_ctrl_out_pending = false;
    s_ctrl_out_pos     = 0;
    USBSetup u = as_usbsetup(&s_ctrl_out_setup);
    PluggableUSB().setup(u);
}

/* SET_REPORT / HID feature report: return bytes staged by the EP0 OUT data
 * stage (see above). Sequential reads, like the Arduino AVR core. */
int USB_RecvControl(void* d, int len) {
    if (len <= 0) return 0;
    uint16_t avail = s_ctrl_out_len - s_ctrl_out_pos;
    uint16_t n = ((uint16_t)len > avail) ? avail : (uint16_t)len;
    memcpy(d, &s_ctrl_out_buf[s_ctrl_out_pos], n);
    s_ctrl_out_pos += n;
    return (int)n;
}
int USB_RecvControlLong(void* d, int len) { return USB_RecvControl(d, len); }

/* ============================================================
 *  Dynamic-EP staging buffers and USBAPI
 * ============================================================ */
/* Staging buffers only allocated for plugged EPs (EP_BASE..EP_BASE+COUNT-1).
 * Indexed by (ep - USBCORE_DYN_EP_BASE).  EP0..EP3 are owned by CDC and use
 * usb_core's g_ep0..g_ep3 buffers directly, so no staging is needed here. */
static uint8_t  s_dyn_ep_buf[USBCORE_DYN_EP_COUNT][USBCORE_DYN_EP_BUF];
static uint8_t  s_dyn_ep_pos[USBCORE_DYN_EP_COUNT];

/* Per dynamic OUT (host->device) endpoint receive ring. The TRNCOMPL ISR
 * copies each received packet out of the endpoint buffer (s_dyn_ep_buf[idx],
 * which is the OUT EP's DATAPTR) into this ring and immediately re-arms the
 * endpoint; USB_Recv()/USB_Available() consume it from main context. Single
 * producer (ISR) / single consumer (main) with uint8_t head/tail = lock-free
 * on this 8-bit core (same pattern as the CDC RX ring). */
#define USBCORE_DYN_RX_RING   64    /* >= one max packet; tune to RAM budget */
static uint8_t          s_dyn_rx_ring[USBCORE_DYN_EP_COUNT][USBCORE_DYN_RX_RING];
static volatile uint8_t s_dyn_rx_head[USBCORE_DYN_EP_COUNT];  /* written by ISR  */
static volatile uint8_t s_dyn_rx_tail[USBCORE_DYN_EP_COUNT];  /* written by main */

/* Forward declaration: dyn_ep_release() is defined further down, but
 * USB_Flush() below needs it to push a staged short packet. */
static int dyn_ep_release(uint8_t ep);

uint8_t USB_Available(uint8_t ep) {
    ep &= 0x07;
    if (ep < USBCORE_DYN_EP_BASE || ep >= USBCORE_NUM_EP) return 0;
    uint8_t i = ep - USBCORE_DYN_EP_BASE;
    int16_t n = (int16_t)s_dyn_rx_head[i] - (int16_t)s_dyn_rx_tail[i];
    if (n < 0) n += USBCORE_DYN_RX_RING;
    return (uint8_t)n;
}

/* TBD when an OUT EP is plugged */
uint8_t USB_SendSpace(uint8_t ep)      {
    ep &= 0x07;
    if (ep < USBCORE_DYN_EP_BASE || ep >= USBCORE_NUM_EP) return 0;
    return USBCORE_DYN_EP_BUF - s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE];
}

int USB_Recv(uint8_t ep, void* data, int len) {
    ep &= 0x07;
    if (ep < USBCORE_DYN_EP_BASE || ep >= USBCORE_NUM_EP) return -1;
    if (len <= 0) return 0;
    uint8_t  i   = ep - USBCORE_DYN_EP_BASE;
    uint8_t *dst = (uint8_t *)data;
    int got = 0;
    while (got < len && s_dyn_rx_tail[i] != s_dyn_rx_head[i]) {
        dst[got++] = s_dyn_rx_ring[i][s_dyn_rx_tail[i]];
        s_dyn_rx_tail[i] = (uint8_t)((s_dyn_rx_tail[i] + 1) % USBCORE_DYN_RX_RING);
    }
    return got;
}
int USB_Recv(uint8_t ep) {
    uint8_t b;
    return (USB_Recv(ep, &b, 1) == 1) ? (int)b : -1;
}

/* Push whatever is staged on a dynamic IN endpoint out to the bus. The
 * Arduino USBAPI lets a library stage payload with USB_Send() (without
 * TRANSFER_RELEASE) and then emit a sub-maxpacket packet with USB_Flush();
 * MIDIUSB sends every event this way. HID never does - it ORs
 * TRANSFER_RELEASE into each report - so USB_Flush() is first exercised here. */
void    USB_Flush(uint8_t ep) {
    ep &= 0x07;
    if (ep < USBCORE_DYN_EP_BASE || ep >= USBCORE_NUM_EP) return;
    if (s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE] > 0) {
        dyn_ep_release(ep);
    }
}

/* Issue an IN packet on a dynamic EP from its staging buffer.
 * Blocking - waits for HW to be idle, sets CNT/clears BUSNAK, then waits
 * for TRNCOMPL. Times out (returns) if the host never IN-polls; HID will
 * just retry next report. */
static int dyn_ep_release(uint8_t ep) {
    if (ep < 4 || ep >= USBCORE_NUM_EP) return -1;

    /* Wait for HW idle (BUSNAK = 1 means no packet armed). */
    uint16_t tmo = 0;
    while (!(g_ep_table.EP[ep].IN.STATUS & USB_BUSNAK_bm)) {
        if (++tmo == 0) return -1;
    }

    /* Arm the packet. */
    g_ep_table.EP[ep].IN.DATAPTR = (uint16_t)s_dyn_ep_buf[ep - USBCORE_DYN_EP_BASE];
    g_ep_table.EP[ep].IN.CNT     = s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE];
    /* Activate: clear BUSNAK (+ stale flags) via INCLR, but PRESERVE TOGGLE so
     * the hardware keeps DATA0/DATA1 in sync with the host (see usb_core.c). */
    while (USB0.INTFLAGSB & USB_RMWBUSY_bm) {}
    USB0.STATUS[ep].INCLR = USB_UNFOVF_bm | USB_TRNCOMPL_bm | USB_STALLED_bm | USB_BUSNAK_bm;

    /* Wait for completion. */
    tmo = 0;
    while (!(g_ep_table.EP[ep].IN.STATUS & USB_TRNCOMPL_bm)) {
        if (++tmo == 0) { s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE] = 0; return -1; }
    }
    /* Clear TRNCOMPL only; the hardware re-sets BUSNAK after the IN completes.
     * Never write STATUS directly here - it would clear TOGGLE. */
    while (USB0.INTFLAGSB & USB_RMWBUSY_bm) {}
    USB0.STATUS[ep].INCLR = USB_TRNCOMPL_bm;

    int sent = s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE];
    s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE] = 0;
    return sent;
}

int USB_Send(uint8_t ep_with_flags, const void* data, int len) {
    uint8_t ep    = ep_with_flags & 0x07;
    uint8_t flags = ep_with_flags & 0xF8;
    if (ep < 4 || ep >= USBCORE_NUM_EP) return -1;
    if (len < 0) return -1;

    /* Append payload to the staging buffer. */
    int total = 0;
    const uint8_t *src = (const uint8_t *)data;
    while (len > 0) {
        int room = USBCORE_DYN_EP_BUF - s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE];
        if (room == 0) {
            /* Flush a full packet, then keep going. */
            int r = dyn_ep_release(ep);
            if (r < 0) return -1;
            room = USBCORE_DYN_EP_BUF;
        }
        int n = (len > room) ? room : len;
        if (flags & TRANSFER_PGM) {
            memcpy_P(&s_dyn_ep_buf[ep - USBCORE_DYN_EP_BASE][s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE]], src, n);
        } else {
            memcpy(&s_dyn_ep_buf[ep - USBCORE_DYN_EP_BASE][s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE]], src, n);
        }
        s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE] += n;
        src   += n;
        len   -= n;
        total += n;
    }

    if (flags & TRANSFER_RELEASE) {
        int r = dyn_ep_release(ep);
        if (r < 0) return -1;
    }
    return total;
}

/* ============================================================
 *  Dispatch helpers (called from usb_standard.c)
 * ============================================================ */
bool usbcore_try_plugged_setup(const usb_setup_t *s) {
    USBSetup u = as_usbsetup(s);
    return PluggableUSB().setup(u);
}

bool usbcore_try_plugged_get_descriptor(const usb_setup_t *s) {
    USBSetup u = as_usbsetup(s);
    return PluggableUSB().getDescriptor(u) > 0;
}

/* ============================================================
 *  SET_CONFIGURATION : program dynamic EPs from epBuffer types
 *  (CDC EPs are programmed inline in handle_set_configuration.)
 * ============================================================ */
void usbcore_init_plugged_endpoints(void) {
    for (uint8_t ep = 4; ep < USBCORE_NUM_EP; ep++) {
        uint8_t t = s_ep_types[ep];
        if (t == 0) continue;       /* slot unused */

        uint8_t  dir_in  = (t & 0x80) ? 1 : 0;
        /* DU TYPE: control / iso / bulkint (bulk and interrupt are the same
         * register value; the descriptor type bits 1..0 distinguish them). */
        uint8_t  type    = (t & 0x03);
        uint8_t  ctrl_tp = (type == 0) ? USB_TYPE_CONTROL_gc
                         : (type == 1) ? USB_TYPE_ISO_gc
                         :               USB_TYPE_BULKINT_gc;

        /* Per-EP buffer size selector. We use 64-byte buffers for all
         * dynamic EPs; HID/Keyboard/Mouse only send a handful per report. */
        uint8_t  bufsz   = USB_BUFSIZE_DEFAULT_BUF64_gc;

        s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE] = 0;
        if (dir_in) {
            g_ep_table.EP[ep].IN.CTRL    = ctrl_tp | bufsz;
            g_ep_table.EP[ep].IN.DATAPTR = (uint16_t)s_dyn_ep_buf[ep - USBCORE_DYN_EP_BASE];
            g_ep_table.EP[ep].IN.CNT     = 0;
            g_ep_table.EP[ep].IN.STATUS  = USB_BUSNAK_bm;    /* idle, owned by SW */
        } else {
            g_ep_table.EP[ep].OUT.CTRL    = ctrl_tp | bufsz;
            g_ep_table.EP[ep].OUT.DATAPTR = (uint16_t)s_dyn_ep_buf[ep - USBCORE_DYN_EP_BASE];
            g_ep_table.EP[ep].OUT.CNT     = 0;
            g_ep_table.EP[ep].OUT.STATUS  = 0;               /* armed for RX */
        }
    }
}

/* Service dynamic OUT (host->device) endpoints. Called from the TRNCOMPL ISR.
 * Dynamic IN reports still complete synchronously inside USB_Send(); only OUT
 * endpoints need servicing here. For each plugged OUT EP that completed a
 * transaction: ack TRNCOMPL, copy the packet into its RX ring, then re-arm.
 * All STATUS access is OUTCLR (atomic, preserves the DATA0/DATA1 TOGGLE) -
 * never a direct STATUS write. */
void usbcore_service_dynamic_ep_trncompl(void) {
    for (uint8_t ep = USBCORE_DYN_EP_BASE; ep < USBCORE_NUM_EP; ep++) {
        uint8_t t = s_ep_types[ep];
        if (t == 0 || (t & 0x80)) continue;            /* unused or IN endpoint */
        if (!(g_ep_table.EP[ep].OUT.STATUS & USB_TRNCOMPL_bm)) continue;

        uint8_t i = ep - USBCORE_DYN_EP_BASE;
        while (USB0.INTFLAGSB & USB_RMWBUSY_bm) {}
        USB0.STATUS[ep].OUTCLR = USB_TRNCOMPL_bm;      /* ack the transaction */

        uint16_t cnt = g_ep_table.EP[ep].OUT.CNT;
        if (cnt > USBCORE_DYN_EP_BUF) cnt = USBCORE_DYN_EP_BUF;
        for (uint16_t k = 0; k < cnt; k++) {
            uint8_t next = (uint8_t)((s_dyn_rx_head[i] + 1) % USBCORE_DYN_RX_RING);
            if (next == s_dyn_rx_tail[i]) break;       /* ring full: drop the rest */
            s_dyn_rx_ring[i][s_dyn_rx_head[i]] = s_dyn_ep_buf[i][k];
            s_dyn_rx_head[i] = next;
        }

        g_ep_table.EP[ep].OUT.CNT = 0;
        while (USB0.INTFLAGSB & USB_RMWBUSY_bm) {}
        USB0.STATUS[ep].OUTCLR = USB_BUSNAK_bm;        /* re-arm for next packet */
    }
}

/* ============================================================
 *  Config-descriptor hook: append plugged modules' interfaces.
 *  Strong override of the weak no-op in USBCore_DU.cpp; called by
 *  usbcore_build_config_descriptor().
 * ============================================================ */
void usbcore_plugged_get_interfaces(uint8_t *ifCount) {
    PluggableUSB().getInterface(ifCount);
}

#endif /* USB0 */
