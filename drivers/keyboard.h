#ifndef _SINS_KEYBOARD_H
#define _SINS_KEYBOARD_H

#include <list.h>
#include <types.h>

#define KEYVAL(...)	{{__VA_ARGS__}}

#define KT_ESC		0
#define KT_SHIFT	1
#define KT_ALT		2
#define KT_CTRL		3

/* special key rule:0x80:8+type<<4, (sub_type:4<<4)+func_num:4 */
#define ESC		KEYVAL(0x80 + KT_ESC, 0)
#define SHIFT_L		KEYVAL(0x80 + KT_SHIFT, 0)
#define SHIFT_R		KEYVAL(0x80 + KT_SHIFT, 1)
#define ALT_L		KEYVAL(0x80 + KT_ALT, 0)
#define ALT_R		KEYVAL(0x80 + KT_ALT, 1)
#define CTRL_L		KEYVAL(0x80 + KT_CTRL, 0)
#define CTRL_R		KEYVAL(0x80 + KT_CTRL, 1)
#define F1		KEYVAL(0x80+4, 0)
#define F2		KEYVAL(0x80+4, 1)
#define F3		KEYVAL(0x80+4, 2)
#define F4		KEYVAL(0x80+4, 3)
#define F5		KEYVAL(0x80+4, 4)
#define F6		KEYVAL(0x80+4, 5)
#define F7		KEYVAL(0x80+4, 6)
#define F8		KEYVAL(0x80+4, 7)
#define F9		KEYVAL(0x80+4, 8)
#define F10		KEYVAL(0x80+4, 9)
#define F11		KEYVAL(0x80+4, 10)
#define F12		KEYVAL(0x80+4, 11)
#define CAPS_LOCK	KEYVAL(0x80+5, 0)
#define NUM_LOCK	KEYVAL(0x80+5, 1)
#define SCROLL_LOCK	KEYVAL(0x80+5, 2)
#define PAD_ENTER	KEYVAL(0x80+6, 0)
#define PAD_HOME	KEYVAL(0x80+6, 1)
#define PAD_UP		KEYVAL(0x80+6, 2)
#define PAD_PAGEUP	KEYVAL(0x80+6, 3)
#define PAD_MINUS	KEYVAL(0x80+6, 4)
#define PAD_LEFT	KEYVAL(0x80+6, 5)
#define PAD_MID		KEYVAL(0x80+6, 6)
#define PAD_RIGHT	KEYVAL(0x80+6, 7)
#define PAD_PLUS	KEYVAL(0x80+6, 8)
#define PAD_END		KEYVAL(0x80+6, 9)
#define PAD_DOWN	KEYVAL(0x80+6, 10)
#define PAD_PAGEDOWN	KEYVAL(0x80+6, 11)
#define PAD_INS		KEYVAL(0x80+6, 12)
#define PAD_DOT		KEYVAL(0x80+6, 13)
#define HOME		KEYVAL(0x80+7, 0)
#define UP		KEYVAL(0x80+7, 1)
#define PAGEUP		KEYVAL(0x80+7, 2)
#define LEFT		KEYVAL(0x80+7, 3)
#define RIGHT		KEYVAL(0x80+7, 4)
#define END		KEYVAL(0x80+7, 5)
#define DOWN		KEYVAL(0x80+7, 6)
#define PAGEDOWN	KEYVAL(0x80+7, 7)
#define INSERT		KEYVAL(0x80+7, 8)
#define DELETE		KEYVAL(0x80+7, 9)
#define GUI_L		KEYVAL(0x80+8, 0)
#define GUI_R		KEYVAL(0x80+8, 1)
#define APPS		KEYVAL(0x80+9, 0)


struct keyval {
	byte val[4];
};

struct kbd_event {
	struct list_head list;
	struct keyval kv;
};

extern void keyboard_read(struct kbd_event *to);

#endif
