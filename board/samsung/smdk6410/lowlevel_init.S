
#include <config.h>
#include <version.h>

#include <s3c6410.h>

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r12, lr

	ldr	r0, =ELFIN_GPIO_BASE
	ldr     r1, =0x55555555	
	str     r1, [r0, #GPKCON0_OFFSET]

	ldr     r1, =0x55555555
	str     r1, [r0, #GPKCON1_OFFSET]

	ldr     r1, =0x22222666
	str     r1, [r0, #GPLCON0_OFFSET]
	
	ldr     r1, =0x04000000
	str     r1, [r0, #GPFCON_OFFSET]
        ldr     r1, =0x2000
	str     r1, [r0, #GPFDAT_OFFSET]
	


	/* LED on only #8 */
	ldr	r0, =ELFIN_GPIO_BASE  
	ldr	r1, =0x00111111
	str	r1, [r0, #GPMCON_OFFSET]

	ldr	r1, =0x00000555
	str	r1, [r0, #GPMPUD_OFFSET]

	ldr	r1, =0xc           /*led 34不亮*/
	str	r1, [r0, #GPMDAT_OFFSET]



	ldr	r1, =0  /*0x55555555 phantom*/
	str	r1, [r0, #MEM1DRVCON_OFFSET]

	/* Disable Watchdog */
	ldr	r0, =0x7e000000		@0x7e004000
	orr	r0, r0, #0x4000
	mov	r1, #0
	str	r1, [r0]

	@ External interrupt pending clear
	ldr	r0, =(ELFIN_GPIO_BASE+EINTPEND_OFFSET)	/*EINTPEND*/
	ldr	r1, [r0]
	str	r1, [r0]

	ldr	r0, =ELFIN_VIC0_BASE_ADDR 	@0x71200000
	ldr	r1, =ELFIN_VIC1_BASE_ADDR 	@0x71300000

	@ Disable all interrupts (VIC0 and VIC1)
	mvn	r3, #0x0
	str	r3, [r0, #oINTMSK]
	str	r3, [r1, #oINTMSK]

	@ Set all interrupts as IRQ
	mov	r3, #0x0
	str	r3, [r0, #oINTMOD]
	str	r3, [r1, #oINTMOD]

	@ Pending Interrupt Clear
	mov	r3, #0x0
	str	r3, [r0, #oVECTADDR]
	str	r3, [r1, #oVECTADDR]

	/* init system clock */


	bl system_clock_init


	/* for UART */
	bl uart_asm_init






ldr	r0, =ELFIN_UART_BASE
ldr	r1, =0x4b4b4b4b
str	r1, [r0, #UTXH_OFFSET]



	bl nand_asm_init

	bl	mem_ctrl_asm_init
#if 1
        ldr     r0, =(ELFIN_CLOCK_POWER_BASE+RST_STAT_OFFSET)
        ldr     r1, [r0]
        bic     r1, r1, #0xfffffff7
        cmp     r1, #0x8
        beq     wakeup_reset

#endif

1:
	ldr	r0, =ELFIN_UART_BASE
	ldr	r1, =0x4b4b4b4b
	str	r1, [r0, #UTXH_OFFSET]
	
	

	mov	lr, r12
	mov	pc, lr
#if 1
wakeup_reset:

	/*Clear wakeup status register*/
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+WAKEUP_STAT_OFFSET)
	ldr	r1, [r0]
	str	r1, [r0]

        /*LED test*/
        ldr     r0, =ELFIN_GPIO_BASE
        ldr     r1, =0x3000
        str     r1, [r0, #GPNDAT_OFFSET]

	/*Load return address and jump to kernel*/
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+INF_REG0_OFFSET)
	ldr	r1, [r0]	/* r1 = physical address of s3c6400_cpu_resume function*/
	mov	pc, r1		/*Jump to kernel (sleep-s3c6400.S)*/
	nop
	nop
#endif



system_clock_init:
	ldr	r0, =ELFIN_CLOCK_POWER_BASE	@0x7e00f000


	ldr	r1, [r0, #OTHERS_OFFSET]
	mov	r2, #0x40
	orr	r1, r1, r2
	str	r1, [r0, #OTHERS_OFFSET]

	nop
	nop
	nop
	nop
	nop

	ldr	r2, =0x80
	orr	r1, r1, r2
	str	r1, [r0, #OTHERS_OFFSET]

check_syncack:
	ldr	r1, [r0, #OTHERS_OFFSET]
	ldr	r2, =0xf00
	and	r1, r1, r2
	cmp	r1, #0xf00
	bne	check_syncack

	mov	r1, #0xff00
	orr	r1, r1, #0xff
	str	r1, [r0, #APLL_LOCK_OFFSET]
	str	r1, [r0, #MPLL_LOCK_OFFSET]
	str	r1, [r0, #EPLL_LOCK_OFFSET]

#if defined(CONFIG_CLKSRC_CLKUART)
	ldr   	r1, [r0, #CLK_DIV2_OFFSET]
	bic	r1, r1, #0x70000
	orr	r1, r1, #0x30000
	str	r1, [r0, #CLK_DIV2_OFFSET]

#endif

	ldr   	r1, [r0, #CLK_DIV0_OFFSET]	/*Set Clock Divider*/
	bic	r1, r1, #0x30000
	bic	r1, r1, #0xff00
	bic	r1, r1, #0xff
	ldr	r2, =CLK_DIV_VAL
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV0_OFFSET]

	ldr	r1, =APLL_VAL
	str	r1, [r0, #APLL_CON_OFFSET]
	ldr	r1, =MPLL_VAL
	str	r1, [r0, #MPLL_CON_OFFSET]

	ldr	r1, =0x80200203			/* FOUT of EPLL is 96MHz */
	str	r1, [r0, #EPLL_CON0_OFFSET]
	ldr	r1, =0x0
	str	r1, [r0, #EPLL_CON1_OFFSET]

	ldr	r1, [r0, #CLK_SRC_OFFSET]	/* APLL, MPLL, EPLL select to Fout */

#if defined(CONFIG_CLKSRC_CLKUART)
	ldr	r2, =0x2007
#else
	ldr	r2, =0x7
#endif
	orr	r1, r1, r2

	str	r1, [r0, #CLK_SRC_OFFSET]

	/* wait at least 200us to stablize all clock */
	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b


			/* Synchronization for VIC port */
	ldr	r1, [r0, #OTHERS_OFFSET]
	orr	r1, r1, #0x20
	str	r1, [r0, #OTHERS_OFFSET]

	mov	pc, lr


/*
 * uart_asm_init: Initialize UART in asm mode, 115200bps fixed.
 * void uart_asm_init(void)
 */
uart_asm_init:
	/* set GPIO to enable UART */
	@ GPIO setting for UART
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x220022
	str   	r1, [r0, #GPACON_OFFSET]

	ldr	r1, =0x2222
	str   	r1, [r0, #GPBCON_OFFSET]

	ldr	r0, =ELFIN_UART_CONSOLE_BASE		@0x7F005000
	mov	r1, #0x0
	str	r1, [r0, #UFCON_OFFSET]
	str	r1, [r0, #UMCON_OFFSET]

	mov	r1, #0x3                	@was 0.
	str	r1, [r0, #ULCON_OFFSET]


#if defined(CONFIG_CLKSRC_CLKUART)
	ldr	r1, =0xe45			/* UARTCLK SRC = 11 => EXT_UCLK1*/
#else
	ldr	r1, =0x245			/* UARTCLK SRC = x0 => PCLK */
#endif

	str	r1, [r0, #UCON_OFFSET]

#if defined(CONFIG_UART_50)
	ldr	r1, =0x1A
#elif defined(CONFIG_UART_66)
	ldr	r1, =0x22
#else
	ldr	r1, =0x1A
#endif
	str	r1, [r0, #UBRDIV_OFFSET]

#if defined(CONFIG_UART_50)
	ldr	r1, =0x3
#elif defined(CONFIG_UART_66)
	ldr	r1, =0x1FFF
#else
	ldr	r1, =0x3
#endif
	str	r1, [r0, #UDIVSLOT_OFFSET]

	ldr	r1, =0x4f4f4f4f
	str	r1, [r0, #UTXH_OFFSET]		@'O'

	mov	pc, lr

/*
 * Nand Interface Init for SMDK6400 */
nand_asm_init:
	ldr	r0, =ELFIN_NAND_BASE
	ldr	r1, [r0, #NFCONF_OFFSET]
	orr	r1, r1, #0x70
	orr	r1, r1, #0x7700
	str     r1, [r0, #NFCONF_OFFSET]

	ldr	r1, [r0, #NFCONT_OFFSET]
	orr	r1, r1, #0x03
	str     r1, [r0, #NFCONT_OFFSET]

	mov	pc, lr


.macro FL_SECTION_ENTRY base,ap,d,c,b
	.word (\base << 20) | (\ap << 10) | (\d << 5) | (1<<4) | (\c << 3) | (\b << 2) | (1<<1)
.endm          /*  first-level desc  */

.section .mmudata, "a"
	.align 14         /* 16K 对齐 */
	// the following alignment creates the mmu table at address 0x4000.
	.globl mmu_table
mmu_table:
	.set __base,0
	// 1:1 mapping for debugging   /* 在小于0xa0000000的地址都是他们自己（相当于不做映射）所以0x5fe00000映射后还是0x5fe00000 */
	.rept 0xA00         /*2.5K个描述符 每个描述符占4个字节，占10K内存*/
	FL_SECTION_ENTRY __base,3,0,0,0
	/* 代入公式得 FL_SECTION_ENTRY(起始) = (0b11 << 10) | (0b1 << 4) | (0b1 << 1)    =  0x0000_4000（对齐后 ???）  */
 	.set __base,__base+1
	.endr

	// access is not allowed.
	.rept 0xC00 - 0xA00      /* 同上的算法 */
	.word 0x00000000          /*  @从0xa00-0xc00共512个描述符，ap为0表示不允许访问。  在虚拟地址0xa0000000到地址0xc000000的地址相当于不做映射，但是任何访问都将导致数据异常终止*/
	.endr


        //256MB for SDRAM 0xC0000000 -> 0x50000000   将虚拟地址0xc0000000 映射到 0x50000000这里 当然也可说访问0xcxxxxxxx都将是访问物理的0x5xxxxxxx
	.set __base, 0x500
    .rept 0xD00 - 0xC00    /*@从0xd00－0xc00共256个描述符（因为每个描述符代表1M，就是256MB）定义，cb比特为11表示内存输入输出都为回写模式 */
    
    
	FL_SECTION_ENTRY __base,3,0,1,1      /* section base =0x5000 0000 */
	.set __base,__base+1
	.endr

        /* access is not allowed.*/
    .rept 0x1000 - 0xD00    /* 4k个（4096描述符，每个描述符可以表示1M的地址空间），在内存中的大小事16K(0x4000)内存 任何访问0xd0000000以上的虚拟地址都是错误的,会导致数据异常终止（data_abort）的错误 */

	.word 0x00000000   /* AP为0，对这里的任何访问都是错误的 */
	.endr
/*  经过计算.mmudata的地址范围是0xcfe30000~0xcfe34000 */
