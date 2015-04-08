#include <sins/mm.h>
#include <list.h>
#include <stddef.h>
#include <sins/kernel.h>
#include <sins/irq.h>


/* 16 32 64 128 256 512 */
#define KMALLOC_MAX_ORDER 5
#define KMALLOC_SHIFT 4
#define KMALLOC_MASK ((1UL << KMALLOC_SHIFT)-1)

typedef struct page_head
{
	unsigned long index;
	struct list_head page_list;
	struct list_head used_list;
	struct list_head free_list;
} page_head_t; 

typedef struct obj_head
{
	struct list_head list;
} obj_head_t;

static struct list_head page_list[KMALLOC_MAX_ORDER + 1];

void kmalloc_dump()
{
	unsigned long count;
	struct list_head *pos;
	unsigned long i;

	for(i = 0; i != KMALLOC_MAX_ORDER + 1; ++i) {
		count = 0;
		list_for_each(pos, &page_list[i]) {
			count++;
		}
		printk(" order[%d]=%d \t", i, count);
	}
	printk("\n");
}


static void init_page(void *page, unsigned long order)
{
	page_head_t *page_head = (page_head_t *)page;
	unsigned long offset = sizeof(page_head_t);
	unsigned long obj_size = (1UL << (order + KMALLOC_SHIFT));
	obj_head_t *obj_head;
	
	page_head->index = order;
	INIT_LIST_HEAD(&page_head->page_list);
	INIT_LIST_HEAD(&page_head->used_list);
	INIT_LIST_HEAD(&page_head->free_list);

	while (offset + sizeof(obj_head_t) + obj_size <= PAGE_SIZE) {
		obj_head = (obj_head_t *)((unsigned long)page + offset);
		list_add(&obj_head->list, &page_head->free_list);
		offset += sizeof(obj_head_t) + obj_size;
	}
}

static long kmalloc_index(unsigned long size)
{
	long index;

	if (size & KMALLOC_MASK) 
		index = 0;
	else
		index = -1;
	size >>= KMALLOC_SHIFT;
	while(size) {
		index++;
		size >>= 1;
	}
	if (index > KMALLOC_MAX_ORDER)
		index = -1;
	return index;
}

/*
	To run this function, you must make sure get_free_page and free_page is ok
	You can use kmalloc and kfree and call this function.
*/
void kmalloc_init()
{
	unsigned long i;
	
	for (i = 0; i != KMALLOC_MAX_ORDER + 1; ++i) {
		INIT_LIST_HEAD(&page_list[i]);
	}
}

void *kmalloc(unsigned long size)
{
	long index = kmalloc_index(size);
	struct list_head *list;
	void *page;
	page_head_t *page_head;
	struct list_head *fetch;
	unsigned long flags;

	BUG_ON(index == -1);	
	
	irq_save(flags);
	
	list = &page_list[index];
	page_head = list_entry(list->next, page_head_t, page_list);

	if (list_empty(list) || list_empty(&page_head->free_list)) {
		page = get_free_page(GFP_NORMAL | GFP_DMA);
		if (page) {
			init_page(page, index);
			page_head = (page_head_t *)page;
			list_add(&page_head->page_list, list);
		} else {
			irq_restore(flags);
			return NULL;
		}
	}

	fetch = page_head->free_list.next;
	/* move entry from free_list to used_list */
	list_move(fetch, &page_head->used_list);

	if (list_empty(&page_head->free_list)) {
		list_move(&page_head->page_list, &page_list[page_head->index]);
	}

	irq_restore(flags);
	
	return (void *)((unsigned long)list_entry(fetch, obj_head_t, list)
		+ sizeof(obj_head_t));
}

void kfree(void *addr)
{
	unsigned long flags;
	obj_head_t *obj_head;
	page_head_t *page_head = (page_head_t *)PAGE_FLOOR(addr);

	WARN_ON(addr == NULL);
	if (addr == NULL)
		return;	

	irq_save(flags);
	obj_head = (obj_head_t *)((unsigned long)addr - sizeof(obj_head_t));
	list_move(&obj_head->list, &page_head->free_list);

	if (list_empty(&page_head->used_list)) {
		list_del(&page_head->page_list);
		free_page(page_head);
		irq_restore(flags);
		return;
	}

	list_move(&page_head->page_list, &page_list[page_head->index]);
	irq_restore(flags);
}




