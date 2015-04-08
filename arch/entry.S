#include <linkage.h>
#include <sins/irq.h>
#include <sins/processor.h>
#include <sins/syscalls.h>

EBX	= 0x00
ECX	= 0x04
EDX	= 0x08
EDI	= 0x0C
ESI	= 0x10
EBP	= 0x14
EAX	= 0x18
GS	= 0x1C
FS	= 0x20
ES	= 0x24
DS	= 0x28
ORI_EAX	= 0x2C
ERR_CODE= 0x30
EIP	= 0x34
CS	= 0x38
EFLAGS	= 0x3C
ESP	= 0x40
SS	= 0x44



irq_index = 0
.rept NR_IRQS
	.align 8
	.if irq_index == 0
		ENTRY(irq_start)
	.else
		ENTRY(irq_start##irq_index)
	.endif
	pushl $irq_index
	jmp do_irq_start
	irq_index = irq_index + 1
.endr


ENTRY(do_irq_start)
	pushl $do_irq
	SAVE_ALL
	movl ERR_CODE(%esp), %eax
	movl ORI_EAX(%esp), %ebx
	pushl %eax	// irq_index
	movl $(GDT_KERNEL_DS << 3), %eax
	movw %ax, %es
	movw %ax, %ds
	movw %ax, %fs
	movw %ax, %gs

	mov $0x20, %al
	outb %al, $0x20
	nop
	nop
	outb %al, $0xA0 //EOI

	call *%ebx	//do_irq
	addl $4, %esp

	mov %esp, %eax	/* pt_regs's address */
	push %eax
	call do_signal
	add $0x4, %esp

	RESTORE_ALL
	iret	

#define ENTRY_NO_ERROR_CODE(name)	ENTRY(name)	\
	pushl $0x0;	\
	pushl $do_##name;	\
	jmp error_code

#define ENTRY_ERROR_CODE(name)		ENTRY(name)	\
	pushl $do_##name;	\
	jmp error_code

/* Here is general implement. */
	.align 4
error_code:
	SAVE_ALL
	movl ERR_CODE(%esp), %eax
	movl ORI_EAX(%esp), %ebx
	pushl %eax	// push error_code
	lea 0x4(%esp), %eax
	pushl %eax	// push pt_regs 
	movl $(GDT_KERNEL_DS<<3), %eax
	movw %ax, %es
	movw %ax, %ds
	movw %ax, %fs
	movw %ax, %gs
	call *%ebx	//call function
	addl $8, %esp
	RESTORE_ALL
	iret

ENTRY_NO_ERROR_CODE(divide_error);
ENTRY_NO_ERROR_CODE(debug);
ENTRY_NO_ERROR_CODE(nmi);
ENTRY_NO_ERROR_CODE(int3);
ENTRY_NO_ERROR_CODE(overflow);
ENTRY_NO_ERROR_CODE(bounds);
ENTRY_NO_ERROR_CODE(invalid_op);
ENTRY_NO_ERROR_CODE(device_not_available);
ENTRY_NO_ERROR_CODE(coprocessor_segment_overrun);
ENTRY_NO_ERROR_CODE(reserved);
ENTRY_NO_ERROR_CODE(coprocessor_error);
ENTRY_ERROR_CODE(double_fault);
ENTRY_ERROR_CODE(invalid_tss);
ENTRY_ERROR_CODE(segment_not_present);
ENTRY_ERROR_CODE(stack_segment);
ENTRY_ERROR_CODE(general_protection);
ENTRY_ERROR_CODE(page_fault);

ENTRY(kthread_wrapper)
	RESTORE_ALL	
	iret
ENTRY(_kthread_wrapper)
	push %ecx
	push %edx
	call *%ecx
	pop %edx
	pop %ecx
	push %eax
	call sys_exit

ENTRY(arch_move_to_user_mode)
	pop %eax /* pop return address(unused) */
	pop %edi /* eip */
	pop %esi /* esp */
	pop %ebx /* argc */
	pop %eax /* argv */
	movl $(GDT_USER_DS<<3+3), %ecx
	pushl %ecx	/* ss */
	pushl %esi	/* esp */
	movl $X86_EFLAGS_USER_INIT, %edx
	pushl %edx	/* eflags */
	pushl $(GDT_USER_CS<<3+3)	/* cs */
	pushl %edi	/* eip */
	movw %cx, %ds
	movw %cx, %es
	movw %cx, %fs
	movw %cx, %gs
	xorl %ecx, %ecx
	xorl %edx, %edx
	xorl %esi, %esi
	xorl %edi, %edi
	xorl %ebp, %ebp
	iret

ENTRY(system_call)
	cmpl $NR_SYSTEM_CALLS -1, %eax
	ja bad_system_call
	sub $0x08, %esp	/* to fix SAVE_ALL and RESTORE_ALL */
	SAVE_ALL
	movl $(GDT_KERNEL_DS<<3), %eax /* point to kernel data */
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movl EAX(%esp), %eax
	call *syscall_table(, %eax, 4)
	movl %eax, EAX(%esp)

	/* we don't directly calucate as we want to check it */
	mov %esp, %eax	/* pt_regs's address */
	push %eax
	call do_signal
	add $0x4, %esp

	RESTORE_ALL
	iret

ENTRY(bad_system_call)
	push %eax
	call bad_syscall
	add $0x4, %esp
	ret

