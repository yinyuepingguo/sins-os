#include <sins/init.h>
#include <sins/kernel.h>
#include <asm/io.h>

static result_t i8259_init()
{
	/* init master */
	outb_p(0x11, 0x20);
	outb_p(0x20, 0x21);
	outb_p(0x04, 0x21);
	/* 0x03 AEOI */
	outb_p(0x01, 0x21); /* NORMAL EOI */
	
	/* init slave */
	outb_p(0x11, 0xA0);
	outb_p(0x28, 0xA1);
	outb_p(0x02, 0xA1);
	outb_p(0x01, 0xA1); /* NORMAL EOI */

	/* mask all */
	outb_p(0xFF, 0x21);
	outb_p(0xFF, 0xA1);
	
	return SUCCESS;
}

arch_initcall(i8259_init);
