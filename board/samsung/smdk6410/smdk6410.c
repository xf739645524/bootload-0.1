/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <regs.h>

/* ------------------------------------------------------------------------- */
#define CS8900_Tacs	(0x0)	// 0clk		address set-up
#define CS8900_Tcos	(0x4)	// 4clk		chip selection set-up
#define CS8900_Tacc	(0xE)	// 14clk	access cycle
#define CS8900_Tcoh	(0x1)	// 1clk		chip selection hold
#define CS8900_Tah	(0x4)	// 4clk		address holding time
#define CS8900_Tacp	(0x6)	// 6clk		page mode access cycle
#define CS8900_PMC	(0x0)	// normal(1data)page mode configuration

static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n" "bne 1b":"=r" (loops):"0"(loops));
}


#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_smdk6410(ulong addr)
{
#ifdef FORLINX_BOOT_RAM128
        if ((0xc0000000 <= addr) && (addr < 0xc8000000)) //256M memory, modify by lxj 2011.08.22
#else  //256M
        if ((0xc0000000 <= addr) && (addr < 0xD0000000)) //256M memory, modify by lxj 2011.08.22
#endif
		return (addr - 0xc0000000 + 0x50000000);
        else
                printf("do not support this address : %08lx\n", addr);

	return addr;
}
#endif


