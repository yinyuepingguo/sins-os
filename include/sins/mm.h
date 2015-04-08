#ifndef _SINS_MM_H
#define _SINS_MM_H

#include <sins/page.h>
#include <types.h>
#include <list.h>
#include <bitops.h>

#define PHYS_OFFSET 0x100000

#define KB (1ULL << 10) 
#define MB (1ULL << 20)
#define GB (1ULL << 30)

#define VIRT_MEM_SIZE (4*GB)

#define ZONE_DMA 	0	
#define ZONE_NORMAL	1
#define ZONE_HIGHMEM	2
#define NR_ZONE_TYPES	3

#define NR_ZONES 10

/* set zones limit */
#define ZONE_DMA_LIMIT 0x1000000
#define ZONE_NORMAL_LIMIT 0x38000000
#define ZONE_HIGHMEM_SIZE 0x8000000

#if ZONE_DMA_LIMIT%PAGE_SIZE != 0
#error "ZONE_DMA_LIMIT must be page aligned"
#endif

#if ZONE_NORMAL_LIMIT%PAGE_SIZE != 0
#error "ZONE_NORMAL_LIMIT must be page aligned"
#endif

#if ZONE_HIGHMEM_SIZE%PAGE_SIZE != 0
#error "ZONE_HIGHMEM_SIZE must be page aligned"
#endif

#if VIRT_MEM_SIZE - PAGE_OFFSET - ZONE_NORMAL_LIMIT != ZONE_HIGHMEM_SIZE
#error "VIRT_MEM_SIZE - PAGE_OFFSET - ZONE_NORMAL_LIMIT != ZONE_HIGHEM_SIZE"
#endif

#define GFP_DMA		(0x1)	
#define GFP_NORMAL	(0x2)

#define MAX_ORDER 8

/* buddy system used */
#define MMAP_BITS_PRIVATE 0
/* buddy system used */
#define MMAP_BITS_USED 1

#define mmap_test_private(mmap)	\
	test_bit((mmap)->flags, MMAP_BITS_PRIVATE)
#define mmap_set_private(mmap)	\
	set_bit((mmap)->flags, MMAP_BITS_PRIVATE)
#define mmap_clear_private(mmap)	\
	clear_bit((mmap)->flags, MMAP_BITS_PRIVATE)

#define mmap_test_used(mmap)	\
	test_bit((mmap)->flags, MMAP_BITS_USED)
#define mmap_set_used(mmap)	\
	set_bit((mmap)->flags, MMAP_BITS_USED)
#define mmap_clear_used(mmap)	\
	clear_bit((mmap)->flags, MMAP_BITS_USED)


typedef struct mem_info
{
	u64 total_size;	
	u64 usable_size;
	u64 limit;
	u64 usable_limit;
} mem_info_t;


typedef struct page
{
	unsigned long flags;
	struct list_head list;
} mem_map_t;


typedef struct free_area
{
	struct list_head list;
	unsigned long size;
} free_area_t;

/*
	zone's data will be initialized by their respective part.
	You can use these data only used part has been initialized.
	This strategy will also be used to mem_map_t which has
	data comes from different parts.
 */
typedef struct mem_zone
{
	/* NOTE:base_addr and size is PAGE ALIGNED */
	/* 32bit,64 bit is not a simple thing */
	unsigned long base_addr;
	unsigned long size;
	mem_map_t *mmap_base;
	mem_map_t *mmap_end;
	
	free_area_t free_area[MAX_ORDER + 1];
	/* pointer to zone with same flag(DMA,NORMAL or HIGHMEM */
	struct mem_zone *next;
} mem_zone_t;


/* [from_zone, to_zone) */
#define zone_for_each(iter, now_zone, from_zone, to_zone) \
	for ((now_zone) = (from_zone); (now_zone) != (to_zone); ++(now_zone)) 	\
		for ((iter) = fast_zone_entry[(now_zone)]; (iter);	\
			(iter) = (iter)->next)

#define zone_type_for_each(iter, zone_type)	\
	for (iter = fast_zone_entry[(zone_type)]; (iter); (iter) = (iter)->next)


#define zone_dma_for_each(iter)	\
	zone_type_for_each(iter, ZONE_DMA)

#define zone_normal_for_each(iter)	\
	zone_type_for_each(iter, ZONE_NORMAL)

#define zone_highmem_for_each(iter)	\
	zone_type_for_each(iter, ZONE_HIGHMEM)

#define zone_all_for_each(iter, now_zone)	\
	zone_for_each(iter, now_zone, ZONE_DMA, NR_ZONE_TYPES)

#define zone_empty(zone_type) (fast_zone_entry[(zone_type)] == NULL)

	
extern char __load[];
extern char __load_end[];

extern char __boot[];
extern char __boot_end[];

extern char __init_task[];
extern char __init_task_end[];

extern char __text[];
extern char __text_end[];

extern char __data[];
extern char __data_end[];

extern char __bss[];
extern char __bss_end[];

extern char __end[];

extern mem_zone_t zones_list[NR_ZONES];
extern pgd_t *kernel_page_dir;
extern pgt_t *__kernel_pgt_base;
extern pgt_t *__kernel_pgt_end;
extern mem_map_t*mmap;
extern mem_zone_t *fast_zone_entry[NR_ZONE_TYPES];
extern mem_info_t mem_info;

#define kernel_pgt_highmem_base() ((pgt_t *)va(pgd_page(kernel_page_dir[	\
	 pgd_index(PAGE_OFFSET + ZONE_NORMAL_LIMIT)])))

extern void mm_init();

extern void mem_zones_dump();

extern void mem_info_dump(); 

extern void zone_page_alloc_dump(mem_zone_t *zone);

extern void highmem_dump();

extern void kmalloc_dump();

extern struct page *phys_to_page(unsigned long addr);

extern mem_zone_t *phys_to_zone(unsigned long addr);

/* Only used for ZONE_DMA and ZONE_NORMAL */
#define virt_to_page(addr) phys_to_page((unsigned long)pa((addr)))

extern void *page_to_phys(struct page *page);

extern mem_zone_t *page_to_zone(struct page *page);

/* Only used for ZONE_DMA and ZONE_NORMAL */
#define page_to_virt(page) va(page_to_phys((page)))

extern void page_alloc_init();

/* DON'T use it IF not clear */
extern void alloc_region(unsigned long base, unsigned long end);

/* DON'T use it IF not clear */
extern void dealloc_region(unsigned long base, unsigned long end);

extern struct page *alloc_pages(unsigned long zone, unsigned long order);

extern void dealloc_pages(struct page *page, unsigned long order);

extern void *get_free_pages(unsigned long gfp_mask, unsigned long order);

#define get_free_page(gfp_mask) get_free_pages((gfp_mask), 0)

extern void *get_zeroed_pages(unsigned long gfp_mask, unsigned long order);
	
#define get_zeroed_page(gfp_mask) get_zeroed_pages(gfp_mask, 0)

#define get_dma_pages(gfp_mask, order)	\
	get_free_pages((gfp_mask) | GFP_DMA,(order))

extern void free_pages(void *addr, unsigned long order);

#define free_page(addr) free_pages((addr), 0)

extern void highmem_init();

extern void *kmap(struct page *page, unsigned long order);

extern void kunmap(void *mapped, unsigned long order);

extern void *ioremap(unsigned long addr, unsigned long size);

extern void iounmap(void *mapped, unsigned long size);

extern void kmalloc_init();

extern void *kmalloc(unsigned long size);

extern void kfree(void *addr);

extern pgd_t *copy_empty_page_dir();

extern struct page *load_page(pgd_t *pgd, unsigned long start_addr);

extern void free_page_dir(pgd_t *pgd);

#endif
