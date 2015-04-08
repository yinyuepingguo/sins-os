OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)
SECTIONS
{
 . = 0x100000;
 __load = . + 0xC0000000;
 .boot ALIGN(0x1000) : AT(0x100000)
 {
  __boot = .;
   *(.boot)
  __boot_end = .;
 }
 . = ALIGN(0x1000);
 . += 0xC0000000;
 __init_task = .;
 __init_task_end = __init_task + 0x1000;
 . = __init_task_end;
 .initcall ALIGN(0x1000) : AT(ADDR(.initcall) - 0xC0000000)
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
 .text ALIGN(0x1000) : AT(ADDR(.text) - 0xC0000000)
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
