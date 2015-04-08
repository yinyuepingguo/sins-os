#include "keyboard.h"
#include <sins/kernel.h>
#include <sins/sched.h>
#include <list.h>
#include <sins/init.h>
#include <string.h>
#include <sins/syscalls.h>

#define NR_EVENTS	10	
#define NR_SCAN_CODES	0x80
#define MAP_COLS	3

static WAIT_QUEUE(wait_kbd);
static struct list_head events_list;
static struct list_head free_events;

static struct keyval keymap[NR_SCAN_CODES * MAP_COLS] = {
			/* no shift	shift		EO XX */
	/* 0x00 */	KEYVAL(0)	,KEYVAL(0)	,KEYVAL(0),
			ESC		,ESC		,ESC,
			KEYVAL('1')	,KEYVAL('!')	,KEYVAL(0),
			KEYVAL('2')	,KEYVAL('@')	,KEYVAL(0),
			KEYVAL('3')	,KEYVAL('#')	,KEYVAL(0),
	/* 0x05 */	KEYVAL('4')	,KEYVAL('$')	,KEYVAL(0),
			KEYVAL('5')	,KEYVAL('%')	,KEYVAL(0),
			KEYVAL('6')	,KEYVAL('^')	,KEYVAL(0),
			KEYVAL('7')	,KEYVAL('&')	,KEYVAL(0),
			KEYVAL('8')	,KEYVAL('*')	,KEYVAL(0),
	/* 0x0A */	KEYVAL('9')	,KEYVAL('(')	,KEYVAL(0),
			KEYVAL('0')	,KEYVAL(')')	,KEYVAL(0),
			KEYVAL('-')	,KEYVAL('_')	,KEYVAL(0),
			KEYVAL('=')	,KEYVAL('+')	,KEYVAL(0),
			KEYVAL('\b')	,KEYVAL('\b')	,KEYVAL(0),
	/* 0x0F */	KEYVAL('\t')	,KEYVAL('\t')	,KEYVAL(0),
			KEYVAL('q')	,KEYVAL('Q')	,KEYVAL(0),
			KEYVAL('w')	,KEYVAL('W')	,KEYVAL(0),
			KEYVAL('e')	,KEYVAL('E')	,KEYVAL(0),
			KEYVAL('r')	,KEYVAL('R')	,KEYVAL(0),
	/* 0x14 */	KEYVAL('t')	,KEYVAL('T')	,KEYVAL(0),
			KEYVAL('y')	,KEYVAL('Y')	,KEYVAL(0),
			KEYVAL('u')	,KEYVAL('U')	,KEYVAL(0),
			KEYVAL('i')	,KEYVAL('I')	,KEYVAL(0),
			KEYVAL('o')	,KEYVAL('O')	,KEYVAL(0),
	/* 0x19 */	KEYVAL('p')	,KEYVAL('P')	,KEYVAL(0),
			KEYVAL('[')	,KEYVAL('{')	,KEYVAL(0),
			KEYVAL(']')	,KEYVAL('}')	,KEYVAL(0),
			KEYVAL('\r')	,KEYVAL('\r')	,PAD_ENTER,
			CTRL_L		,CTRL_L		,CTRL_R,
	/* 0x1E*/	KEYVAL('a')	,KEYVAL('A')	,KEYVAL(0),
			KEYVAL('s')	,KEYVAL('S')	,KEYVAL(0),
			KEYVAL('d')	,KEYVAL('D')	,KEYVAL(0),
			KEYVAL('f')	,KEYVAL('F')	,KEYVAL(0),
			KEYVAL('g')	,KEYVAL('G')	,KEYVAL(0),
	/* 0x23 */	KEYVAL('h')	,KEYVAL('H')	,KEYVAL(0),
			KEYVAL('j')	,KEYVAL('J')	,KEYVAL(0),
			KEYVAL('k')	,KEYVAL('K')	,KEYVAL(0),
			KEYVAL('l')	,KEYVAL('L')	,KEYVAL(0),
			KEYVAL(';')	,KEYVAL(':')	,KEYVAL(0),
	/* 0x28 */	KEYVAL('\'')	,KEYVAL('\"')	,KEYVAL(0),
			KEYVAL('`')	,KEYVAL('~')	,KEYVAL(0),
			SHIFT_L		,SHIFT_L	,KEYVAL(0),
			KEYVAL('\\')	,KEYVAL('|')	,KEYVAL(0),
			KEYVAL('z')	,KEYVAL('Z')	,KEYVAL(0),
	/* 0x2D*/	KEYVAL('x')	,KEYVAL('X')	,KEYVAL(0),
			KEYVAL('c')	,KEYVAL('C')	,KEYVAL(0),
			KEYVAL('v')	,KEYVAL('V')	,KEYVAL(0),
			KEYVAL('b')	,KEYVAL('B')	,KEYVAL(0),
			KEYVAL('n')	,KEYVAL('N')	,KEYVAL(0),
	/* 0x32 */	KEYVAL('m')	,KEYVAL('M')	,KEYVAL(0),
			KEYVAL(',')	,KEYVAL('<')	,KEYVAL(0),
			KEYVAL('.')	,KEYVAL('>')	,KEYVAL(0),
			KEYVAL('/')	,KEYVAL('?')	,KEYVAL(0),
			SHIFT_R		,SHIFT_R	,KEYVAL(0),
	/* 0x37*/	KEYVAL('*')	,KEYVAL('*')	,KEYVAL(0),
			ALT_L		,ALT_L		,ALT_R,
			KEYVAL(' ')	,KEYVAL(' ')	,KEYVAL(0),
			CAPS_LOCK	,CAPS_LOCK	,KEYVAL(0),
			F1		,F1		,KEYVAL(0),
	/* 0x3C */	F2		,F2		,KEYVAL(0),
			F3		,F3		,KEYVAL(0),
			F4		,F4		,KEYVAL(0),
			F5		,F5		,KEYVAL(0),
			F6		,F6		,KEYVAL(0),
	/* 0x41 */	F7		,F7		,KEYVAL(0),
			F8		,F8		,KEYVAL(0),
			F9		,F9		,KEYVAL(0),
			F10		,F10		,KEYVAL(0),
			NUM_LOCK	,NUM_LOCK	,KEYVAL(0),
	/* 0x46 */	SCROLL_LOCK	,SCROLL_LOCK	,KEYVAL(0),
			PAD_HOME	,KEYVAL('7')	,HOME,
			PAD_UP		,KEYVAL('8')	,UP,
			PAD_PAGEUP	,KEYVAL('9')	,PAGEUP,
			PAD_MINUS	,KEYVAL('-')	,KEYVAL(0),
	/* 0x4B */	PAD_LEFT	,KEYVAL('4')	,LEFT,
			PAD_MID		,KEYVAL('5')	,KEYVAL(0),
			PAD_RIGHT	,KEYVAL('6')	,RIGHT,
			PAD_PLUS	,KEYVAL('+')	,KEYVAL(0),
			PAD_END		,KEYVAL('1')	,END,
	/* 0x50 */	PAD_DOWN	,KEYVAL('2')	,DOWN,
			PAD_PAGEDOWN	,KEYVAL('3')	,PAGEDOWN,
			PAD_INS		,KEYVAL('0')	,INSERT,
			PAD_DOT		,KEYVAL('.')	,DELETE,
			KEYVAL(0)	,KEYVAL(0)	,KEYVAL(0),
	/* 0x55 */	KEYVAL(0)	,KEYVAL(0)	,KEYVAL(0),
			KEYVAL(0)	,KEYVAL(0)	,KEYVAL(0),
			F11		,F11		,KEYVAL(0),
			F12		,F12		,KEYVAL(0),
			KEYVAL(0)	,KEYVAL(0)	,KEYVAL(0),
	/* 0x5A */	KEYVAL(0)	,KEYVAL(0)	,KEYVAL(0),
			KEYVAL(0)	,KEYVAL(0)	,GUI_L,
			KEYVAL(0)	,KEYVAL(0)	,GUI_R,
			KEYVAL(0)	,KEYVAL(0)	,APPS,
			KEYVAL(0)	,KEYVAL(0)	,KEYVAL(0),
	/* 0x6F */	KEYVAL(0)	,KEYVAL(0)	,KEYVAL(0),
	/* 0x7F ... */
};

static void keyboard_handler()
{
	static unsigned char shift = 0;
	static unsigned char ctrl = 0;
	struct kbd_event *event;
	unsigned char code;
	unsigned char make;
	struct keyval *kv;

	/* read and process */
	code = inb(0x60);

	if (code == 0xE1) {
		return;
	} else if (code == 0xE0) {
		return;
	} else {
		kv = &keymap[(code&0x7F)*MAP_COLS];

		if (code & 0x80)
			make = 0;
		else
			make = 1;

		if (kv->val[0] & 0x80) {
			/* special key */
			switch (kv->val[0]&0x7F) {
			case KT_ESC:
				break;
			case KT_SHIFT:
				shift = !shift;
				break;
			case KT_ALT:
				break;
			case KT_CTRL:
				ctrl = !ctrl;
				break;
			}
			return;
		} else {
			if (make) {
				/* normal key */
				kv += shift;
			} else
				return;
		}
	}

	if (list_empty(&free_events)) {
		BUG_ON(list_empty(&events_list));
		return;
	}

	event = list_first_entry(&free_events, struct kbd_event, list);
	memcpy(&event->kv, kv, sizeof(*kv));
	/* do process bottom */

	// add to events_list's tail
	list_move_tail(&event->list, &events_list);
	wake_up(&wait_kbd);
}

/* read one event from events_list */
void keyboard_read(struct kbd_event *to)
{
	unsigned long flags;
	struct kbd_event *event;
	
	wait_event(&wait_kbd, !list_empty(&events_list));
	irq_save(flags);
	event = list_first_entry(&events_list, struct kbd_event, list);
	list_move_tail(&event->list, &free_events);
	memcpy(to, event, sizeof(struct kbd_event));	
	irq_restore(flags);
}

static result_t keyboard_setup() 
{
	unsigned long i;
	struct kbd_event *event;
	struct list_head *pos, *tmp;
	result_t res;

	INIT_LIST_HEAD(&events_list);
	INIT_LIST_HEAD(&free_events);
	for (i = 0; i != NR_EVENTS; ++i) {
		event = kmalloc(sizeof(struct kbd_event));
		memset(event, 0, sizeof(struct kbd_event));
		if (event)
			list_add(&event->list, &free_events);
		else
			return -ENOMEM;
	}

	res = request_irq(IRQ_KEYBOARD, keyboard_handler, "keyboard");
	if (FAILED(res)) {
		list_for_each_safe(pos, tmp, &free_events) {
			event = list_first_entry(pos, struct kbd_event, list);
			list_del(&event->list);
			kfree(event);
		}
		return res;
	}
	enable_irq(IRQ_KEYBOARD);
	inb(0x60);
	return SUCCESS;
}

device_initcall(keyboard_setup);
