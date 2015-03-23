#include <common.h>
#include <command.h>
#include <malloc.h>
#include <devices.h>
#include <version.h>
#include <net.h>
#include <asm/io.h>
#include <regs-lcd.h>
////////me added
#include <nand.h>
#include<image.h>
#include<zlib.h>
#include <asm/byteorder.h>

////////

const char version_string[]=
	U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ")"CONFIG_IDENT_STRING;

DECLARE_GLOBAL_DATA_PTR;
image_header_t iheader;
static struct tag* params;


#if (CONFIG_COMMANDS & CFG_CMD_NAND)
void nand_init (void);
#endif

#ifdef CONFIG_ONENAND
void onenand_init(void);
#endif

ulong monitor_flash_len;

static ulong mem_malloc_start = 0;
static ulong mem_malloc_end = 0;
static ulong mem_malloc_brk = 0;

static
void mem_malloc_init (ulong dest_addr)
{
	mem_malloc_start = dest_addr;
	mem_malloc_end = dest_addr + CFG_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;

}
void *sbrk (ptrdiff_t increment)
{
	ulong old =mem_malloc_brk;
	ulong new=old+increment;
	if((new<mem_malloc_start) || (new> mem_malloc_end))
	{
		return NULL;
	}
	mem_malloc_brk=new;
	return ((void*)old);
}



typedef int (init_fnc_t) (void);

int cpu_init(void)
{
	return 0;
}
/*dm9000移植部分*/

#define DM9000_Tacs     (0x0)   // 0clk         address set-up
#define DM9000_Tcos     (0x4)   // 4clk         chip selection set-up
#define DM9000_Tacc     (0xE)   // 14clk        access cycle
#define DM9000_Tcoh     (0x1)   // 1clk         chip selection hold
#define DM9000_Tah      (0x4)   // 4clk         address holding time
#define DM9000_Tacp     (0x6)   // 6clk         page mode access cycle
#define DM9000_PMC      (0x0)   // normal(1data)page mode configuration

void dm9000_pre_init(void)
{
	SROM_BW_REG &= ~(0xf << 4);
	SROM_BW_REG |= (1<<7) | (1<<6) | (1<<4);
	SROM_BC1_REG = ((DM9000_Tacs<<28)+(DM9000_Tcos<<24)+(DM9000_Tacc<<16)+(DM9000_Tcoh<<12)+(DM9000_Tah<<8)+(DM9000_Tacp<<4)+(DM9000_PMC));
}


/* 具体的参详 http://blog.csdn.net/muge0913/article/details/7176580  */
int board_init(void)
{
	dm9000_pre_init();

	gd->bd->bi_arch_number=MACH_TYPE;
	gd->bd->bi_boot_params = (PHYS_SDRAM_1 +0x100);    //0x50000010
	return 0;
}
#if 0
/*  分析见http://blog.chinaunix.net/uid-20799298-id-99651.html  */
int interrupt_init(void)    /* timer_init() */
{
	S3C64XX_TIMERS *const timers = S3C64XX_GetBase_TIMERS();

	timers->TCFG0=0x0f00;

	if (timer_load_val == 0) {
		/*
		 * for 10 ms clock period @ PCLK with 4 bit divider = 1/2
		 * (default) and prescaler = 16. Should be 10390
		 * @33.25MHz and 15625 @ 50 MHz
		 */
		timer_load_val = get_PCLK() / (2 * 16 * 100);
	}

	/* load value for 10 ms timeout */
	lastdec = timers->TCNTB4 = timer_load_val;
	/* auto load, manual update of Timer 4 */
	timers->TCON = (timers->TCON & ~0x00700000) | TCON_4_AUTO | TCON_4_UPDATE;
	/* auto load, start Timer 4 */
	timers->TCON = (timers->TCON & ~0x00700000) | TCON_4_AUTO | COUNT_4_ON;
	timestamp = 0;

	return (0);
}
#endif
int init_baudrate(void)
{
	gd->bd->bi_baudrate =115200;

	return (0);

}

int print_cpuinfo(void)
{
    printf("****************************************\r\n");
    printf("**    BootLoad-0.1                    **\r\n");
    printf("**    Updated for OK6410 	Board **\r\n");
    printf("**    Version (2015-2-23)            **\r\n");
    printf("**    Web: http://www.ecit.edu.cn   **\r\n");
    printf("****************************************\r\n");

    printf("\nCPU:     S3C6410 @%dMHz\n", get_ARMCLK()/1000000);
    printf("         Fclk = %dMHz, Hclk = %dMHz, Pclk = %dMHz",
           get_FCLK()/1000000, get_HCLK()/1000000, get_PCLK()/1000000);


    /**************
* Display Serial SRC
***************/

#if defined(CONFIG_CLKSRC_CLKUART)
    puts(", Serial = CLKUART ");
#else
    puts(", Serial = PCLK ");
#endif

    if(OTHERS_REG & 0x80)
        printf("(SYNC Mode) \n");
    else
        printf("(ASYNC Mode) \n");
    return 0;
}

int checkboard(void)
{
	printf("Board:   OK6410\n");
	return (0);
}


int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
        gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}
static int display_dram_config (void)
{
	int i;

	ulong size = 0;

	for (i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
	}

	puts("DRAM:    ");
	print_size(size, "\n");

	return (0);
}



init_fnc_t *init_sequence[] = {
	cpu_init,		/* basic cpu dependent setup */
	board_init,		/* basic board dependent setup */
	interrupt_init,		/* set up exceptions */

	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */

#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
	dram_init,		/* configure available RAM banks */
	display_dram_config,

	NULL,
};



static void setup_start_tag (bd_t *bd)
{
	params = (struct tag *) bd->bi_boot_params;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);
}

static void setup_memory_tags (bd_t *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);

		params->u.mem.start = bd->bi_dram[i].start;
		params->u.mem.size = bd->bi_dram[i].size;

		params = tag_next (params);
	}
}

static void setup_commandline_tag (bd_t *bd, char *commandline)
{
	char *p;

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++);

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;

	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;

	strcpy (params->u.cmdline.cmdline, p);

	params = tag_next (params);
}




static void setup_end_tag (bd_t *bd)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}






void start_armboot (void)
{
	init_fnc_t **init_fnc_ptr;
	char *s;


	ulong gd_base;

	gd_base = CFG_UBOOT_BASE + CFG_UBOOT_SIZE - CFG_MALLOC_LEN - CFG_STACK_SIZE - sizeof(gd_t);

	gd = (gd_t*)gd_base;
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	memset ((void*)gd, 0, sizeof (gd_t));
	gd->bd = (bd_t*)((char*)gd - sizeof(bd_t));
	memset (gd->bd, 0, sizeof (bd_t));

	monitor_flash_len = _bss_start - _armboot_start;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}
	bd_t *bd=gd->bd;

	/* armboot_start is defined in the board-specific linker script */

	mem_malloc_init (CFG_UBOOT_BASE + CFG_UBOOT_SIZE - CFG_MALLOC_LEN - CFG_STACK_SIZE);



#if (CONFIG_COMMANDS & CFG_CMD_NAND)
	puts ("NAND:    ");
	nand_init();		/* go init the NAND */
#endif

	/* IP Address */
	gd->bd->bi_ip_addr = 0xc0a82345;   //192.168.35.69



	/* MAC Address */
	gd->bd->bi_enetaddr[0]=0x00;
	gd->bd->bi_enetaddr[1]=0x40;
	gd->bd->bi_enetaddr[2]=0x5c;
	gd->bd->bi_enetaddr[3]=0x26;
	gd->bd->bi_enetaddr[4]=0x0a;
	gd->bd->bi_enetaddr[5]=0x5b;


	devices_init ();	/* get the devices list going. */


	jumptable_init ();

	enable_interrupts ();

	puts("11-26/4-33\n");

		
//	run_command("nand read 0xc0008000 0x200000 0x500000",0);
extern nand_info_t nand_info[];     //nand 操作的接口
extern int nand_curr_device;         //下载到内存的地址
	nand_info_t *nand;
	nand=&nand_info[nand_curr_device];
	nand_read_options_t opts;
	memset(&opts,0,sizeof(opts));
	opts.buffer= (u_char*)(0xc0008000);
	opts.length=0x500000;
	opts.offset=0x200000;
	opts.quiet=0;
	nand_read_opts(nand,&opts);
	printf("kernel is loaded:%x\n",(ulong)opts.buffer);


	//run_command("bootm 0xc0008000",0);
//do_bootm();
	image_header_t *hdr = &iheader;
#define LINUX_ZIMAGE_MAGIC	0x016f2818
	ulong addr=0xc0008000;
	if (*(ulong *)(addr + 9*4) == LINUX_ZIMAGE_MAGIC) {
		printf("Boot with zImage\n");
		addr = virt_to_phys(addr);
		hdr->ih_os = IH_OS_LINUX;
		hdr->ih_ep = ntohl(addr);
	}
//do_bootm_linux
	char *commandline="root=/dev/mtdblock2 rootfstype=yaffs2 init=/linuxrc console=ttySAC0,115200";
	void (*theKernel)(int zero, int arch, uint params);

	theKernel = (void (*)(int, int, uint))ntohl(hdr->ih_ep);
	printf("\naddr=%x,hdr=%x,,theKernel=%x\n",addr,(int)hdr,(ulong)theKernel);
//ATAG
	setup_start_tag (bd);


	setup_memory_tags (bd);

	setup_commandline_tag (bd, commandline);


	setup_end_tag (bd);


	/* we assume that the kernel is in place */
	printf ("\nStarting kernel ...\n\n");

	cleanup_before_linux ();

	theKernel (0, bd->bi_arch_number, bd->bi_boot_params);

	hang();

	/* NOTREACHED - no way out of command loop except booting */
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}

