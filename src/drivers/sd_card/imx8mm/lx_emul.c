/*
 * \brief  Driver-specific emulation of Linux Kernel functions
 * \author Stefan Kalkowski
 * \author Jean-Adrien Domage
 * \date   2021-10-01
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul.h>


#include <linux/gfp.h>

struct page * __alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order,
                                     int preferred_nid, nodemask_t * nodemask)
{
	unsigned const num_pages = (1 << order);
	void *   const ptr = lx_emul_mem_alloc_aligned(PAGE_SIZE*num_pages, PAGE_SIZE);
	return lx_emul_virt_to_pages(ptr, num_pages);
}


#include <asm-generic/delay.h>
#include <linux/dma-mapping.h>
#include <linux/types.h>

void vfree(const void *addr)
{
	(void)addr;
	lx_emul_trace(__func__);
}

void * dmam_alloc_attrs(struct device * dev,size_t size,dma_addr_t * dma_handle,gfp_t gfp,unsigned long attrs)
{
	return dma_alloc_attrs(dev, size, dma_handle, gfp, attrs);
}


#include <linux/fs.h>

int alloc_chrdev_region(dev_t * dev,unsigned baseminor,unsigned count,const char * name)
{
	static dev_t counter = 0;
	*dev = counter++;
	return 0;
}


extern int cache_line_size(void);
int cache_line_size(void)
{
	return 128;
}


#include <linux/blkdev.h>
#include <linux/sched.h>

long __sched io_schedule_timeout(long timeout)
{
	long ret;
	int  old_iowait = current->in_iowait;

	current->in_iowait = 1;
	blk_schedule_flush_plug(current);
	ret = schedule_timeout(timeout);
	current->in_iowait = old_iowait;

	return ret;
}

void __sched io_schedule(void)
{
	io_schedule_timeout(10);
}

#include <linux/nodemask.h>

nodemask_t node_states[NR_NODE_STATES] __read_mostly = {
	[N_POSSIBLE] = NODE_MASK_ALL,
	[N_ONLINE] = { { [0] = 1UL } },
#ifndef CONFIG_NUMA
	[N_NORMAL_MEMORY] = { { [0] = 1UL } },
#ifdef CONFIG_HIGHMEM
	[N_HIGH_MEMORY] = { { [0] = 1UL } },
#endif
	[N_MEMORY] = { { [0] = 1UL } },
	[N_CPU] = { { [0] = 1UL } },
#endif /* NUMA */
};


#include <linux/nodemask.h>

unsigned int nr_node_ids = MAX_NUMNODES;
