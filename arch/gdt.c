#include <types.h>
#include <asm/processor.h>
//#include <sins/sched.h>
#include <sins/mm.h>
#include <sins/init.h>

#define INIT_TASK \
/*tss*/ {0,(u32)__init_task_end,0x10,0,0,0,0,	\
	(u32)&kernel_page_dir,\
	0,0,0,0,0,0,0,0, \
	0,0,0x17,0x17,0x17,0x17,0x17,0x17, \
	0,0x80000000, \
	{} \
}

typedef u64 gdt_entry_t;
struct tss_struct tss = INIT_TASK;

#pragma pack(2)
typedef struct gdt_ptr {
	u16 len;
	gdt_entry_t *ptr;
} gdt_ptr_t;
#pragma pack()

gdt_entry_t gdt[NR_GDT_ENTRYS] __attribute__((aligned(16))) = {
	[GDT_KERNEL_CS] = GDT_ENTRY(0xc09b, 0, 0xfffff),
	[GDT_KERNEL_DS] = GDT_ENTRY(0xc093, 0, 0xfffff),
	[GDT_USER_CS]	= GDT_ENTRY(0xc0fb, 0, 0xfffff),
	[GDT_USER_DS]	= GDT_ENTRY(0xc3f3, 0, 0xfffff),
};

static result_t gdt_setup()
{
	gdt_ptr_t gdt_ptr;

	gdt_ptr.len = sizeof(gdt);
	gdt_ptr.ptr = (gdt_entry_t *)gdt;
	lgdt(gdt_ptr);
	flush_selectors(GDT_KERNEL_DS);
	gdt[GDT_TSS] = GDT_ENTRY(0x0089, (unsigned long)&tss, 103);
	ltr(GDT_TSS);
	return SUCCESS;
}

void write_esp0(unsigned long esp0)
{
	tss.esp0 = esp0;
}

arch_initcall(gdt_setup);
