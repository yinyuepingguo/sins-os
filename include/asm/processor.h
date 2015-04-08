#ifndef _ASM_PROCESSOR_H
#define _ASM_PROCESSOR_H

/*
 * EFLAGS bits
 */
#define X86_EFLAGS_CF	0x00000001 /* Carry Flag */
#define X86_EFLAGS_PF	0x00000004 /* Parity Flag */
#define X86_EFLAGS_AF	0x00000010 /* Auxillary carry Flag */
#define X86_EFLAGS_ZF	0x00000040 /* Zero Flag */
#define X86_EFLAGS_SF	0x00000080 /* Sign Flag */
#define X86_EFLAGS_TF	0x00000100 /* Trap Flag */
#define X86_EFLAGS_IF	0x00000200 /* Interrupt Flag */
#define X86_EFLAGS_DF	0x00000400 /* Direction Flag */
#define X86_EFLAGS_OF	0x00000800 /* Overflow Flag */
#define X86_EFLAGS_IOPL	0x00003000 /* IOPL mask */
#define X86_EFLAGS_NT	0x00004000 /* Nested Task */
#define X86_EFLAGS_RF	0x00010000 /* Resume Flag */
#define X86_EFLAGS_VM	0x00020000 /* Virtual Mode */
#define X86_EFLAGS_AC	0x00040000 /* Alignment Check */
#define X86_EFLAGS_VIF	0x00080000 /* Virtual Interrupt Flag */
#define X86_EFLAGS_VIP	0x00100000 /* Virtual Interrupt Pending */
#define X86_EFLAGS_ID	0x00200000 /* CPUID detection flag */

#define X86_EFLAGS_USER_INIT	(X86_EFLAGS_IF | 0x02)

/*
 * Basic CPU control in CR0
 */
#define X86_CR0_PE	0x00000001 /* Protection Enable */
#define X86_CR0_MP	0x00000002 /* Monitor Coprocessor */
#define X86_CR0_EM	0x00000004 /* Emulation */
#define X86_CR0_TS	0x00000008 /* Task Switched */
#define X86_CR0_ET	0x00000010 /* Extension Type */
#define X86_CR0_NE	0x00000020 /* Numeric Error */
#define X86_CR0_WP	0x00010000 /* Write Protect */
#define X86_CR0_AM	0x00040000 /* Alignment Mask */
#define X86_CR0_NW	0x20000000 /* Not Write-through */
#define X86_CR0_CD	0x40000000 /* Cache Disable */
#define X86_CR0_PG	0x80000000 /* Paging */

/*
 * Paging options in CR3
 */
#define X86_CR3_PWT	0x00000008 /* Page Write Through */
#define X86_CR3_PCD	0x00000010 /* Page Cache Disable */

/*
 * Intel CPU features in CR4
 */
#define X86_CR4_VME	0x00000001 /* enable vm86 extensions */
#define X86_CR4_PVI	0x00000002 /* virtual interrupts flag enable */
#define X86_CR4_TSD	0x00000004 /* disable time stamp at ipl 3 */
#define X86_CR4_DE	0x00000008 /* enable debugging extensions */
#define X86_CR4_PSE	0x00000010 /* enable page size extensions */
#define X86_CR4_PAE	0x00000020 /* enable physical address extensions */
#define X86_CR4_MCE	0x00000040 /* Machine check enable */
#define X86_CR4_PGE	0x00000080 /* enable global pages */
#define X86_CR4_PCE	0x00000100 /* enable performance counters at ipl 3 */
#define X86_CR4_OSFXSR	0x00000200 /* enable fast FPU save and restore */
#define X86_CR4_OSXMMEXCPT 0x00000400 /* enable unmasked SSE exceptions */
#define X86_CR4_VMXE	0x00002000 /* enable VMX virtualization */
#define X86_CR4_OSXSAVE 0x00040000 /* enable xsave and xrestore */


#define LOCK_PREFIX "lock;"

#define arch_halt()	asm volatile("hlt"::)
#define arch_nop()	asm volatile("nop"::)

#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))

#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)

#define _set_seg_desc(gate_addr,type,dpl,base,limit) {\
	*(gate_addr) = ((base) & 0xff000000) | \
		(((base) & 0x00ff0000)>>16) | \
		((limit) & 0xf0000) | \
		((dpl)<<13) | \
		(0x00408000) | \
		((type)<<8); \
	*((gate_addr)+1) = (((base) & 0x0000ffff)<<16) | \
		((limit) & 0x0ffff); }

#define _set_tssldt_desc(n,addr,type) 	\
	__asm__ volatile("movw $104,%1\n\t" \
	"movw %%ax,%2\n\t" \
	"rorl $16,%%eax\n\t" \
	"movb %%al,%3\n\t" \
	"movb $" type ",%4\n\t" \
	"movb $0x00,%5\n\t" \
	"movb %%ah,%6\n\t" \
	"rorl $16,%%eax" \
	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
	)

#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),addr,"0x89")
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),addr,"0x82")

#define GDT_ENTRY(attr,base,limit)          \
        ((((base)&0xff000000ULL)<<(56-24))| \
        (((attr)&0xf0ffULL)<<40)|           \
        (((base)&0x00ff0000ULL)<<(32-16))|  \
        (((base)&0x0000ffffULL)<<16)|       \
        ((limit)&0x0000ffffULL)|            \
        (((limit)&0x000f0000ULL)<<(48-16)))

#define lgdt(ptr)       \
        asm volatile("lgdtl %0\n"::"rm" (ptr))

#define lidt(ptr)       \
        asm volatile("lidtl %0\n"::"rm" (ptr))

#define flush_selectors(ds)       \
        asm volatile("push $0x08\n\t"	\
		"push $1f\n\t"	\
		"retf\n\t"	\
		"1:\tmovl %0, %%eax\n\t"       \
                "movw %%ax, %%ds\n\t"   \
                "movw %%ax, %%es\n\t"   \
                "movw %%ax, %%ss\n\t"   \
                "movw %%ax, %%fs\n\t"   \
                "movw %%ax, %%gs\n\t"::"i" ((ds)<<3):"eax")


#define ltr(n) __asm__("ltr %%ax"::"a" (((n)<<3)&(0xF8)))

#define NR_GDT_ENTRYS	32
#define GDT_KERNEL_CS	1
#define GDT_KERNEL_DS	2
#define GDT_USER_CS	3
#define GDT_USER_DS	4
#define GDT_TSS		5

#define NR_IDT_ENTRYS 	256

#define ARCH_NR_EXTERNAL_VECTORS 0x20

#define SAVE_ALL	\
	push %ds;	\
	push %es;	\
	push %fs;	\
	push %gs;	\
	push %eax;	\
	push %ebp;	\
	push %esi;	\
	push %edi;	\
	push %edx;	\
	push %ecx;	\
	push %ebx;	\
	
#define RESTORE_ALL	\
	pop %ebx;	\
	pop %ecx;	\
	pop %edx;	\
	pop %edi;	\
	pop %esi;	\
	pop %ebp;	\
	pop %eax;	\
	pop %gs;	\
	pop %fs;	\
	pop %es;	\
	pop %ds;	\
	add $0x8, %esp


#ifndef __ASSEMBLY__

#include <types.h>
#include <linkage.h>

struct arch_pt_regs {
	u32 ebx, ecx, edx;
	u32 edi, esi, ebp;
	u32 eax;
	u32 gs, fs, es, ds; 
	u32 ori_eax;
	u32 err_code;
	u32 eip, cs;
	u32 eflags;
	u32 esp, ss;
};

struct i387_struct {
        u32    cwd;
        u32    swd;
        u32    twd;
        u32    fip;
        u32    fcs;
        u32    foo;
        u32    fos;
        u32    st_space[20];   /* 8*10 bytes for each FP-reg = 80 bytes */
};

struct tss_struct {
        u32    back_link;      /* 16 high bits zero */
        u32    esp0;
        u32    ss0;            /* 16 high bits zero */
        u32    esp1;
        u32    ss1;            /* 16 high bits zero */
        u32    esp2;
        u32    ss2;            /* 16 high bits zero */
        u32    cr3;
        u32    eip;
        u32    eflags;
        u32    eax,ecx,edx,ebx;
        u32    esp;
        u32    ebp;
        u32    esi;
        u32    edi;
        u32    es;             /* 16 high bits zero */
        u32    cs;             /* 16 high bits zero */
        u32    ss;             /* 16 high bits zero */
        u32    ds;             /* 16 high bits zero */
        u32    fs;             /* 16 high bits zero */
        u32    gs;             /* 16 high bits zero */
        u32    ldt;            /* 16 high bits zero */
        u32    trace_bitmap;   /* bits: trace 0, bitmap 16-31 */
        struct i387_struct i387;
};

extern asmlinkage void arch_move_to_user_mode(unsigned long eip, unsigned long esp,
	unsigned long argc, const char *argv[]);
extern void write_esp0(unsigned long esp0);

static inline int arch_from_kernel(struct arch_pt_regs *regs)
{
	return (regs->cs & 3) != 3;
}

extern result_t arch_check_user_regs(struct arch_pt_regs *regs);

#endif /* not def __ASSEMBLY__ */

#endif
