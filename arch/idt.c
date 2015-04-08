#include <sins/processor.h>
#include <sins/kernel.h>
#include <sins/init.h>
#include <sins/irq.h>
#include <types.h>

typedef u64 idt_entry_t;

#pragma pack(2)
struct idt_ptr
{
	u16 len;
	idt_entry_t *ptr;
};
#pragma pack()

idt_entry_t idt[NR_IDT_ENTRYS] __attribute ((aligned(16)));

asmlinkage void divide_error();
asmlinkage void debug();
asmlinkage void nmi();
asmlinkage void int3();
asmlinkage void overflow();
asmlinkage void bounds();
asmlinkage void invalid_op();
asmlinkage void device_not_available();
asmlinkage void double_fault();
asmlinkage void coprocessor_segment_overrun();
asmlinkage void invalid_tss();
asmlinkage void segment_not_present();
asmlinkage void stack_segment();
asmlinkage void general_protection();
asmlinkage void page_fault();
asmlinkage void coprocessor_error();
asmlinkage void reserved();
asmlinkage void system_call();

extern void irq_start();

static result_t idt_setup()
{
	unsigned long i;
	struct idt_ptr ptr = {sizeof(idt), (idt_entry_t *)&idt};
	
	lidt(ptr);
	set_trap_gate(0, &divide_error);
	set_trap_gate(1, &debug);
	set_trap_gate(2, &nmi);
	set_system_gate(3, &int3);
	set_system_gate(4, &overflow);
	set_system_gate(5, &bounds);
	set_trap_gate(6, &invalid_op);
	set_trap_gate(7, &device_not_available);
	set_trap_gate(8, &double_fault);
	set_trap_gate(9, &coprocessor_segment_overrun);
	set_trap_gate(10, &invalid_tss);
	set_trap_gate(11, &segment_not_present);
	set_trap_gate(12, &stack_segment);
	set_trap_gate(13, general_protection);
	set_trap_gate(14, &page_fault);
	set_trap_gate(15, &reserved);
	set_trap_gate(16, &coprocessor_error);
	
	for (i = 17; i != NR_IDT_ENTRYS; ++i)
		set_trap_gate(i, &reserved);

	set_system_gate(0x80, &system_call);

	for (i = NR_EXTERNAL_VECTORS;
		i != NR_EXTERNAL_VECTORS + NR_IRQS; ++i)
		set_intr_gate(i, (char *)&irq_start + 
			8*(i-NR_EXTERNAL_VECTORS));

	return SUCCESS;
}

arch_initcall(idt_setup);
