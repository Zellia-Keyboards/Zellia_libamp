#pragma once
#include <stddef.h>
#include <inttypes.h>
#include "keyboard.h"
#include "joystick.h"

#define MAX_ENDPOINTS 8
#define USB_DESC_WORD(x) (uint8_t)((x) & 0xFF), (uint8_t)(((x) >> 8) & 0xFF)

#define KEYBOARD_EPSIZE 8
#define SHARED_EPSIZE 32
#define MOUSE_EPSIZE 16
#define RAW_EPSIZE 0x40
#define CONSOLE_EPSIZE 32
#define MIDI_STREAM_EPSIZE 64
#define CDC_NOTIFICATION_EPSIZE 8
#define CDC_EPSIZE 16

/*
 * Interface indexes
 */
enum usb_interfaces {
    KEYBOARD_INTERFACE = 0,

// It is important that the Raw HID interface is at a constant
// interface number, to support Linux/OSX platforms and chrome.hid
// If Raw HID is enabled, let it be always 1.
    RAW_INTERFACE,
    SHARED_INTERFACE,
    TOTAL_INTERFACES
};

/*
 * Endpoint numbers
 */
enum usb_endpoints {
    __unused_epnum__ = 0, // Endpoint numbering starts at 1
    KEYBOARD_EPNUM,
    RAW_EPNUM,
    SHARED_EPNUM,
    TOTAL_EPNUM,
};

#if TOTAL_EPNUM + 1 > MAX_ENDPOINTS
    #error too many endpoints
#endif

#define ENDPOINT_DIR_IN 0x80
#define ENDPOINT_DIR_OUT 0x00

#define KEYBOARD_EPIN_ADDR  (ENDPOINT_DIR_IN | KEYBOARD_EPNUM)
#define KEYBOARD_EPOUT_ADDR (ENDPOINT_DIR_OUT | KEYBOARD_EPNUM)

#define RAW_EPIN_ADDR  (ENDPOINT_DIR_IN | RAW_EPNUM)
#define RAW_EPOUT_ADDR (ENDPOINT_DIR_OUT | RAW_EPNUM)

#define SHARED_EPIN_ADDR  (ENDPOINT_DIR_IN | SHARED_EPNUM)

// clang-format off

/*
 * HID report descriptors
 */

// Boot keyboard report: 8 modifier bits, 6 keycode slots, 5 LED outputs (boot protocol compatible).
static const uint8_t KeyboardReport[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x06,       // Usage (Keyboard)
    0xA1, 0x01,       // Collection (Application)
        0x05, 0x07,   //   Usage Page (Keyboard/Keypad)
        0x19, 0xE0,   //   Usage Minimum (Keyboard Left Control)
        0x29, 0xE7,   //   Usage Maximum (Keyboard Right GUI)
        0x15, 0x00,   //   Logical Minimum (0)
        0x25, 0x01,   //   Logical Maximum (1)
        0x95, 0x08,   //   Report Count (8 modifiers)
        0x75, 0x01,   //   Report Size (1 bit)
        0x81, 0x02,   //   Input (Data, Variable, Absolute)
        0x95, 0x01,   //   Report Count (1 reserved byte)
        0x75, 0x08,   //   Report Size (8 bits)
        0x81, 0x01,   //   Input (Constant)
        0x05, 0x07,   //   Usage Page (Keyboard/Keypad)
        0x19, 0x00,   //   Usage Minimum (Reserved)
        0x29, 0xFF,   //   Usage Maximum (255)
        0x15, 0x00,   //   Logical Minimum (0)
        0x26, 0xFF, 0x00, // Logical Maximum (255)
        0x95, 0x06,   //   Report Count (6 keycodes)
        0x75, 0x08,   //   Report Size (8 bits)
        0x81, 0x00,   //   Input (Data, Array, Absolute)
        0x05, 0x08,   //   Usage Page (LEDs)
        0x19, 0x01,   //   Usage Minimum (Num Lock)
        0x29, 0x05,   //   Usage Maximum (Kana)
        0x15, 0x00,   //   Logical Minimum (0)
        0x25, 0x01,   //   Logical Maximum (1)
        0x95, 0x05,   //   Report Count (5 LEDs)
        0x75, 0x01,   //   Report Size (1 bit)
        0x91, 0x02,   //   Output (Data, Variable, Absolute, Non-volatile)
        0x95, 0x01,   //   Report Count (1 padding field)
        0x75, 0x03,   //   Report Size (3 bits)
        0x91, 0x01,   //   Output (Constant)
    0xC0              // End Collection
};

// Shared HID report: mouse, joystick, system control, consumer control, and NKRO keyboard over a single endpoint.
static const uint8_t SharedReport[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x02,       // Usage (Mouse)
    0xA1, 0x01,       // Collection (Application)
        0x85, REPORT_ID_MOUSE, //   Report ID (Mouse)
        0x09, 0x01,   //   Usage (Pointer)
        0xA1, 0x00,   //   Collection (Physical)
            0x05, 0x09, //     Usage Page (Buttons)
            0x19, 0x01, //     Usage Minimum (Button 1)
            0x29, 0x08, //     Usage Maximum (Button 8)
            0x15, 0x00, //     Logical Minimum (0)
            0x25, 0x01, //     Logical Maximum (1)
            0x95, 0x08, //     Report Count (8 buttons)
            0x75, 0x01, //     Report Size (1 bit)
            0x81, 0x02, //     Input (Data, Variable, Absolute)
            0x05, 0x01, //     Usage Page (Generic Desktop)
            0x09, 0x30, //     Usage (X)
            0x09, 0x31, //     Usage (Y)
            0x15, 0x81, //     Logical Minimum (-127)
            0x25, 0x7F, //     Logical Maximum (127)
            0x95, 0x02, //     Report Count (X & Y)
            0x75, 0x08, //     Report Size (8 bits)
            0x81, 0x06, //     Input (Data, Variable, Relative)
            0x09, 0x38, //     Usage (Wheel)
            0x15, 0x81, //     Logical Minimum (-127)
            0x25, 0x7F, //     Logical Maximum (127)
            0x95, 0x01, //     Report Count (Wheel)
            0x75, 0x08, //     Report Size (8 bits)
            0x81, 0x06, //     Input (Data, Variable, Relative)
            0x05, 0x0C, //     Usage Page (Consumer)
            0x0A, 0x38, 0x02, //  Usage (AC Pan)
            0x15, 0x81, //     Logical Minimum (-127)
            0x25, 0x7F, //     Logical Maximum (127)
            0x95, 0x01, //     Report Count (AC Pan)
            0x75, 0x08, //     Report Size (8 bits)
            0x81, 0x06, //     Input (Data, Variable, Relative)
        0xC0,          //   End Collection (Physical)
    0xC0,              // End Collection (Mouse)
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x04,       // Usage (Joystick)
    0xA1, 0x01,       // Collection (Application)
        0x85, REPORT_ID_JOYSTICK, // Report ID (Joystick)
        0xA1, 0x00,   //   Collection (Physical)
            0x05, 0x01, //     Usage Page (Generic Desktop)
            0x09, 0x30, //     Usage (X)
            0x09, 0x31, //     Usage (Y)
            0x09, 0x32, //     Usage (Z)
            0x09, 0x33, //     Usage (Rx)
            0x09, 0x34, //     Usage (Ry)
            0x09, 0x35, //     Usage (Rz)
            0x15, (uint8_t)(-JOYSTICK_MAX_VALUE), // Logical Minimum (-max axis)
            0x25, (uint8_t)(JOYSTICK_MAX_VALUE),  // Logical Maximum (+max axis)
            0x95, (uint8_t)JOYSTICK_AXIS_COUNT,   // Report Count (axes)
            0x75, 0x08, //     Report Size (8 bits per axis)
            0x81, 0x02, //     Input (Data, Variable, Absolute)
            0x05, 0x09, //     Usage Page (Buttons)
            0x19, 0x01, //     Usage Minimum (Button 1)
            0x29, (uint8_t)JOYSTICK_BUTTON_COUNT, // Usage Maximum (Button N)
            0x15, 0x00, //     Logical Minimum (0)
            0x25, 0x01, //     Logical Maximum (1)
            0x95, (uint8_t)JOYSTICK_BUTTON_COUNT, // Report Count (buttons)
            0x75, 0x01, //     Report Size (1 bit)
            0x81, 0x02, //     Input (Data, Variable, Absolute)
        0xC0,          //   End Collection (Joystick physical)
    0xC0,              // End Collection (Joystick application)
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x80,       // Usage (System Control)
    0xA1, 0x01,       // Collection (Application)
        0x85, REPORT_ID_SYSTEM, // Report ID (System Control)
        0x19, 0x01,   //   Usage Minimum (Power)
        0x2A, 0xB7, 0x00, // Usage Maximum (System Display LCD Autoscale)
        0x15, 0x01,   //   Logical Minimum (1)
        0x26, 0xB7, 0x00, // Logical Maximum (system last)
        0x95, 0x01,   //   Report Count (1 control)
        0x75, 0x10,   //   Report Size (16 bits)
        0x81, 0x00,   //   Input (Data, Array, Absolute)
    0xC0,              // End Collection (System Control)
    0x05, 0x0C,       // Usage Page (Consumer)
    0x09, 0x01,       // Usage (Consumer Control)
    0xA1, 0x01,       // Collection (Application)
        0x85, REPORT_ID_CONSUMER, // Report ID (Consumer Control)
        0x19, 0x01,   //   Usage Minimum (Consumer Control)
        0x2A, 0xA0, 0x02, // Usage Maximum (AC Desktop Show All Applications)
        0x15, 0x01,   //   Logical Minimum (1)
        0x26, 0xA0, 0x02, // Logical Maximum (last consumer usage)
        0x95, 0x01,   //   Report Count (1 control)
        0x75, 0x10,   //   Report Size (16 bits)
        0x81, 0x00,   //   Input (Data, Array, Absolute)
    0xC0,              // End Collection (Consumer Control)
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x06,       // Usage (Keyboard)
    0xA1, 0x01,       // Collection (Application)
        0x85, REPORT_ID_NKRO, // Report ID (NKRO Keyboard)
        0x05, 0x07,   //   Usage Page (Keyboard/Keypad)
        0x19, 0xE0,   //   Usage Minimum (Keyboard Left Control)
        0x29, 0xE7,   //   Usage Maximum (Keyboard Right GUI)
        0x15, 0x00,   //   Logical Minimum (0)
        0x25, 0x01,   //   Logical Maximum (1)
        0x95, 0x08,   //   Report Count (8 modifiers)
        0x75, 0x01,   //   Report Size (1 bit)
        0x81, 0x02,   //   Input (Data, Variable, Absolute)
        0x05, 0x07,   //   Usage Page (Keyboard/Keypad)
        0x19, 0x00,   //   Usage Minimum (Reserved)
        0x29, (uint8_t)((NKRO_REPORT_BITS * 8) - 1), // Usage Maximum (Last NKRO bit)
        0x15, 0x00,   //   Logical Minimum (0)
        0x25, 0x01,   //   Logical Maximum (1)
        0x95, (uint8_t)(NKRO_REPORT_BITS * 8), // Report Count (NKRO bits)
        0x75, 0x01,   //   Report Size (1 bit)
        0x81, 0x02,   //   Input (Data, Variable, Absolute)
        0x05, 0x08,   //   Usage Page (LEDs)
        0x19, 0x01,   //   Usage Minimum (Num Lock)
        0x29, 0x05,   //   Usage Maximum (Kana)
        0x95, 0x05,   //   Report Count (5 LEDs)
        0x75, 0x01,   //   Report Size (1 bit)
        0x91, 0x02,   //   Output (Data, Variable, Absolute, Non-volatile)
        0x95, 0x01,   //   Report Count (1 padding field)
        0x75, 0x03,   //   Report Size (3 bits)
        0x91, 0x01,   //   Output (Constant)
    0xC0              // End Collection (NKRO Keyboard)
};

static const uint8_t RawReport[] = {
    0x06, 0x60, 0xFF, // Usage Page (Vendor Defined 0xFF60)
    0x09, 0x61,       // Usage (Vendor Defined 0x61)
    0xA1, 0x01,       // Collection (Application)
        0x09, 0x62,   //   Usage (Vendor Defined 0x62) - data to host
        0x15, 0x00,   //   Logical Minimum (0)
        0x26, 0xFF, 0x00, // Logical Maximum (255)
        0x95, 0x40,   //   Report Count (64 bytes / RAW_EPSIZE)
        0x75, 0x08,   //   Report Size (8 bits)
        0x81, 0x02,   //   Input (Data, Variable, Absolute)
        0x09, 0x63,   //   Usage (Vendor Defined 0x63) - data from host
        0x15, 0x00,   //   Logical Minimum (0)
        0x26, 0xFF, 0x00, // Logical Maximum (255)
        0x95, 0x40,   //   Report Count (64 bytes / RAW_EPSIZE)
        0x75, 0x08,   //   Report Size (8 bits)
        0x91, 0x02,   //   Output (Data, Variable, Absolute, Non-volatile)
    0xC0              // End Collection
};

/*
 * Device descriptor
 */
static const uint8_t DeviceDescriptor[] = {
    0x12,             // bLength (sizeof(USB_Descriptor_Device_t))
    0x01,             // bDescriptorType (DTYPE_Device)
    USB_DESC_WORD(0x0200), // bcdUSB (USB 2.00)
    0x00,             // bDeviceClass (USB_CSCP_NoDeviceClass)
    0x00,             // bDeviceSubClass (USB_CSCP_NoDeviceSubclass)
    0x00,             // bDeviceProtocol (USB_CSCP_NoDeviceProtocol)
    0x40,             // bMaxPacketSize0 (FIXED_CONTROL_ENDPOINT_SIZE)
    USB_DESC_WORD(0xFEED), // idVendor (VENDOR_ID)
    USB_DESC_WORD(0x572F), // idProduct (PRODUCT_ID)
    USB_DESC_WORD(0x0002), // bcdDevice (DEVICE_VER)
    0x01,             // iManufacturer
    0x02,             // iProduct
    0x03,             // iSerialNumber
    0x01              // bNumConfigurations (FIXED_NUM_CONFIGURATIONS)
};

/*
 * Configuration descriptors
 */
static const uint8_t ConfigurationDescriptor[] = {
    // Configuration descriptor
    0x09, 0x02,                 // bLength, bDescriptorType (Configuration)
    USB_DESC_WORD(0x005B),      // wTotalLength (sizeof configuration tree)
    0x03,                       // bNumInterfaces (TOTAL_INTERFACES)
    0x01,                       // bConfigurationValue
    0x00,                       // iConfiguration (NO_DESCRIPTOR)
    0xA0,                       // bmAttributes (BUS-powered, remote wakeup)
    0xFA,                       // bMaxPower (500mA)

    // Keyboard interface descriptor
    0x09, 0x04,                 // bLength, bDescriptorType (Interface)
    0x00,                       // bInterfaceNumber (KEYBOARD_INTERFACE)
    0x00,                       // bAlternateSetting
    0x01,                       // bNumEndpoints
    0x03,                       // bInterfaceClass (HID)
    0x01,                       // bInterfaceSubClass (Boot)
    0x01,                       // bInterfaceProtocol (Keyboard)
    0x00,                       // iInterface

    // Keyboard HID descriptor
    0x09, 0x21,                 // bLength, bDescriptorType (HID)
    USB_DESC_WORD(0x0111),      // bcdHID (1.11)
    0x00,                       // bCountryCode
    0x01,                       // bNumDescriptors
    0x22,                       // bDescriptorType (Report)
    USB_DESC_WORD(sizeof(KeyboardReport)), // wDescriptorLength

    // Keyboard IN endpoint descriptor
    0x07, 0x05,                 // bLength, bDescriptorType (Endpoint)
    0x81,                       // bEndpointAddress (KEYBOARD_EPIN_ADDR)
    0x03,                       // bmAttributes (Interrupt)
    USB_DESC_WORD(0x0008),      // wMaxPacketSize (KEYBOARD_EPSIZE)
    0x01,                       // bInterval (USB_POLLING_INTERVAL_MS)

    // Raw HID interface descriptor
    0x09, 0x04,                 // bLength, bDescriptorType (Interface)
    0x01,                       // bInterfaceNumber (RAW_INTERFACE)
    0x00,                       // bAlternateSetting
    0x02,                       // bNumEndpoints
    0x03,                       // bInterfaceClass (HID)
    0x00,                       // bInterfaceSubClass (Non-boot)
    0x00,                       // bInterfaceProtocol (Non-boot)
    0x00,                       // iInterface

    // Raw HID descriptor
    0x09, 0x21,                 // bLength, bDescriptorType (HID)
    USB_DESC_WORD(0x0111),      // bcdHID (1.11)
    0x00,                       // bCountryCode
    0x01,                       // bNumDescriptors
    0x22,                       // bDescriptorType (Report)
    USB_DESC_WORD(sizeof(RawReport)), // wDescriptorLength

    // Raw IN endpoint descriptor
    0x07, 0x05,                 // bLength, bDescriptorType (Endpoint)
    0x82,                       // bEndpointAddress (RAW_EPIN_ADDR)
    0x03,                       // bmAttributes (Interrupt)
    USB_DESC_WORD(0x0040),      // wMaxPacketSize (RAW_EPSIZE)
    0x01,                       // bInterval

    // Raw OUT endpoint descriptor
    0x07, 0x05,                 // bLength, bDescriptorType (Endpoint)
    0x02,                       // bEndpointAddress (RAW_EPOUT_ADDR)
    0x03,                       // bmAttributes (Interrupt)
    USB_DESC_WORD(0x0040),      // wMaxPacketSize (RAW_EPSIZE)
    0x01,                       // bInterval

    // Shared HID interface descriptor
    0x09, 0x04,                 // bLength, bDescriptorType (Interface)
    0x02,                       // bInterfaceNumber (SHARED_INTERFACE)
    0x00,                       // bAlternateSetting
    0x01,                       // bNumEndpoints
    0x03,                       // bInterfaceClass (HID)
    0x00,                       // bInterfaceSubClass (Non-boot)
    0x00,                       // bInterfaceProtocol (Non-boot)
    0x00,                       // iInterface

    // Shared HID descriptor
    0x09, 0x21,                 // bLength, bDescriptorType (HID)
    USB_DESC_WORD(0x0111),      // bcdHID (1.11)
    0x00,                       // bCountryCode
    0x01,                       // bNumDescriptors
    0x22,                       // bDescriptorType (Report)
    USB_DESC_WORD(sizeof(SharedReport)), // wDescriptorLength

    // Shared IN endpoint descriptor
    0x07, 0x05,                 // bLength, bDescriptorType (Endpoint)
    0x83,                       // bEndpointAddress (SHARED_EPIN_ADDR)
    0x03,                       // bmAttributes (Interrupt)
    USB_DESC_WORD(0x0020),      // wMaxPacketSize (SHARED_EPSIZE)
    0x01                        // bInterval (USB_POLLING_INTERVAL_MS)
};

/*
 * String descriptors
 */
static const uint8_t LanguageString[] = {
    0x04, 0x03, 0x09, 0x04 // bLength, bDescriptorType, wLANGID[0] (English US)
};

static const uint8_t ManufacturerString[] = {
    0x0E, 0x03,             // bLength, bDescriptorType
    0x5A, 0x00,             // 'Z'
    0x65, 0x00,             // 'e'
    0x6C, 0x00,             // 'l'
    0x6C, 0x00,             // 'l'
    0x69, 0x00,             // 'i'
    0x61, 0x00              // 'a'
};

static const uint8_t ProductString[] = {
    0x12, 0x03,             // bLength, bDescriptorType
    0x5A, 0x00,             // 'Z'
    0x65, 0x00,             // 'e'
    0x6C, 0x00,             // 'l'
    0x6C, 0x00,             // 'l'
    0x69, 0x00,             // 'i'
    0x61, 0x00,             // 'a'
    0x4B, 0x00,             // 'K'
    0x42, 0x00              // 'B'
};

// clang-format on

// clang-format off
static const uint8_t SerialNumberString[] = {
    0x12, 0x03,             // bLength, bDescriptorType
    0x32, 0x00,             // '2'
    0x30, 0x00,             // '0'
    0x32, 0x00,             // '2'
    0x35, 0x00,             // '5'
    0x31, 0x00,             // '1'
    0x38, 0x00,             // '8'
    0x30, 0x00,             // '0'
    0x38, 0x00              // '8'
};
