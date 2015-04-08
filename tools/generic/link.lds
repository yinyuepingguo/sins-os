OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)
SECTIONS
{
 . = 0x00000000;
 .text ALIGN(0x1000) : AT(0x00000000)
 {
  *(.text)
 }
 .data :
 {
   *(.data)
   *(.bss)
 }
}
