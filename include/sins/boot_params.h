#ifndef _BOOT_PARAMS_H
#define _BOOT_PARAMS_H

#include <types.h>


#define NR_MMAP_ENTRYS 30 
#define NR_DRIVES 5

#define MMAP_USABLE 0x1

struct memory_map
{
	u64 base_addr;
	u64 size;
	unsigned long type;
};

struct drive_info
{
	u8 mode;
	u16 cylinders;
	u8 heads;
	u8 sectors;
};

typedef struct boot_params 
{
	unsigned long boot_device;
	unsigned long mmap_length;
	unsigned long drives_length;
	struct memory_map mmap[NR_MMAP_ENTRYS];
	struct drive_info drives[NR_DRIVES];
} boot_params_t;


extern boot_params_t boot_params;

extern void parse_boot_params(unsigned long magic, unsigned long addr);
extern void dump_boot_params();

#endif
