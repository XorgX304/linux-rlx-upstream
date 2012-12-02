/*
 * Copyright 2006, Realtek Semiconductor Corp.
 *
 * arch/rlx/rlxocp/prom.c
 *   Early initialization code for the RLX OCP Platform
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 7, 2006
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <asm/page.h>
#include <asm/cpu.h>

//#include <asm/rlxbsp.h>

#include <bspcpu.h>
#include <bspchip.h>


extern char arcs_cmdline[];

#ifdef CONFIG_EARLY_PRINTK
static int promcons_output __initdata = 0;
                
__init void unregister_prom_console(void)
{
	if (promcons_output) {
		promcons_output = 0;
	}
}
                                                                                                    
void disable_early_printk(void)
	__attribute__ ((alias("unregister_prom_console")));
#endif
                                                                                                    

const char *get_system_type(void)
{
	return "RTL8196C";
}



/* Do basic initialization */
void __init prom_meminit(void)
{
	u_long mem_size;
       	/*now: always believe DRAM configuration register*/
        {
                unsigned int DCRvalue = 0;
                unsigned int bus_width = 0, chip_sel = 0, row_cnt = 0, col_cnt = 0,bank_cnt = 0;
 
                DCRvalue = ( (*(volatile unsigned int *)BSP_MC_MTCR0));
 
                /*bit 19,0:2 bank; 1: 4 bank*/
                switch(DCRvalue & 0x080000)
                {
		case 0x0:
			bank_cnt = 2;
			break;
		case 0x080000:
			bank_cnt = 4;
			break;
		default:
			bank_cnt = 0;
			break;
                }
 
                /*bit 22~24: colomn count*/
                switch(DCRvalue & 0x01C00000)
                {
		case 0x00000000:
			col_cnt = 256;
			break;
		case 0x00400000:
			col_cnt = 512;
			break;
		case 0x00800000:
			col_cnt = 1024;
			break;
		case 0x00C00000:
			col_cnt = 2048;
			break;
		case 0x01000000:
			col_cnt = 4096;
			break;
		default:
			printk("unknow column count(0x%x)\n",DCRvalue & 0x01C00000);
			break;
                }
 
                /*bit 25~26: row count*/
                switch(DCRvalue & 0x06000000)
                {
		case 0x00000000:
			row_cnt = 2048;
			break;
		case 0x02000000:
			row_cnt = 4096;
			break;
		case 0x04000000:
			row_cnt = 8192;
			break;
		case 0x06000000:
			row_cnt = 16384;
			break;
                }
 
                /*bit 27: chip select*/
                switch(DCRvalue & 0x08000000)
                {
		case 0x0:
			chip_sel = 1;
			break;
		case 0x08000000:
			chip_sel = 2;
			break;
                }
 
                /*bit 28~29: bus width*/
                switch(DCRvalue & 0x30000000)
                {
		case 0x0:
			bus_width = 8;
			break;
		case 0x10000000:
			bus_width = 16;
			break;
		case 0x20000000:
			bus_width = 32;
			break;
		default:
			printk("bus width is reseved!\n");
			break;
                }
 
                /*total size(Byte)*/
                mem_size = (row_cnt * col_cnt *bank_cnt) * (bus_width >> 3) * chip_sel;     
 
        }
	mem_size = cpu_mem_size;
	add_memory_region(0, mem_size, BOOT_MEM_RAM);
}

#define UART0_BASE		0xB8002000
#define UART0_THR		(UART0_BASE + 0x000)
#define UART0_FCR		(UART0_BASE + 0x008)
#define UART0_LSR       (UART0_BASE + 0x014)
#define TXRST			0x04
#define CHAR_TRIGGER_14	0xC0
#define LSR_THRE		0x20
#define TxCHAR_AVAIL	0x00
#define TxCHAR_EMPTY	0x20


void  __init prom_console_init(void)
{
	/* Let's assume bootloader has done the job */
}


void prom_putchar(char c)
{
	unsigned int busy_cnt = 0;

	do
	{
		/* Prevent Hanging */
		if (busy_cnt++ >= 30000)
		{
			/* Reset Tx FIFO */
			REG8(UART0_FCR) = TXRST | CHAR_TRIGGER_14;
			return;
		}
	} while ((REG8(UART0_LSR) & LSR_THRE) == TxCHAR_AVAIL);

	/* Send Character */
	REG8(UART0_THR) = c;
}



void __init prom_free_prom_memory(void)
{
	return;
}

void __init prom_init(void)
{
   prom_console_init();
   prom_meminit();
}
