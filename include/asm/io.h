#ifndef _IO_H
#define _IO_H

#define outb(value,port) \
	__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))


#define inb(port) ({ \
	unsigned char _v; \
	__asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
	_v; \
	})

#define outb_p(value,port) \
	__asm__ ("outb %%al,%%dx\n" \
		"\tjmp 1f\n" \
		"1:\tjmp 1f\n" \
		"1:"::"a" (value),"d" (port))

#define inb_p(port) ({ \
	unsigned char _v; \
	__asm__ volatile ("inb %%dx,%%al\n" \
		"\tjmp 1f\n" \
		"1:\tjmp 1f\n" \
		"1:":"=a" (_v):"d" (port)); \
	_v; \
	})

#define port_read(port, buf, nr)	\
	__asm__("cld;rep;insw":: "d" (port), "D" (buf), "c" (nr))

#define port_write(port, buf, nr)	\
	__asm__("cld;rep;outsw"::"d" (port), "S" (buf), "c" (nr))

#define CMOS_READ(addr) ({	\
	outb_p(0x80 | addr, 0x70);	\
	inb_p(0x71);	\
	})

#endif
