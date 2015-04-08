#include <sins/mm.h>
#include <stddef.h>
#include <list.h>
#include <sins/kernel.h>
#include <sins/irq.h>
#include <string.h>

/*
	page_alloc works well when buddy system initializes successfully!
	(you can use them after init_page_alloc() called)
	To call init_page_alloc, You need to initialize all zones' mmap.
	mmap_base,mmap_end, set zero for MMAP_BITS_PRIVATE and MMAP_BITS_USED.
*/
static void add_to_free_area(mem_zone_t *zone, mem_map_t *mmap,
	unsigned long order);

static mem_map_t *pop_from_free_area(mem_zone_t *zone, unsigned long order);

static void remove_from_free_area(mem_zone_t *zone, mem_map_t *mmap,
	unsigned long order);

static mem_map_t *get_buddy(mem_zone_t *zone, mem_map_t *me, unsigned long order);

static mem_map_t *reduce(mem_zone_t *zone, mem_map_t *mmap, unsigned long order);

static mem_map_t *expand(mem_map_t *buddy1, mem_map_t *buddy2,
	unsigned long order);

static void split_to(mem_map_t *mmap, unsigned long from_order,
	unsigned long to_order);

/* only used for build_zone_buddy_system */
static void init_free_area(free_area_t *free_area)
{
	INIT_LIST_HEAD(&free_area->list);
	free_area->size = 0;
}

/* only used for build_zone_buddy_system */
static void __init_add_to_free_area(mem_zone_t *zone, mem_map_t *mmap,
	unsigned long order)
{
	mmap_set_private(mmap);
	zone->free_area[order].size++;
	list_add(&mmap->list, &zone->free_area[order].list);
}

static void _build_zone_buddy_system(mem_zone_t *zone, mem_map_t *mmap,
	mem_map_t *end)
{
	unsigned long size;
	unsigned long mmap_index;
	unsigned long order;

	mmap_index = 0;
	size = (end - mmap) * PAGE_SIZE;
	while(size >= PAGE_SIZE) {
		if (size > (PAGE_SIZE << MAX_ORDER))
			order = MAX_ORDER;
		else
			order = get_order_floor(size);

		__init_add_to_free_area(zone, mmap + mmap_index, order);
		mmap_index += (1UL << order);
		size -= (PAGE_SIZE << order);
	}

	BUG_ON(size != 0);
}


static void build_zone_buddy_system(mem_zone_t *zone)
{
	unsigned long i;

	for (i = 0; i != MAX_ORDER + 1; ++i) {
		init_free_area(&zone->free_area[i]);    
	}
	
	_build_zone_buddy_system(zone, zone->mmap_base, zone->mmap_end);
}

/*
	advise only used for init_page_alloc, change region that has been added
	to buddy system to used state.
	NOTE: base and end are physic addresses;
	It's NOT a fast function. [base, end) scope must be in free_area.
	DON'T use it if not really necessary.
*/
void alloc_region(unsigned long base, unsigned long end)
{	
	mem_zone_t *zone;
	unsigned long i;
	struct list_head *pos, *tmp;
	unsigned long phys_addr;
	unsigned long phys_addr_end;
	unsigned long phys_addr_min;
	unsigned long phys_addr_end_max;
	struct page *page;
	unsigned long flags;
	
	BUG_ON(base % PAGE_SIZE);
	BUG_ON(end % PAGE_SIZE);
	BUG_ON(base > end);

	irq_save(flags);
	zone = phys_to_zone(base);
	
	BUG_ON(zone == NULL);
	BUG_ON(zone != phys_to_zone(end));

	phys_addr_min = ULONG_MAX;
	phys_addr_end_max = 0;
	for (i = 0; i != MAX_ORDER + 1; ++i) {
		list_for_each_safe(pos, tmp, &zone->free_area[i].list) {

			page = list_entry(pos, struct page, list);
			phys_addr = (unsigned long)page_to_phys(page);
			phys_addr_end = phys_addr + (PAGE_SIZE << i);

			if ((base >= phys_addr && base < phys_addr_end) ||
				(phys_addr > base && end > phys_addr)) {

#ifdef DEBUG
				printk("phys_addr = 0x%x, end = 0x%x, min = 0x%x,"
					" max = 0x%x\n", phys_addr, phys_addr_end, 
					phys_addr_min, phys_addr_end_max);
#endif

				remove_from_free_area(zone, page, i);
				mmap_set_used(page);
				split_to(page, i, 0);

				if (phys_addr < phys_addr_min)
					phys_addr_min = phys_addr;
				if (phys_addr_end > phys_addr_end_max)
					phys_addr_end_max = phys_addr_end;
			}
		}
	}

#ifdef DEBUG
	printk("phys_addr_min = 0x%x, base = 0x%x, end = 0x%x,"
		" phys_addr_end_max = 0x%x\n", phys_addr_min, 
		base, end, phys_addr_end_max);
#endif

	dealloc_region(phys_addr_min, base);
	dealloc_region(end, phys_addr_end_max);
	irq_restore(flags);
}

/*
	This function can give back Area's page's allocated by alloc_region.
	And you can give back part of this region. 
	You can use it if you really know what you are doing.
 */
void dealloc_region(unsigned long base, unsigned long end)
{
	mem_zone_t *zone;
	unsigned long i;

	BUG_ON(base % PAGE_SIZE);
	BUG_ON(end % PAGE_SIZE);
	BUG_ON(base > end);

	zone = phys_to_zone(base);

	BUG_ON(zone == NULL);
	BUG_ON(zone != phys_to_zone(end));

	for (i = base; i != end; i += PAGE_SIZE) {
		dealloc_pages(phys_to_page(i), 0);
	}
}

/*
	to hide inner implement.
	alloc* ,  get* and free* is enable after call it.
 */
void page_alloc_init()
{
	mem_zone_t *iter;
	unsigned long i;

	zone_all_for_each(iter, i) {
		build_zone_buddy_system(iter);
	}

	/* wipe [__load, __kernel_pgt_end). */
	alloc_region((unsigned long)PAGE_FLOOR(pa(__load)),
		(unsigned long)PAGE_CEIL(pa(__kernel_pgt_end)));
}

void zone_page_alloc_dump(mem_zone_t *zone)
{
	unsigned long i;
	unsigned long count;
	struct list_head *pos;

	for (i = 0; i != MAX_ORDER + 1; ++i) {
		count = 0;
		list_for_each(pos, &zone->free_area[i].list) {
			count++;
		}
		BUG_ON(zone->free_area[i].size != count);
		printk(" order[%d] = %d \t\t",
			i ,count);
	}
	printk("\n");
}

static void split_to(mem_map_t *mmap, unsigned long from_order,
	unsigned long to_order)
{
	unsigned long i;
	
	BUG_ON(from_order < to_order);

	for (i = 0; i != (1UL << from_order); i += (1UL << to_order)) {
		if (mmap_test_private(mmap))
			mmap_set_private(&mmap[i]);
		if (mmap_test_used(mmap))
			mmap_set_used(&mmap[i]);
	}
}


struct page *phys_to_page(unsigned long addr)
{
	unsigned long zone_type;
	mem_zone_t *zone = NULL;

	addr = PAGE_FLOOR(addr);

	if (addr < ZONE_DMA_LIMIT)
		zone_type = ZONE_DMA;
	else if (addr < ZONE_NORMAL_LIMIT)
		zone_type = ZONE_NORMAL;
	else
		zone_type = ZONE_HIGHMEM;

	zone_type_for_each(zone, zone_type) {
		if (zone->base_addr <= addr &&
			addr < zone->base_addr + zone->size)
			return zone->mmap_base +
				(addr - zone->base_addr) / PAGE_SIZE;
	}

	return NULL;
}

mem_zone_t *phys_to_zone(unsigned long addr)
{
	unsigned long zone_type;
	mem_zone_t *zone = NULL;

	addr = PAGE_FLOOR(addr);

	if (addr < ZONE_DMA_LIMIT)
		zone_type = ZONE_DMA;
	else if (addr < ZONE_NORMAL_LIMIT)
		zone_type = ZONE_NORMAL;
	else
		zone_type = ZONE_HIGHMEM;

	zone_type_for_each(zone, zone_type) {
		if (zone->base_addr <= addr &&
			addr < zone->base_addr + zone->size)
			return zone;
	}

	return NULL;
}

void *page_to_phys(struct page *page) 
{
	mem_zone_t *zone;

	BUG_ON((zone = page_to_zone(page)) == NULL);
	return (void *)(zone->base_addr + (page - zone->mmap_base) * PAGE_SIZE);
}

mem_zone_t *page_to_zone(struct page *page)
{
	unsigned long zone_type;
	mem_zone_t *zone;

	zone_all_for_each(zone, zone_type) {
		if (page >= zone->mmap_base && page < zone->mmap_end)
			return zone;
	}

	return NULL;
}


static void add_to_free_area(mem_zone_t *zone, mem_map_t *mmap,
	unsigned long order)
{
	BUG_ON(!mmap_test_private(mmap));
	zone->free_area[order].size++;
	list_add(&mmap->list, &zone->free_area[order].list);
}

static mem_map_t *pop_from_free_area(mem_zone_t *zone, unsigned long order)
{
	struct list_head *list = &zone->free_area[order].list;
	struct list_head *fetch;

	if (list_empty(list))
		return NULL;
	
	zone->free_area[order].size--;
	fetch = list->next;
	list_del_init(fetch);
	return list_entry(fetch, mem_map_t, list);
}

static void remove_from_free_area(mem_zone_t *zone, mem_map_t *mmap,
	unsigned long order)
{
	zone->free_area[order].size--;
	list_del_init(&mmap->list);	
}

/* When you call this, you need to know which zone I'm in */
static mem_map_t *get_buddy(mem_zone_t *zone, mem_map_t *me, unsigned long order)
{
	unsigned long combine_size = 1UL << (order + 1);
	unsigned long buddy_size = 1UL << order;
	unsigned long offset = me - zone->mmap_base;
	mem_map_t *my_buddy;

	if (offset % combine_size == 0) {
		/* I'm left boy, my buddy is right */
		my_buddy = zone->mmap_base + offset + buddy_size;

		if ((my_buddy + buddy_size - zone->mmap_end) > 0) {
			/* out of boundary */
			return NULL;
		}
	}
	else {
		/* I'm right boy, my buddy is left */	
		my_buddy = zone->mmap_base + offset - buddy_size;
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

/* 
	This will reduce mmap's size by half.
	You need tell mmap's order. 
	return a reduced mmap.The other half returns free_area.
*/
static mem_map_t *reduce(mem_zone_t *zone, mem_map_t *mmap, unsigned long order)
{
	mem_map_t *buddy1, *buddy2;
	BUG_ON(order == 0);

	buddy1 = mmap;
	buddy2 = mmap + (1UL << (order - 1));
	mmap_set_private(buddy2);
	add_to_free_area(zone, buddy2, order - 1);
	return buddy1;
}

static mem_map_t *expand(mem_map_t *buddy1, mem_map_t *buddy2, unsigned long order)
{
	mem_map_t *left_buddy;
	mem_map_t *right_buddy;

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


/* kernel function */
struct page *alloc_pages(unsigned long zone_type, unsigned long order)
{
	unsigned long search_order;
	mem_zone_t *zone;
	mem_map_t *fetch;
	unsigned long i;
	unsigned long flags;

	BUG_ON(zone_type >= NR_ZONE_TYPES);
	WARN_ON(order > MAX_ORDER);
	if (order > MAX_ORDER)
		return NULL;

	irq_save(flags);
	zone_for_each(zone, i, zone_type, zone_type + 1) {
		for(search_order = order;
			search_order != MAX_ORDER + 1; ++search_order) {
			fetch = pop_from_free_area(zone, search_order);
			if (fetch) {
				while(search_order != order) {
					fetch = reduce(zone, fetch, search_order);
					--search_order;	
				}
				mmap_set_used(fetch);
				irq_restore(flags);
				return fetch;
			}
			else
				continue;
		}
	}
#ifdef DEBUG
	printk("page_alloc returns NULL(no mem)\n");
#endif
	irq_restore(flags);
	return NULL;
}

/* kernel function */
void dealloc_pages(struct page* page, unsigned long order)
{
	mem_zone_t *zone;
	struct page *buddy1, *buddy2;
	unsigned long flags;

	irq_save(flags);
	WARN_ON(page == NULL);
	BUG_ON(!mmap_test_used(page));
	BUG_ON(order > MAX_ORDER);

	BUG_ON((zone = page_to_zone(page)) == NULL);

	mmap_clear_used(page);

	buddy1 = page;
	buddy2 = NULL;
	while(order != MAX_ORDER &&
		(buddy2 = get_buddy(zone, buddy1, order)) != NULL) {

#ifdef DEBUG		
		printk("%s while:zone = 0x%x, buddy1 = 0x%x, buddy2 = 0x%x, order = 0x%x\n",
		__FUNCTION__, zone, buddy1, buddy2, order);
#endif

		remove_from_free_area(zone, buddy2, order);
		buddy1 = expand(buddy1, buddy2, order);
		order++;
	}

#ifdef DEBUG
	printk("%s :zone = 0x%x, buddy1 = 0x%x, order = 0x%x\n",
		__FUNCTION__, zone, buddy1, order);
#endif

	add_to_free_area(zone, buddy1, order); 
	irq_restore(flags);
}

void *get_free_pages(unsigned long gfp_mask, unsigned long order)
{
	struct page* page;

	if (gfp_mask & GFP_NORMAL)
		page = alloc_pages(ZONE_NORMAL, order);		
	else if (gfp_mask & GFP_DMA)
		page = alloc_pages(ZONE_DMA, order);
	else
		page = NULL;

	if (page)
		return page_to_virt(page);
	else
		return NULL;
}

void free_pages(void *addr, unsigned long order)
{
	dealloc_pages(virt_to_page(addr), order);
}

void *get_zeroed_pages(unsigned long gfp_mask, unsigned long order)
{
	void *page = get_free_pages(gfp_mask, order);
	
	if (page)
		memset(page, 0, PAGE_SIZE << order);
	
	return page;
}


