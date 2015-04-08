OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

#define PAGE_SIZE 0x1000
#define PAGE_OFFSET 0xC0000000
#define PHYS_OFFSET 0x100000

SECTIONS
{
	. = PHYS_OFFSET;

	__load = . + PAGE_OFFSET;

	.boot ALIGN(PAGE_SIZE) : AT(PHYS_OFFSET) 
	{
		__boot = .;
		 *(.boot) 
		__boot_end = .;
	}

	. = ALIGN(PAGE_SIZE);
	. += PAGE_OFFSET;

	__init_task = .;
	__init_task_end = __init_task + PAGE_SIZE;	

	. = __init_task_end;
	
	.initcall ALIGN(PAGE_SIZE) : AT(ADDR(.initcall) - PAGE_OFFSET)
	{
		__initcall = .;

		__initcall_pure = .;
		*(.initcall0.init)
		__initcall_pure_end = .;

		__initcall_arch = .;
		*(.initcall1.init)
		__initcall_arch_end = .;

		__initcall_archdone = .;
		*(.initcall2.init)
		__initcall_archdone_end = .;

		__initcall_subsys = .;
		*(.initcall3.init)
		__initcall_subsys_end = .;
		
		__initcall_fs = .;
		*(.initcall4.init)
		__initcall_fs_end = .;

		__initcall_device = .;
		*(.initcall5.init)
		__initcall_device_end = .;

		__initcall_end = .;
	}
	
	
	.text ALIGN(PAGE_SIZE) : AT(ADDR(.text) - PAGE_OFFSET)
	{
		__text = .;
		*(.text) 
		__text_end = .;
	}

	.data : 
	{
		__data = .;	
		 *(.data) 
		__data_end = .;
	}

	.bss : 
	{
		__bss = .;
		 *(.bss) 
		__bss_end = .;
	}

	__load_end = .;
	__end = .; 
}
