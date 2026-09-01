/**
 * usb_core.h
 * Public USB stack API
 *
 * Uses official USB_EP_TABLE_t and bit definitions from ioavr64du32.h.
 * Architecture: fully interrupt-driven (USB0_BUSEVENT + USB0_TRNCOMPL).
 * A sketch needs no usbPoll() calls; usbPoll() is a no-op kept only for
 * source compatibility.
 */
#ifndef USB_CORE_H
#define USB_CORE_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include "usb_descriptors.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * USB SETUP packet structure (USB 2.0 spec 9.3, Table 9-2)
 * ============================================================ */
typedef struct __attribute__((packed)) {
    uint8_t  bmRequestType;
    uint8_t  bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} usb_setup_t;

/* ============================================================
 * Control transfer state machine
 * ============================================================ */
typedef enum {
    CTRL_IDLE,
    CTRL_DATA_IN_STAGE,
    CTRL_DATA_OUT_STAGE,
    CTRL_STATUS_IN_STAGE,
    CTRL_STATUS_OUT_STAGE,
    CTRL_STATUS_PENDING_ADDR
} ctrl_state_t;

/* ============================================================
 * Public API
 * ============================================================ */
void usbInit(void);
void usbAttach(void);
void usbDetach(void);
bool usbIsConfigured(void);

/* Stage `n` bytes from a PROGMEM source into the config-descriptor accumulator.
 * Used by usb_standard.c to ship device/string descriptors that live in flash. */
void usbcore_acc_reset(void);
void usbcore_acc_load_P(const uint8_t *src_pgm, uint16_t n);
const uint8_t *usbcore_acc_buf(void);
uint16_t       usbcore_acc_len(void);

/* No-op kept for source compatibility. The stack is interrupt-driven, so
 * calling this from loop() is neither required nor has any effect. */
void usbPoll(void);

/* ============================================================
 * Internal globals shared between USB modules
 *
 * g_ep_table holds ONLY the endpoint descriptor table:
 *   EP[USB_NUM_EP] : endpoint descriptors (USB_NUM_EP x 16 bytes)
 *
 * The official USB_EP_TABLE_t (ioavr64du32.h) additionally reserves
 * FIFO[32] (used only when CTRLA.FIFOEN=1) and FRAMENUM (used only
 * when CTRLA.STFRNUM=1). This stack enables neither, so by default a
 * trimmed table sized by the USB_EP_SLOTS knob is used instead, saving
 * 162 B at 8 EPs / 34 B at 16 EPs. The hardware only ever accesses
 * EPPTR .. EPPTR+(MAXEP+1)*16-1 with FIFOEN=STFRNUM=0 (DS40002548A
 * Figure 28-9), and MAXEP derives from the same knob, so it can never
 * read past this struct.
 *
 * The omitted areas remain available behind two INTERNAL build macros
 * (deliberately not exposed in the boards.txt menus), each matching
 * its CTRLA enable bit 1:1 per Figure 28-9:
 *
 *   -DUSB_EP_TABLE_FIFO     reserve the transaction-complete FIFO,
 *                           (MAXEP+1)x2 bytes at negative offsets
 *                           below EPPTR (prerequisite for FIFOEN=1)
 *   -DUSB_EP_TABLE_FRAMENUM reserve FRAMENUM right after the EP
 *                           table (prerequisite for STFRNUM=1)
 *
 * Defining a macro only reserves the RAM area; actually using the
 * feature additionally requires setting CTRLA.FIFOEN / CTRLA.STFRNUM
 * and the corresponding handling code, which this stack does not
 * contain today. EPPTR = &g_ep_table.EP[0] is correct in every
 * layout combination (with the FIFO present it points past it,
 * exactly as the memory map requires).
 *
 * DS40002548A 28.5.7: EPPTR[0] must be zero -> keep aligned(2).
 * (EP[0]'s offset is 0 or (MAXEP+1)*2, both even, so the attribute
 * on the object suffices in every layout.)
 * Access endpoints as: g_ep_table.EP[n].OUT.STATUS, .CTRL, .CNT, etc.
 * ============================================================ */
typedef struct {
#if defined(USB_EP_TABLE_FIFO)
    register8_t FIFO[USB_NUM_EP * 2];  /* active when CTRLA.FIFOEN=1  */
#endif
    USB_EP_PAIR_t EP[USB_NUM_EP];      /* USB_EP_PAIR_t from ioavr64du32.h */
#if defined(USB_EP_TABLE_FRAMENUM)
    _WORDREGISTER(FRAMENUM);           /* active when CTRLA.STFRNUM=1 */
#endif
} usb_ep_table_t;
extern usb_ep_table_t g_ep_table;

extern uint8_t  g_ep0_setup_buf[8];
extern uint8_t  g_ep0_data_buf[USB_EP0_SIZE];
extern uint8_t  g_ep1_in_buf[USB_EP1_SIZE];   /* CDC notify           */
extern uint8_t  g_ep2_out_buf[USB_EP2_SIZE];  /* CDC data RX          */
extern uint8_t  g_ep3_in_buf[USB_EP3_SIZE];   /* CDC data TX          */

extern ctrl_state_t g_ctrl_state;
extern uint8_t  g_pending_address;
extern volatile uint8_t  g_current_configuration;
extern uint8_t  g_remote_wakeup_enabled;

/* Diagnostic counters */

/* diagnostic snapshot globals (captured at the end of the TRNCOMPL ISR) */

/* ============================================================
 * Internal helpers (shared between USB modules)
 * ============================================================ */
void ep0_start_data_in(const uint8_t *data, uint16_t len, uint16_t host_requested);
/* PROGMEM-source variant: streams the data stage from flash in <=64 B
 * chunks via g_ep0_data_buf (no RAM staging buffer required). */
void ep0_start_data_in_P(const uint8_t *data_pgm, uint16_t len, uint16_t host_requested);
void ep0_start_data_out(uint8_t *buffer, uint16_t len);
void ep0_send_zlp(void);
void ep0_stall(void);

/* RMWBUSY wait - call before any STATUS[n].xxxCLR/xxxSET write */
static inline void usb_rmw_wait(void) {
    while (USB0.INTFLAGSB & USB_RMWBUSY_bm) {}
}

#ifdef __cplusplus
}
#endif

#endif /* USB_CORE_H */
