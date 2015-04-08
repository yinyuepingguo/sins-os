#include <sins/console.h>
#include <sins/kernel.h>
#include <asm/io.h>
#include <string.h>
#include <ctype.h>
#include <list.h>
#include <stddef.h>
#include <sins/init.h>
#include <sins/irq.h>

#define PAGE_SIZE 0x1000
#define PAGE_OFFSET 0xC0000000

#define SCREEN_START (0xB8000 + PAGE_OFFSET)
#define SCREEN_END (0xC0000 + PAGE_OFFSET)
#define LINES 25
#define COLUMNS 80

static unsigned long origin = SCREEN_START;
static unsigned long scr_end = SCREEN_START + LINES*COLUMNS*2;
static unsigned long pos;
static unsigned long x, y;
static unsigned long lines = LINES, columns = COLUMNS;

static void gotoxy(unsigned int new_x, unsigned int new_y)
{
	if (new_x >= columns || new_y >= lines)
		return;
	
	x = new_x;
	y = new_y;
	pos = origin + ((y*COLUMNS + x) << 1);
}

static void set_origin()
{
	outb_p(12, 0x3d4);
	outb_p(0xff & ((origin - SCREEN_START) >> 9), 0x3d5);
	outb_p(13, 0x3d4);
	outb_p(0xff & ((origin - SCREEN_START) >> 1), 0x3d5);
}

static void set_cursor()
{
        outb_p(14,0x3d4);
        outb_p(0xff&((pos-SCREEN_START)>>9),0x3d5);
        outb_p(15,0x3d4);
        outb_p(0xff&((pos-SCREEN_START)>>1),0x3d5);
}

static void clear()
{
	unsigned long i;
	
	for (i = SCREEN_START; i != SCREEN_END; i+=2)
		*(u16 *)i = 0x0720;
}

static void scroll_up()
{
	unsigned long i;

	if ((scr_end + (COLUMNS<<1)) < SCREEN_END) {
		origin += (COLUMNS<<1);
		scr_end += (COLUMNS<<1);
	}
	else {
		memmove((void *)SCREEN_START, (void *)origin, COLUMNS*(LINES-1)*2);
		origin = SCREEN_START;
		scr_end = SCREEN_START + COLUMNS*LINES*2;
	}
	for (i = scr_end - COLUMNS*2; i != scr_end; i+=2)
		*(u16 *)i = 0x0720;
	set_origin();
}
static void __used scroll_down()
{
	if (origin != SCREEN_START)
	{
		origin -= (COLUMNS<<1);
		scr_end -= (COLUMNS<<1);
	}
	else
	{
		origin = SCREEN_END - COLUMNS*LINES*2;
		scr_end = SCREEN_END;
	}
	set_origin();
}

static void __used del_char()
{
	if (x) {
		pos -= 2;
		x--;
		*(u16*)pos = 0x0720;
	}
}

static void putchar(unsigned short ch)
{
	ch = 0x0700 + ch;
	*(u16 *)pos = ch;

	if (++x >= COLUMNS) {
		y++;x = 0;
	}

	if (y >= LINES) {
		scroll_up();
		y--;
	}
	gotoxy(x, y);
}

static void insert_line()
{
	if (++y >= LINES) {
		scroll_up();
		y--;
	}	
	gotoxy(x, y);
}

static void cntrl_left()
{
	x = 0;
	gotoxy(x, y);
}

static void cntrl_vtable()
{
	unsigned long old_x = x;
	x = (x + 7)/8 * 8;
	if (x == old_x)
		x += 8;
	if (x >= COLUMNS) {
		y++;
		x = 0;
	}
	if (y >= LINES) {
		scroll_up();
		y--;
	}
	gotoxy(x, y);
}

static void cntrl_back()
{
	if (x) {
		x--;	
	} else if (y) {
		y--;
		x = COLUMNS-1;
	}
	gotoxy(x, y);
}

static void cntrl_beep()
{
	*((u16 *)(scr_end - 2)) = 0x0201;
}

void con_write(const char *buf, size_t count)
{
	unsigned char ch;
	unsigned long flags;

	while (count--) {
		ch = *buf;
		irq_save(flags);
		if (isprint(ch))
			putchar(ch);
		else if (iscntrl(ch)) {
			switch (ch) {
				case '\n':
					/* act as \r\n */
					insert_line();	
					cntrl_left();
					break;
				case '\t':
					cntrl_vtable();
					break;
				case '\r':
					cntrl_left();
					break;
				case '\b':
					cntrl_back();
					break;
				case '\a':
					cntrl_beep();
					break;
			}
		}
		irq_restore(flags);
		buf++;
	}		
	set_cursor();
}

struct console early_console = {
	.write = con_write,
	.flags = CON_BOOT | CON_ENABLE
};

static result_t __init con_init()
{
	clear();
	gotoxy(0, 0);
	set_cursor();
	return register_console(&early_console);
}

pure_initcall(con_init);
