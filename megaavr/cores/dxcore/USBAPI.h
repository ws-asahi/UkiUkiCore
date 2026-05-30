/* Copyright (c) 2026 Yusuke Shimizu (Workshop Asahi). SPDX-License-Identifier: MIT */
/* DxCore-style passthrough wrapper. The actual API lives at api/USBAPI.h
 * (vendored from arduino/ArduinoCore-API). This lets the bundled HID library
 * (and user libraries that follow the upstream Arduino convention) write
 * `#include "USBAPI.h"` and have it resolve from the core include path. */
#pragma once
#include "api/USBAPI.h"
#include "usb_ep_types.h"
