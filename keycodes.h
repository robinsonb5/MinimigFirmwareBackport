// http://wiki.amigaos.net/index.php/Keymap_Library
// http://www.win.tue.nl/~aeb/linux/kbd/scancodes-14.html

// amiga unmapped: 
// 0x0d |
// 0x5a KP-(
// 0x5b KP-)

#define MISS  0xff
#define KEYCODE_MAX (0x6f)

#define OSD               0x0100     // to be used by OSD, not the core itself
#define CAPS_LOCK_TOGGLE  0x0200     // caps lock toggle behaviour
#define NUM_LOCK_TOGGLE   0x0400

// codes >= 0x69 are for OSD only and are not sent to the amiga itself

// keycode translation table
const unsigned short usb2ami[] = {
  MISS,  // 00: NoEvent
  MISS,  // 01: Overrun Error
  MISS,  // 02: POST fail
  MISS,  // 03: ErrorUndefined
  0x20,  // 04: a
  0x35,  // 05: b
  0x33,  // 06: c
  0x22,  // 07: d
  0x12,  // 08: e
  0x23,  // 09: f
  0x24,  // 0a: g
  0x25,  // 0b: h
  0x17,  // 0c: i
  0x26,  // 0d: j
  0x27,  // 0e: k
  0x28,  // 0f: l
  0x37,  // 10: m
  0x36,  // 11: n
  0x18,  // 12: o
  0x19,  // 13: p
  0x10,  // 14: q
  0x13,  // 15: r
  0x21,  // 16: s
  0x14,  // 17: t
  0x16,  // 18: u
  0x34,  // 19: v
  0x11,  // 1a: w
  0x32,  // 1b: x
  0x15,  // 1c: y
  0x31,  // 1d: z
  0x01,  // 1e: 1
  0x02,  // 1f: 2
  0x03,  // 20: 3
  0x04,  // 21: 4
  0x05,  // 22: 5
  0x06,  // 23: 6
  0x07,  // 24: 7
  0x08,  // 25: 8
  0x09,  // 26: 9
  0x0a,  // 27: 0
  0x44,  // 28: Return
  0x45,  // 29: Escape
  0x41,  // 2a: Backspace
  0x42,  // 2b: Tab
  0x40,  // 2c: Space
  0x0b,  // 2d: -
  0x0c,  // 2e: =
  0x1a,  // 2f: [
  0x1b,  // 30: ]
  MISS,  // 31: backslash
  0x2b,  // 32: Europe 1
  0x29,  // 33: ; 
  0x2a,  // 34: '
  0x00,  // 35: `
  0x38,  // 36: ,
  0x39,  // 37: .
  0x3a,  // 38: /
  0x62 | CAPS_LOCK_TOGGLE,  // 39: Caps Lock
  0x50,  // 3a: F1
  0x51,  // 3b: F2
  0x52,  // 3c: F3
  0x53,  // 3d: F4
  0x54,  // 3e: F5
  0x55,  // 3f: F6
  0x56,  // 40: F7
  0x57,  // 41: F8
  0x58,  // 42: F9
  0x59,  // 43: F10
  0x5f,  // 44: F11
  0x69 | OSD,  // 45: F12 (OSD)
  0x6e | OSD,  // 46: Print Screen (OSD)
  0x69 | OSD,  // 47: Scroll Lock (OSD)
  MISS,  // 48: Pause
  MISS,  // 49: Insert
  MISS,  // 4a: Home
  0x6c | OSD,  // 4b: Page Up (OSD)
  0x46,  // 4c: Delete
  MISS,  // 4d: End
  0x6d | OSD,  // 4e: Page Down (OSD)
  0x4e,  // 4f: Right Arrow
  0x4f,  // 50: Left Arrow
  0x4d,  // 51: Down Arrow
  0x4c,  // 52: Up Arrow
  NUM_LOCK_TOGGLE,  // 53: Num Lock
  0x5c,  // 54: KP /
  0x5d,  // 55: KP *
  0x4a,  // 56: KP -
  0x5e,  // 57: KP +
  MISS,  // 58: KP Enter
  0x1d,  // 59: KP 1
  0x1e,  // 5a: KP 2
  0x1f,  // 5b: KP 3
  0x2d,  // 5c: KP 4
  0x2e,  // 5d: KP 5
  0x2f,  // 5e: KP 6
  0x3d,  // 5f: KP 7
  0x3e,  // 60: KP 8
  0x3f,  // 61: KP 9
  0x0f,  // 62: KP 0
  0x3c,  // 63: KP .
  0x30,  // 64: Europe 2
  MISS,  // 65: App
  MISS,  // 66: Power
  MISS,  // 67: KP =
  MISS,  // 68: F13
  MISS,  // 69: F14
  MISS,  // 6a: F15
  MISS,  // 6b: F16
  MISS,  // 6c: F17
  MISS,  // 6d: F18
  MISS,  // 6e: F19
  MISS   // 6f: F20
};
