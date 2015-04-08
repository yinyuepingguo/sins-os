#include <sins/boot_params.h>
#include <boot/multiboot.h>
#include <sins/kernel.h>
#include <types.h>

#define CHECK_FLAG(flags,bit)    ((flags) & (1 << (bit)))

boot_params_t boot_params;

void dump_boot_params()
{
	unsigned long i;

	printk("boot device = 0x%x\n", boot_params.boot_device);
	printk("mmap length = %d\n", boot_params.mmap_length);
	for (i = 0; i != boot_params.mmap_length; ++i) {
		printk(" base_addr = 0x%0.8x%0.8x, size = 0x%0.8x%0.8x, type = 0x%x\n",
			(unsigned long)(boot_params.mmap[i].base_addr>>32),
			(unsigned long)(boot_params.mmap[i].base_addr),
			(unsigned long)(boot_params.mmap[i].size>>32),
			(unsigned long)(boot_params.mmap[i].size),
			boot_params.mmap[i].type);
	}
	printk("drives length = %d\n", boot_params.drives_length);
	for (i = 0; i != boot_params.drives_length; ++i) {
		printk("drive[%d]: mode = 0x%08x, %d cylinders,"
			" %d heads, %d sectors\n", i,
			boot_params.drives[i].mode,
			boot_params.drives[i].cylinders,
			boot_params.drives[i].heads,
			boot_params.drives[i].sectors);
	}

}

void parse_boot_params(unsigned long magic, unsigned long addr)
{
	multiboot_info_t *mbi;

	/* multiboot load me */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		BUG("Unknown boot loader!");
	}
	
	mbi = (multiboot_info_t *)addr;
	
	if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
		BUG("multiboot:both bits 4 and 5 are set");
	}
	
	if(CHECK_FLAG(mbi->flags, 1)) {
		boot_params.boot_device = mbi->boot_device;
	}		
	if(CHECK_FLAG(mbi->flags, 6)) {
		multiboot_memory_map_t *mmap;
		unsigned long index;
		unsigned long length;
		
		mmap = (multiboot_memory_map_t *)mbi->mmap_addr;
		length = mbi->mmap_length / sizeof(multiboot_memory_map_t);	
		BUG_ON(length > NR_MMAP_ENTRYS);

		boot_params.mmap_length = length;

		for (index = 0; index != length; ++index) {
			boot_params.mmap[index].base_addr
				= (u64)mmap[index].base_addr_low
					+ (((u64)mmap[index].base_addr_high)<<32);
			boot_params.mmap[index].size
				= (u64)mmap[index].length_low
					+ (((u64)mmap[index].length_high)<<32);
			boot_params.mmap[index].type
				= mmap[index].type;
		}
	}
	if(CHECK_FLAG(mbi->flags, 7)) {
		multiboot_drive_info_t *drive_info;
		unsigned long drive_addr = mbi->drives_addr;
		unsigned long drive_end = mbi->drives_addr + mbi->drives_length;
		unsigned long index;
	
		drive_info = (multiboot_drive_info_t *)mbi->drives_addr;
		for (index = 0; drive_addr < drive_end;	++index) {
			drive_info = (multiboot_drive_info_t *)drive_addr;
			boot_params.drives[index].mode =
						drive_info->drive_mode;
			boot_params.drives[index].cylinders =
						drive_info->drive_cylinders;
			boot_params.drives[index].heads =
						drive_info->drive_heads;
			boot_params.drives[index].sectors =
						drive_info->drive_sectors;
			drive_addr += drive_info->size;

		}
		boot_params.drives_length = index;
				
	}

#ifdef DEBUG		
	dump_boot_params();
#endif
}
