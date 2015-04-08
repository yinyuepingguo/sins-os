OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

#define PAGE_SIZE 0x1000

SECTIONS
{
	. = 0x00000000;

	.text ALIGN(PAGE_SIZE) : AT(0x00000000)
	{
		*(.text) 
	}
	.data : 
	{
		 *(.data) 
		 *(.bss) 
	}
}
