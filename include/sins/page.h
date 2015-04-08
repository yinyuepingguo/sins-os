#ifndef _SINS_PAGE_H
#define _SINS_PAGE_H

#include <types.h>
#include <asm/page.h>

#define PAGE_OFFSET 0xC0000000

#define PAGE_SHIFT 12
#define PGD_SHIFT 22

#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PGD_SIZE (1 << PGD_SHIFT)

#define PAGE_MASK (~(PAGE_SIZE - 1))
#define PGD_MASK (~(PGD_SIZE - 1))
#define PGT_MASK (PAGE_MASK ^ PGD_MASK)

#define PAGE_ALIGN(addr) \
	(((unsigned long)(addr) + PAGE_SIZE - 1) & PAGE_MASK)
#define PAGE_FLOOR(addr) ((unsigned long)(addr) & PAGE_MASK)
#define PAGE_CEIL(addr) PAGE_ALIGN(addr)


#define PTRS_PER_PGT 1024
#define PTRS_PER_PGD 1024

typedef struct { u32 pgt;} pgt_t;
typedef struct { u32 pgd; } pgd_t;
typedef u32 pgprot_t;


#define page_offset(addr) ((unsigned long)(addr) & ~PAGE_MASK)
#define pgt_index(addr)	\
	(((unsigned long)(addr) & PGT_MASK) >> PAGE_SHIFT)
#define pgd_index(addr)	\
	 (((unsigned long)(addr) & PGD_MASK) >> PGD_SHIFT)

#define va(addr) ((void *)((unsigned long)(addr) + PAGE_OFFSET))
#define pa(addr) ((void *)((unsigned long)(addr) - PAGE_OFFSET))

#define PAGE_PRESENT	(1 << 0)
#define PAGE_RW	 	(1 << 1)
#define PAGE_USER	(1 << 2)
#define PAGE_ACCESSED	(1 << 5)
#define PAGE_DIRTY	(1 << 6)

#define PAGE_ADDR_MASK (~((1 << 12) - 1))
#define PAGE_FLAGS_MASK (((1 << 12) - 1))


static inline pgt_t pgt_modify(pgt_t pgt, pgprot_t prot)
{
	return (pgt_t){(((pgt.pgt) & ~PAGE_FLAGS_MASK) | prot)};
}

static inline pgd_t pgd_modify(pgd_t pgd, pgprot_t prot)
{
	return (pgd_t){(((pgd.pgd) & ~PAGE_FLAGS_MASK) | prot)};
}

static inline result_t pgt_check(pgt_t pgt, pgprot_t prot)
{
	return (((pgt.pgt) & PAGE_FLAGS_MASK) & prot);
}

static inline result_t pgd_check(pgd_t pgd, pgprot_t prot)
{
	return (((pgd.pgd) & PAGE_FLAGS_MASK) & prot);
}

static inline pgt_t mk_pgt(unsigned long addr, pgprot_t prot)
{
	return (pgt_t){(addr | prot)};
}
static inline pgd_t mk_pgd(unsigned long addr, pgprot_t prot)
{
	return (pgd_t){(addr | prot)};
}

#define pgt_present(pgt) pgt_check(pgt, PAGE_PRESENT)
#define pgd_present(pgd) pgd_check(pgd, PAGE_PRESENT)


/* get address contains*/
static inline unsigned long pgd_page(pgd_t pgd)
{
	return (pgd.pgd & PAGE_ADDR_MASK);
}

static inline unsigned long pgt_page(pgt_t pgt)
{
	return (pgt.pgt & PAGE_ADDR_MASK);
}

#define switch_pgdir(page_dir) arch_switch_pgdir(pa(page_dir))


/* Pure 2^n version of get_order */
static inline int get_order(unsigned long size)
{
        int order;

        size = (size-1) >> (PAGE_SHIFT-1);
        order = -1;
        do {
                size >>= 1;
                order++;
        } while (size);
        return order;
}

static inline int get_order_floor(unsigned long size)
{
        int order;

        size = size >> PAGE_SHIFT;
        order = -1;
        do {
                size >>= 1;
                order++;
        } while (size);
        return order;
}

#endif
