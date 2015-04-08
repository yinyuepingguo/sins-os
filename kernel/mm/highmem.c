#include <sins/mm.h>
#include <list.h>
#include <stddef.h>
#include <sins/kernel.h>
#include <sins/irq.h>

/* defined here. I don't want these structs used by outer */
typedef struct high_map
{
	unsigned long flags;
	pgt_t *entry;
	struct list_head list;	
} high_map_t;

static high_map_t *hmap, *hmap_end;

typedef struct high_area
{
	struct list_head list;
	unsigned long size;
} high_area_t;

high_area_t high_area[MAX_ORDER + 1];

void highmem_dump()
{
	unsigned long i;
	unsigned long count;
	struct list_head *pos;
		
	for (i = 0; i != MAX_ORDER + 1; ++i) {
		count = 0;
		list_for_each(pos, &high_area[i].list) {
			count++;
		}
		BUG_ON(high_area[i].size != count);
		printk(" horder[%d] = %d \t\t", i, count);
	}
	printk("\n");
}


static void __init_add_to_high_area(high_map_t *hmap, unsigned long order)
{
	mmap_set_private(hmap);	
	high_area[order].size++;
	list_add(&hmap->list, &high_area[order].list);
}

static void init_high_area()
{
	unsigned long i;
	
	for (i = 0; i != MAX_ORDER + 1; ++i) {
		INIT_LIST_HEAD(&high_area[i].list);
		high_area[i].size = 0;
	}
}

static void init_high_map()
{
	high_map_t *iter;
	pgt_t *high_base;

	iter = hmap;
	high_base = kernel_pgt_highmem_base(); 

	while (iter != hmap_end) {
		iter->entry = high_base + (iter - hmap); 
		iter++;
	}
}

static void add_to_high_area(high_map_t *hmap, unsigned long order)
{
	BUG_ON(!mmap_test_private(hmap));
	high_area[order].size++;
	list_add(&hmap->list, &high_area[order].list);
}

static high_map_t *pop_from_high_area(unsigned long order)
{
	struct list_head *list = &high_area[order].list;
	struct list_head *fetch;

	if (list_empty(list))
		return NULL;
	
	high_area[order].size--;
	fetch = list->next;
	list_del_init(fetch);
	return list_entry(fetch, high_map_t, list);
}

static void remove_from_high_area(high_map_t *hmap, unsigned long order)
{
	high_area[order].size--;
	list_del_init(&hmap->list);
}

static high_map_t *get_buddy(high_map_t *me, unsigned long order)
{
	unsigned long combine_size = 1UL << (order + 1);
	unsigned long buddy_size = 1UL << order;
	unsigned long offset = me - hmap;
	high_map_t *my_buddy;

	if (offset % combine_size == 0) {
		/* I'm left boy, my buddy is right */
		my_buddy = hmap + offset + buddy_size;

		if ((my_buddy + buddy_size - hmap_end) > 0) {
			/* out of boundary */
			return NULL;
		}
	}
	else {
		/* I'm right boy, my buddy is left */	
		my_buddy = hmap + offset - buddy_size;
	}
	
	/* my buddy is not a head */
	if (!mmap_test_private(my_buddy)) {
		return NULL;
	}
	/* my buddy have been allocated */
	if (mmap_test_used(my_buddy)) {
		return NULL;
	}

	if (order == 0 || !mmap_test_private(my_buddy + (1UL << (order - 1)))) {
		return my_buddy;
	}
	
	return NULL;

}

static high_map_t *reduce(high_map_t *hmap, unsigned long order)
{
	high_map_t *buddy1, *buddy2;
	BUG_ON(order == 0);

	buddy1 = hmap;
	buddy2 = hmap + (1UL << (order - 1));
	mmap_set_private(buddy2);
	add_to_high_area(buddy2, order - 1);
	return buddy1;
}

static high_map_t *expand(high_map_t *buddy1, high_map_t *buddy2,
	unsigned long order) 
{
	high_map_t *left_buddy;
	high_map_t *right_buddy;

	if (buddy1 < buddy2) {
		left_buddy = buddy1;
		right_buddy = buddy2;
	}
	else {
		left_buddy = buddy2;
		right_buddy = buddy1;
	}

	mmap_clear_private(right_buddy);
	return left_buddy;
}

static void build_buddy_system()
{
	unsigned long size;
	unsigned long hmap_index;
	unsigned long order;

	hmap_index = 0;
	size = (hmap_end - hmap) * PAGE_SIZE;

	while(size >= PAGE_SIZE) {
		if (size > (PAGE_SIZE << MAX_ORDER))
			order = MAX_ORDER;
		else
			order = get_order_floor(size);

		__init_add_to_high_area(hmap + hmap_index, order);
		hmap_index += (1UL << order);
		size -= (PAGE_SIZE << order);
	}

	BUG_ON(size != 0);
}

/*
	to call this function, you need to make sure get_free_pages is ok.
	After call highmem_init(), kmap, kunmap, ioremap, iounmap is ok.
*/
void highmem_init()
{
	unsigned long need_size;

	need_size = ZONE_HIGHMEM_SIZE / PAGE_SIZE * sizeof(high_map_t);
	hmap = get_zeroed_pages(GFP_DMA|GFP_NORMAL, get_order(need_size));
	hmap_end = hmap + need_size / sizeof(high_map_t);

#ifdef DEBUG	
	printk("hmap = 0x%x, hmap_end = 0x%x, size = %d\n", hmap, hmap_end,
		hmap_end - hmap);
#endif

	BUG_ON(hmap == NULL);

	init_high_area();

	init_high_map();
	
	build_buddy_system();
}

static high_map_t *alloc_high_map(unsigned long order)
{
	unsigned long search_order;
	high_map_t *fetch;
	unsigned long flags;

	WARN_ON(order > MAX_ORDER);

	if (order > MAX_ORDER)
		return NULL;
	
	irq_save(flags);
	for(search_order = order;
		search_order != MAX_ORDER + 1; ++search_order) {
		fetch = pop_from_high_area(search_order);
		if (fetch) {
			while(search_order != order) {
				fetch = reduce(fetch, search_order);
				--search_order;	
			}
			mmap_set_used(fetch);

			BUG_ON(!mmap_test_private(fetch));
			irq_restore(flags);
			return fetch;
		}
		else
			continue;
	}
	irq_restore(flags);
	return NULL;
}

static void dealloc_high_map(high_map_t *hmap, unsigned long order)
{
	high_map_t *buddy1, *buddy2;
	unsigned long flags;

	irq_save(flags);
	WARN_ON(hmap == NULL);
	BUG_ON(!mmap_test_used(hmap));
	BUG_ON(order > MAX_ORDER);

	mmap_clear_used(hmap);

	buddy1 = hmap;
	buddy2 = NULL;
	while(order != MAX_ORDER &&
		(buddy2 = get_buddy(buddy1, order)) != NULL) {

#ifdef DEBUG		
		printk("%s while: buddy1 = 0x%x, buddy2 = 0x%x, order = 0x%x\n",
		__FUNCTION__, buddy1, buddy2, order);
#endif

		remove_from_high_area(buddy2, order);
		buddy1 = expand(buddy1, buddy2, order);
		order++;
	}

#ifdef DEBUG
	printk("%s : buddy1 = 0x%x, order = 0x%x\n",
		__FUNCTION__, buddy1, order);
#endif

	add_to_high_area(buddy1, order); 
	irq_restore(flags);
}

static void *hmap_to_virt(high_map_t *map)
{
	unsigned long offset = map - hmap;

	BUG_ON(map < hmap);
	BUG_ON(map >= hmap_end);	

	return (void *)(PAGE_OFFSET + ZONE_NORMAL_LIMIT + offset * PAGE_SIZE);
} 

static high_map_t *virt_to_hmap(unsigned long addr)
{
	BUG_ON(addr < PAGE_OFFSET + ZONE_NORMAL_LIMIT);
	return hmap + (addr - PAGE_OFFSET - ZONE_NORMAL_LIMIT) / PAGE_SIZE;
}

void *kmap(struct page *page, unsigned long order)
{
	unsigned long phys = (unsigned long)page_to_phys(page);
	high_map_t *hmap = alloc_high_map(order);
	unsigned long i;

	if (hmap) {
		for (i = 0; i != (1UL << order); ++i) {
			*hmap[i].entry = mk_pgt(phys, PAGE_PRESENT | PAGE_RW);
			phys += PAGE_SIZE;
		}
		return hmap_to_virt(hmap);
	}
	else {
		return NULL;
	}
}

void kunmap(void *mapped, unsigned long order)
{
	high_map_t *hmap = virt_to_hmap((unsigned long)mapped);
	dealloc_high_map(hmap, order);	
}

void *ioremap(unsigned long addr, unsigned long size)
{
	unsigned long order = get_order(size);
	high_map_t *hmap;
	unsigned long i;

	BUG_ON(PAGE_ALIGN(addr) != addr);

	hmap = alloc_high_map(order);
	
	if (hmap) {
		for (i = 0; i != (1UL << order); ++i) {
			*hmap[i].entry = mk_pgt(addr, PAGE_PRESENT | PAGE_RW);
			addr += PAGE_SIZE;
		}
		return hmap_to_virt(hmap);
	}
	else
		return NULL;
}

void iounmap(void *mapped, unsigned long size)
{
	kunmap(mapped, get_order(size));
}
