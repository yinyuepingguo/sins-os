#include <sins/mm.h>
#include <sins/sched.h>
#include <types.h>
#include <string.h>
#include <sins/error.h>

static inline struct page *get_avail_pages(unsigned long order)
{
	struct page *p = NULL;

	p = alloc_pages(ZONE_HIGHMEM, order);
	if (!p)
		p = alloc_pages(ZONE_NORMAL, order);
	if (!p)
		p = alloc_pages(ZONE_DMA, order);
	return p;
}

pgd_t *copy_empty_page_dir()
{
	pgd_t *src = kernel_page_dir;
	pgd_t *dest = get_free_page(GFP_NORMAL | GFP_DMA);
	
	if (dest == NULL)
		return NULL;

	memcpy(dest, src, PAGE_SIZE);
	return dest;
}

void free_page_dir(pgd_t *pgd)
{
	unsigned long di;
	unsigned long ti;
	pgt_t *pgt;
	struct page *p;
	unsigned long count = 0;

	for (di = 0; di != PAGE_OFFSET/PGD_SIZE; ++di)	{
		if (pgd_present(pgd[di])) {
			pgt = va(pgd_page(pgd[di]));
			for (ti = 0; ti != PTRS_PER_PGT; ++ti) {
				if (pgt_present(pgt[ti])) {
					p = phys_to_page(pgt_page(pgt[ti]));
					dealloc_pages(p, 0);
					count++;
				}
			}
			free_page(pgt);
		}
	}
	
	free_page(pgd);
}

#define USER_PAGE_MASK (PAGE_PRESENT | PAGE_RW | PAGE_USER)

/* only setup pages for user address space */
struct page *load_page(pgd_t *pgd, unsigned long start_addr)
{
	struct page *pages;
	pgt_t *now_pgt;
	unsigned long pgd_i;
	unsigned long total_count = 1;
	unsigned long now_count = 0;
	unsigned long now_addr;
	pgt_t *pgte;
	unsigned long real_addr;

	/* start_addr must align to PAGE_SIZE */
	BUG_ON(page_offset(start_addr) != 0);
	if (start_addr + PAGE_SIZE > PAGE_OFFSET)
		return NULL;
	pages = get_avail_pages(0);
	if (pages == NULL)
		return NULL;

	now_addr = start_addr + now_count*PAGE_SIZE;
	/* if pgd is not PRESENT, allocate page for pgd */
	pgd_i = pgd_index(now_addr);	
	if (!pgd_present(pgd[pgd_i])) {
		now_pgt = (pgt_t *)get_zeroed_page(GFP_NORMAL|GFP_DMA);
		if (now_pgt == NULL)
			goto failed;
		pgd[pgd_i] = mk_pgd((unsigned long)pa(now_pgt)
						,USER_PAGE_MASK);
	} else {
		now_pgt = (pgt_t *)va(pgd_page(pgd[pgd_i])); 
	}
	pgte = &now_pgt[pgt_index(now_addr)];
	BUG_ON(pgt_present(*pgte));
	real_addr = (unsigned long)page_to_phys(pages + now_count);
	*pgte = mk_pgt(real_addr, USER_PAGE_MASK);
	return pages;
failed:
	dealloc_pages(pages, 0);
	total_count = now_count;
	now_count = 0;
	while(now_count != total_count) {
		now_addr = start_addr + now_count*PAGE_SIZE;
		pgd_i = pgd_index(now_addr);
		free_page(va(pgd_page(pgd[pgd_i])));
		++now_count;
	}
	return NULL;
}
