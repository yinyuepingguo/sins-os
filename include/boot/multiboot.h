#ifndef _MULTIBOOT_H
#define _MULTIBOOT_H

/* Multiboot header 的魔数。 */
#define MULTIBOOT_HEADER_MAGIC          0x1BADB002

/* Multiboot header 的标志。 */
#define MULTIBOOT_HEADER_FLAGS         0x00000003

/* Multiboot 兼容的引导程序传递来的魔数。 */
#define MULTIBOOT_BOOTLOADER_MAGIC      0x2BADB002

#ifndef ASM_FILE

/* Multiboot header。 */
typedef struct multiboot_header
{
	unsigned long magic;
	unsigned long flags;
	unsigned long checksum;
	unsigned long header_addr;
	unsigned long load_addr;
	unsigned long load_end_addr;
	unsigned long bss_end_addr;
	unsigned long entry_addr;
} multiboot_header_t;

/* a.out 符号表。 */
typedef struct multiboot_aout_symbol_table
{
	unsigned long tabsize;
	unsigned long strsize;
	unsigned long addr;
	unsigned long reserved;
} multiboot_aout_symbol_table_t;

/* ELF 的 section header table。 */
typedef struct multiboot_elf_section_header_table
{
	unsigned long num;
	unsigned long size;
	unsigned long addr;
	unsigned long shndx;
} multiboot_elf_section_header_table_t;

/* Multiboot 信息。 */
typedef struct multiboot_info
{
	unsigned long flags;
	unsigned long mem_lower;
	unsigned long mem_upper;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long mods_count;
	unsigned long mods_addr;
	union
	{
	  multiboot_aout_symbol_table_t aout_sym;
	  multiboot_elf_section_header_table_t elf_sec;
	} u;
	unsigned long mmap_length;
	unsigned long mmap_addr;
	unsigned long drives_length;
	unsigned long drives_addr;
} multiboot_info_t;

/* 内存分布。小心，偏移量 0 是 base_addr_low 而不是 size 。 */
typedef struct multiboot_memory_map
{
	unsigned long size;
	unsigned long base_addr_low;
	unsigned long base_addr_high;
	unsigned long length_low;
	unsigned long length_high;
	unsigned long type;
} multiboot_memory_map_t;

typedef struct multiboot_drive_info
{
	unsigned long size;
	unsigned char drive_number;
	unsigned char drive_mode;
	unsigned short drive_cylinders;
	unsigned char drive_heads;
	unsigned char drive_sectors;	
	unsigned short drive_ports[0];
} multiboot_drive_info_t;

#endif /* ! ASM_FILE */
#endif 
