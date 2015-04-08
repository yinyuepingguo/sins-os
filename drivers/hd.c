/* NOTE: THIS HD driver is not strong. */

#include <sins/init.h>
#include <sins/kernel.h>
#include <sins/fs.h>
#include <asm/io.h>
#include <sins/sched.h>
#include <sins/processor.h>
#include <bitops.h>
#include "hdreg.h"
#include <sins/boot_params.h>

static struct hd_i_struct{
	int head,sect,cyl,wpcom,lzone,ctl;
	} hd_info[NR_DRIVES];

static WAIT_QUEUE(wait_hd);
static WAIT_QUEUE(wait_done);

static struct hd_struct {
	long start_sect;
	long nr_sects;
	unsigned char enable;
} hd[5*NR_DRIVES]={{0,0,0},};

static void hd_out(unsigned int drive,unsigned int nsect,unsigned int sect,
		unsigned int head,unsigned int cyl,unsigned int cmd,
		void (*intr_addr)(void), struct block *blk);
static void (*do_hd)(void) = NULL;
static struct block *wait_blk = NULL;


static int win_result(void)
{
	int i=inb(HD_STATUS);

	if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT
			| SEEK_STAT | ERR_STAT))
		== (READY_STAT | SEEK_STAT))
		return(0); /* ok */
	if (i&1) i=inb(HD_ERROR);
	return (1);
}

static int intr_done = 0;

void unexpected_hd_interrupt(void)
{
	panic("Unexpected HD interrupt");
}

static int controller_ready(void)
{
	int retries=1000;

	while (--retries && (inb(HD_STATUS)&0xc0)!=0x40);
	return (retries);
}


static void read_intr(void)
{
	if (win_result()) {
		BUG("bad_read_intr");
		return;
	}
	if (inb(HD_STATUS) & DRQ_STAT && intr_done != 2048) {
		port_read(HD_DATA, wait_blk->data + intr_done*2, 256);
		intr_done +=256;
	}
	if (intr_done == 2048) {
		wait_blk->uptodate = 1;
		wake_up(&wait_done);
		wake_up(&wait_hd);
	}
}

static void write_intr(void)
{
	if (win_result()) {
		BUG("bad_write_intr");
		return;
	}
	if (inb(HD_STATUS) & DRQ_STAT && intr_done != 2048) {
		port_write(HD_DATA, wait_blk->data + intr_done*2, 256);
		intr_done +=256;
	}
	if (intr_done == 2048) {
		wait_blk->dirty = 0;
		wake_up(&wait_done);
		wake_up(&wait_hd);
	}
}



static void hd_out(unsigned int drive,unsigned int nsect,unsigned int sect,
		unsigned int head,unsigned int cyl,unsigned int cmd,
		void (*intr_addr)(void), struct block *blk)
{
	int port;
	unsigned long flags;

	if (drive>1 || head>15)
		panic("Trying to write bad sector");

	irq_save(flags);

	wait_event(&wait_hd, list_empty(&wait_done));

	if (!controller_ready())
		panic("HD controller not ready");

	intr_done = 0;
	do_hd = intr_addr;
	wait_blk = blk;
	outb(hd_info[drive].ctl, HD_CMD);
	port=HD_DATA;
	outb_p(hd_info[drive].wpcom, ++port);
	outb_p(nsect,++port);
	outb_p(sect,++port);
	outb_p(cyl,++port);
	outb_p(cyl>>8,++port);
	outb_p(0xA0|(drive<<4)|head,++port);
	outb(cmd,++port);
	wait(&wait_done);
	irq_restore(flags);
}

void rw_abs_hd(int rw, unsigned long nr, unsigned long sec,
	unsigned long head, unsigned long cyl, struct block *blk)
{
	unsigned long cmd;

	if (rw == BLK_WRITE) {
		cmd = WIN_WRITE;
		hd_out(nr , BLOCK_SIZE/512, sec, head, cyl, cmd, &write_intr, blk);
	} else if (rw == BLK_READ) {
		cmd = WIN_READ;
		hd_out(nr , BLOCK_SIZE/512, sec, head, cyl, cmd, &read_intr, blk);
	} else {
		panic("unknown hd-command");
	}
}


static void hd_rw_handler(int rw, struct block *blk)
{
	unsigned long block, dev;
	unsigned long sec, head, cyl;
	
	block = blk->block_nr * (BLOCK_SIZE / 512);
	dev = MINOR(blk->dev);

	if (dev >= 5*NR_DRIVES || !hd[dev].enable ||
		block + BLOCK_SIZE/512 > hd[dev].nr_sects) 
		panic("hd parameters invalid!");

	block += hd[dev].start_sect;
	dev /= 5;	
	__asm__("divl %4":"=a" (block),"=d" (sec):"0" (block),"1" (0),
		"r" (hd_info[dev].sect));
	__asm__("divl %4":"=a" (cyl),"=d" (head):"0" (block),"1" (0),
		"r" (hd_info[dev].head));
	rw_abs_hd(rw, dev, sec + 1, head, cyl, blk);
}

static void hd_intr()
{
	BUG_ON(do_hd == NULL);
	do_hd();
}

static void partition()
{
	unsigned long i, j;
	struct block *blk;
	struct partition *p;

	for (i = 0; i < boot_params.drives_length; ++i) {
		hd_info[i].head = boot_params.drives[i].heads;
		hd_info[i].cyl = boot_params.drives[i].cylinders;
		hd_info[i].sect = boot_params.drives[i].sectors;
		hd_info[i].wpcom = -1;
		hd_info[i].lzone = 920;
		hd_info[i].ctl = 0x00;
		if (hd_info[i].head > 8)
			set_bit(hd_info[i].ctl, 3);

		printk("drive[%d]: %d cylinders, %d heads, %d sectors\n",
			i, hd_info[i].cyl, hd_info[i].head, hd_info[i].sect);

		hd[i*5].start_sect = 0;
		hd[i*5].nr_sects =
			hd_info[i].head * hd_info[i].sect * hd_info[i].cyl;
		hd[i*5].enable = 1;
		blk = bread(MKDEV(DEV_HD, i*5), 0);

		if (blk == NULL) {
			panic("Unable to read partition table for drive[%d]",
				i);	
		}
		if (blk->data[510] != 0x55 ||
			blk->data[511] != 0xAA) {
			panic("Bad Partition table on drive[%d]", i);
		}
		p = (struct partition *)(0x1BE + blk->data);
		for (j = 1; j < 5; j++, p++) {
			if (p->start_sect == 0 && p->nr_sects == 0)
				continue;
			hd[j+5*i].start_sect = p->start_sect;
			hd[j+5*i].nr_sects = p->nr_sects;
			hd[j+5*i].enable = 1;
			printk("drive[%d]:start sector = %d,"
				" sectors = %d, size = %dMB\n",
				i , p->start_sect, p->nr_sects,
					p->nr_sects/2048);
		}
		brelse(blk);
	}		
	printk("Partition table%s ok.\n",
		(boot_params.drives_length>1)?"s":"");
}

static result_t hd_setup()
{
	result_t ret;

	ret = request_irq(IRQ_AT_WINI, hd_intr, "hd");
	if (FAILED(ret))
		return ret;
	enable_irq(IRQ_AT_WINI);
	enable_irq(IRQ_CASCADE);
	ret = register_blkdev(DEV_HD, "hd", hd_rw_handler, 512);
	if (FAILED(ret))
		return ret;

	partition();
	mount_root(MKDEV(DEV_HD, 1));

	return SUCCESS;
}

device_initcall(hd_setup);
