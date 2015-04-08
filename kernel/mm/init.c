#include <sins/mm.h>
#include <sins/boot_params.h>
#include <stddef.h>
#include <sins/kernel.h>
#include <string.h>
#include <div64.h>
#include <math.h>
#include <list.h>

mem_info_t mem_info;
mem_zone_t zones_list[NR_ZONES];
pgd_t *kernel_page_dir;
pgt_t *__kernel_pgt_base;
pgt_t *__kernel_pgt_end;
mem_map_t *mmap;
mem_zone_t *fast_zone_entry[NR_ZONE_TYPES];

static void init_fast_zone_entry()
{
	unsigned long i;
	mem_zone_t *iter = zones_list;
	mem_zone_t *next;
	
	for (i = 0; i != NR_ZONE_TYPES; ++i) {
		fast_zone_entry[i] = NULL;
	}

	while(iter) {
		next = iter->next;

		if (iter->base_addr < ZONE_DMA_LIMIT) {
			if (fast_zone_entry[ZONE_DMA] == NULL) {
				fast_zone_entry[ZONE_DMA] = iter;
			}
			if(iter->next != NULL
				&& iter->next->base_addr >= ZONE_DMA_LIMIT) {
				iter->next = NULL;
			}
		}
		else if (iter->base_addr >=ZONE_DMA_LIMIT
			&& iter->base_addr < ZONE_NORMAL_LIMIT) {
			if (fast_zone_entry[ZONE_NORMAL] == NULL) {
				fast_zone_entry[ZONE_NORMAL] = iter;
			}
			if (iter->next != NULL
				&& iter->next->base_addr >= ZONE_NORMAL_LIMIT) {
				iter->next = NULL;
			}
		}
		else if (iter->base_addr >= ZONE_NORMAL_LIMIT) {
			if (fast_zone_entry[ZONE_HIGHMEM] ==NULL) {
				fast_zone_entry[ZONE_HIGHMEM] = iter;
			}
		}
		iter = next;
	}
} 

static unsigned long add_to_zones_list(
	unsigned long index, unsigned long base, unsigned long size)
{
	unsigned long aligned_base;
	unsigned long aligned_size;
	unsigned long count = 0;

	aligned_base = PAGE_ALIGN(base);
	aligned_size = size - (aligned_base - base);
		
	if (aligned_size < PAGE_SIZE)
		return 0;	

	zones_list[index].next = NULL;

	if (aligned_base < ZONE_DMA_LIMIT &&
		aligned_base + size >= ZONE_DMA_LIMIT) {

		if (ZONE_DMA_LIMIT - aligned_base >= PAGE_SIZE) {
			zones_list[index].base_addr = aligned_base;
			zones_list[index].size = ZONE_DMA_LIMIT - aligned_base;
			count++;
		}

		count += add_to_zones_list(index + count, ZONE_DMA_LIMIT,
			base + size - ZONE_DMA_LIMIT);	
	}
	else if (aligned_base < ZONE_NORMAL_LIMIT &&
		aligned_base + size >= ZONE_NORMAL_LIMIT) {

		if (ZONE_NORMAL_LIMIT - aligned_base >= PAGE_SIZE) {
			zones_list[index].base_addr = aligned_base;
			zones_list[index].size = ZONE_NORMAL_LIMIT - aligned_base;
			count++;
		}

		count += add_to_zones_list(index + count, ZONE_NORMAL_LIMIT,
			base + size - ZONE_NORMAL_LIMIT);
	}
	else {
		if (PAGE_FLOOR(aligned_size) >= PAGE_SIZE) {
			zones_list[index].base_addr = aligned_base;
			zones_list[index].size = PAGE_FLOOR(aligned_size);
			count++;
		}
	}

	if (count != 0 && index != 0) {
		zones_list[index - 1].next = &zones_list[index];
	}
	return count;
}

static void mem_zones_create()
{
	unsigned long i;
	struct memory_map *mmap;
	u64 base_addr;
	unsigned long zones_index;
	
	base_addr = 0;
	zones_index = 0;
	for (i = 0; i != boot_params.mmap_length; ++i) {
		mmap = &boot_params.mmap[i];
			
		if (mmap->type == MMAP_USABLE &&
			mmap->base_addr >= base_addr) {	
			
			/* skip when mem_size more than 4GB */
			WARN_ON(mmap->base_addr + mmap->size >= 4*GB);
			if (mmap->base_addr + mmap->size >= 4*GB)
				break;

			zones_index += add_to_zones_list(
				zones_index, mmap->base_addr, mmap->size);
			BUG_ON(zones_index >= NR_ZONES);
		}

		base_addr = mmap->base_addr;
	}	
	
	init_fast_zone_entry();
}

void mem_zones_dump()
{
	mem_zone_t *iter = zones_list;	
	unsigned long i;
	
	zone_all_for_each(iter, i) {
		printk(" zone[%d]: ", i);
		printk("base=0x%x,size=0x%x,mbase=0x%x,mend=0x%x\n",
			iter->base_addr, iter->size, iter->mmap_base, iter->mmap_end);
	}
}

static void mem_info_init()
{
	u64 total_mem_size = 0;
	u64 usable_mem_size = 0;
	u64 mem_limit = 0;	
	mem_zone_t *iter;
	unsigned long i;

	for (i = 0; i != NR_ZONE_TYPES; ++i) {
		iter = fast_zone_entry[i];
		while(iter) {
			usable_mem_size += iter->size;
			iter = iter->next;
		}
	}

	for (i = 0; i != boot_params.mmap_length; ++i) {
		if (boot_params.mmap[i].type == MMAP_USABLE) {
			total_mem_size += boot_params.mmap[i].size;
			mem_limit = boot_params.mmap[i].base_addr
				+ boot_params.mmap[i].size;
		}
	}
	
	mem_info.total_size = total_mem_size;
	mem_info.usable_size = usable_mem_size;
	mem_info.limit = mem_limit;
	if (mem_limit > 4*GB)
		mem_info.usable_limit = 4*GB;
	else
		mem_info.usable_limit = mem_limit;
}

void mem_info_dump()
{
	printk("total memory size: %lld KB(%lld MB).",
		mem_info.total_size / KB,
		mem_info.total_size / MB);
	printk("usable memory size: %lld KB(%lld MB).\n",
		mem_info.usable_size / KB,
		mem_info.usable_size / MB);
	printk("real limit: 0x%0.8x%0.8x, usable limit: 0x%0.8x%0.8x\n",
		(unsigned long)(mem_info.limit>>32),
		(unsigned long)mem_info.limit,
		(unsigned long)(mem_info.usable_limit>>32),
		(unsigned long)mem_info.usable_limit);
}

/* implement of paging_init.initialize page directory and page tables */
static void _paging_init()
{
	unsigned long dir_index;
	unsigned long pte_index;
	unsigned long i;
	unsigned long addr;
	pgt_t *pgt_base;
	unsigned long virt_mem_size;
	
	/* initialize page directory */
	for (dir_index = 0; dir_index != pgd_index(PAGE_OFFSET); ++dir_index) {
		kernel_page_dir[dir_index] = mk_pgd(0, 0);
	}
	for (dir_index = pgd_index(PAGE_OFFSET), i = 0;
		dir_index != PTRS_PER_PGD; ++dir_index, ++i) {
		pgt_base = __kernel_pgt_base + PTRS_PER_PGT * i;
		kernel_page_dir[dir_index] = mk_pgd(
			(unsigned long)pa(pgt_base),
			PAGE_PRESENT | PAGE_RW); 
	}	

	if (mem_info.usable_limit > ZONE_NORMAL_LIMIT)
		virt_mem_size = ZONE_NORMAL_LIMIT;
	else
		virt_mem_size = mem_info.usable_limit;

	/* initialize page tables(non-highmem) */
	addr = 0;
	for (pte_index = 0; pte_index != PTRS_PER_PGT *
		ceil_div(virt_mem_size/PAGE_SIZE, PTRS_PER_PGT);
		++pte_index, addr += PAGE_SIZE) {
		__kernel_pgt_base[pte_index] = mk_pgt(addr,
			PAGE_PRESENT | PAGE_RW);	
	}

	/* initialize page tables(highmem) */
	addr = 0;
	while (__kernel_pgt_base + pte_index != __kernel_pgt_end)
	{
		__kernel_pgt_base[pte_index] =
			mk_pgt(addr , PAGE_PRESENT | PAGE_RW);
		pte_index++;
		addr += PAGE_SIZE;
	}
	
	/* done!switch to new page directory */
	switch_pgdir(kernel_page_dir);
}

/* do necessary check and calculations for __paging_init() */
static void paging_init()
{
	unsigned long nr_total_pages;
	unsigned long nr_used_pages;
	unsigned long nr_need_pages;
	unsigned long dir_index;
	unsigned long pte_index;
	unsigned long virt_mem_size;
	
	pgd_t *init_page_dir = (pgd_t *)PAGE_ALIGN(__boot);
	pgt_t *pte_addr;
	pgt_t pte;
	pgd_t pgd;
	/* page_dir 从内核结束的地方开始*/
	kernel_page_dir = (pgd_t *)PAGE_ALIGN(__end);
	nr_total_pages = 0;
	/* calculate number of available pages */	
	for (dir_index = pgd_index(PAGE_OFFSET);
		dir_index != PTRS_PER_PGD; ++dir_index) {

		pgd = init_page_dir[dir_index];
		if (pgd_present(pgd)) {
			pte_addr = (pgt_t *)pgd_page(pgd);
			for (pte_index = 0; pte_index != PTRS_PER_PGT;
				++pte_index) {
				pte = pte_addr[pte_index];
				if (pgt_present(pte))
					nr_total_pages++;
				else
					break;	
			}
		}//pgd_present
		else
			break;
	}
	nr_used_pages = 
		(pgd_index(PAGE_ALIGN(__end)) - pgd_index(PAGE_OFFSET))
			*PTRS_PER_PGT + pgt_index(PAGE_ALIGN(__end));

	virt_mem_size = ZONE_HIGHMEM_SIZE;
	nr_need_pages = ceil_div(virt_mem_size / PAGE_SIZE, PTRS_PER_PGT) + 1;
	/* if highmem is needed */
	if (mem_info.usable_limit > ZONE_NORMAL_LIMIT)
		virt_mem_size = ZONE_NORMAL_LIMIT;	
	else
		virt_mem_size = mem_info.usable_limit;
	/* nr_need_pages is equal to nr_page_dir(1) adds nr_page_tables */
	nr_need_pages += ceil_div(virt_mem_size / PAGE_SIZE, PTRS_PER_PGT);

	/* calculate kernel page end for using */
	__kernel_pgt_base = (pgt_t *)(kernel_page_dir + PTRS_PER_PGD);
	__kernel_pgt_end = __kernel_pgt_base + (nr_need_pages - 1)*PTRS_PER_PGT;

	printk("%d boot pages, %d pages used, ",
		nr_total_pages, nr_used_pages);	
	printk("%d pages remain and %d pages needed.\n",
		nr_total_pages - nr_used_pages, nr_need_pages);

	/* can't continue if no enough pages */
	BUG_ON(nr_need_pages > nr_total_pages - nr_used_pages);

#ifdef DEBUG
	printk("__end = 0x%x, __kernel_pgt_end = 0x%x\n",
		__end, __kernel_pgt_end);
#endif

	_paging_init();
}


/*
	find a fix zone and cut need_size from its size.
	NOTE:
	1.new size will be page aligned.
	2.mem_info.usable will not be equal to total size of zones continually.
*/
static unsigned long cut_zone_size(unsigned long need_size)
{
	unsigned long i;
	mem_zone_t *iter;
	mem_zone_t *fix_zone;
	unsigned long real_size;
	unsigned long max_size;

	real_size = 0;
	max_size = 0;
	fix_zone = NULL;

	zone_for_each(iter, i, ZONE_DMA, ZONE_HIGHMEM) {
		if ((u32)pa(__load) >= iter->base_addr &&
			(u32)pa(PAGE_ALIGN(__kernel_pgt_end))
				<= iter->base_addr + iter->size) {

			real_size = iter->size + iter->base_addr 
				- (u32)pa(PAGE_ALIGN(__kernel_pgt_end));
		}
		else
			real_size = iter->size; 

#ifdef DEBUG
		printk("real_size = %x \n", real_size);
#endif
		if (real_size > max_size) {
			max_size = real_size;
			fix_zone = iter;
		} 
	}
	
#ifdef DEBUG 
	printk("need_size = 0x%x, PAGE_CEIL(need_size) = 0x%x,"
		" real_size = 0x%x, fix_zone = 0x%x\n", need_size,
		PAGE_CEIL(need_size), real_size, fix_zone);
#endif

	BUG_ON(fix_zone == NULL);
	/* now fix_zone point to zone has max usable size memory */
	BUG_ON(real_size <= PAGE_CEIL(need_size));

	fix_zone->size -= PAGE_CEIL(need_size);	
	
	return (unsigned long)va(fix_zone->base_addr + fix_zone->size);
}


/* implement for mem_map_create */
static void _mem_map_create()
{
	mem_zone_t *iter;
	unsigned long i;
	mem_map_t *base;
	unsigned long num;

	base = mmap;

	zone_all_for_each(iter, i) {
		num = iter->size / PAGE_SIZE;
		iter->mmap_base = base;
		base += num;
		iter->mmap_end = base;
		memset(iter->mmap_base, 0,
			(unsigned long)iter->mmap_end - (unsigned long)iter->mmap_base);
	}
}

/* pick an area to store mmap */
static void mem_map_create()
{
	u64 need_size;
	
	need_size = sizeof(mem_map_t) * mem_info.usable_size; 
	do_div64(need_size, PAGE_SIZE + sizeof(mem_map_t));
	need_size = align_down((unsigned long)need_size, sizeof(mem_map_t));

#ifdef DEBUG
	printk(" need_size = %lld\n", need_size);
#endif

	mmap = (mem_map_t *)cut_zone_size(need_size);	

#ifdef DEBUG
	printk("mmap = 0x%x\n", mmap);
#endif

	_mem_map_create();
}

#ifdef DEBUG
static void page_alloc_test()
{
	unsigned long count;
	mem_zone_t *zone;
	unsigned long i;
	void *fetch[159];
	
	zone_dma_for_each(zone) {
		zone_page_alloc_dump(zone);
	}
	printk("\n");
	
	for (i = 0; i != 159; ++i)
	{
		fetch[i] = get_zeroed_page(GFP_DMA);
	}
	while(get_zeroed_page(GFP_DMA))
		count++;
	
	zone_dma_for_each(zone) {
		zone_page_alloc_dump(zone);
	}
	printk("\n");
	
	for (i = 0; i != 159; ++i) {
		if (fetch[i])
			free_page(fetch[i]);
	}
	
	zone_dma_for_each(zone) {
		zone_page_alloc_dump(zone);
	}
	printk("count = %d\n", count);
}
#endif

#ifdef DEBUG
static void highmem_test()
{
	unsigned long count;
	struct page *page;
	void *mapped[257];
	unsigned long i;

	page = alloc_pages(ZONE_NORMAL, 0);

	count = 256*128 ;
	i = 0;
	highmem_dump();
	while(count--) {	
		if (i != 256)
			mapped[i++] = kmap(page, 0);
		else
			kmap(page, 0);
	}
	highmem_dump();
	for (i = 0; i != 256; ++i) {
		kunmap(mapped[i], 0);
	}
	highmem_dump();
}
#endif

#ifdef DEBUG
static void kmalloc_test()
{
	void *fetch[100];
	unsigned long i;

	kmalloc_dump();	

	for (i = 0; i != 100; ++i)
	{
		fetch[i] = kmalloc(466);
	}

	kmalloc_dump();

	for (i = 0; i != 100; ++i)
	{
		kfree(fetch[i]);
	}
	
	kmalloc_dump();
}
#endif

void mm_init()
{
	mem_zones_create();
#ifdef DEBUG
	mem_zones_dump();
#endif

	mem_info_init();
#ifdef DEBUG
	mem_info_dump();
#endif
	/* can access total physic memory after paging_init() */
	paging_init();

	mem_map_create();

	mem_zones_dump();
	
	/* enable alloc_pages */	
	page_alloc_init();

#ifdef DEBUG
	page_alloc_test();
#endif

	highmem_init();	

#ifdef DEBUG
	highmem_test();
#endif

	kmalloc_init();

#ifdef DEBUG
	kmalloc_test();
#endif
}
