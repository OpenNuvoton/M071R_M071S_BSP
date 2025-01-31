/**************************************************************************//**
 * @file     main.c
 * @version  V2.00
 * @brief    M071R_M071S Series GPIO Driver Sample Code
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"


#define PLLCON_SETTING  CLK_PLLCON_72MHz_HXT
#define PLL_CLOCK       72000000


void SYS_Init(void)
{
	uint32_t u32TimeOutCnt;

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC 22.1184MHz clock */
    CLK->PWRCON |= CLK_PWRCON_OSC22M_EN_Msk;

    /* Waiting for Internal RC clock ready */
    u32TimeOutCnt = __HIRC;
    while(!(CLK->CLKSTATUS & CLK_CLKSTATUS_OSC22M_STB_Msk))
		if(--u32TimeOutCnt == 0) break;

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK->CLKSEL0 = (CLK->CLKSEL0 & (~CLK_CLKSEL0_HCLK_S_Msk)) | CLK_CLKSEL0_HCLK_S_HIRC;
    CLK->CLKDIV = (CLK->CLKDIV & (~CLK_CLKDIV_HCLK_N_Msk)) | CLK_CLKDIV_HCLK(1);

    /* Set PLL to power down mode and PLL_STB bit in CLKSTATUS register will be cleared by hardware */
    CLK->PLLCON |= CLK_PLLCON_PD_Msk;

    /* Enable external XTAL 12MHz clock */
    CLK->PWRCON |= CLK_PWRCON_XTL12M_EN_Msk;

    /* Waiting for external XTAL clock ready */
    u32TimeOutCnt = __HIRC;
    while(!(CLK->CLKSTATUS & CLK_CLKSTATUS_XTL12M_STB_Msk))
		if(--u32TimeOutCnt == 0) break;

    /* System optimization when CPU runs at 72MHz */
    FMC->FATCON |= 0x50;

    /* Set core clock as PLL_CLOCK from PLL */
    CLK->PLLCON = PLLCON_SETTING;
    u32TimeOutCnt = __HIRC;
    while(!(CLK->CLKSTATUS & CLK_CLKSTATUS_PLL_STB_Msk))
		if(--u32TimeOutCnt == 0) break;
    CLK->CLKSEL0 = (CLK->CLKSEL0 & (~CLK_CLKSEL0_HCLK_S_Msk)) | CLK_CLKSEL0_HCLK_S_PLL;

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CyclesPerUs automatically. */
    //SystemCoreClockUpdate();
    PllClock        = PLL_CLOCK;            // PLL
    SystemCoreClock = PLL_CLOCK / 1;        // HCLK
    CyclesPerUs     = PLL_CLOCK / 1000000;  // For CLK_SysTickDelay()

    /* Enable UART module clock */
    CLK->APBCLK |= CLK_APBCLK_UART0_EN_Msk;

    /* Select UART module clock source */
    CLK->CLKSEL1 &= ~CLK_CLKSEL1_UART_S_Msk;
    CLK->CLKSEL1 |= CLK_CLKSEL1_UART_S_HXT;

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD);

}

void UART0_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART0 */
    SYS->IPRSTC2 |=  SYS_IPRSTC2_UART0_RST_Msk;
    SYS->IPRSTC2 &= ~SYS_IPRSTC2_UART0_RST_Msk;

    /* Configure UART0 and set UART0 Baudrate */
    UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HXT, 115200);
    UART0->LCR = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    int32_t i32Err;

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();

    printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
    printf("+-------------------------------------------------+\n");
    printf("|    PB.2(Output) and PE.5(Input) Sample Code     |\n");
    printf("+-------------------------------------------------+\n\n");
    printf("  >> Please connect PB.2 and PE.5 first << \n");
    printf("     Press any key to start test by using [Pin Data Input/Output Control] \n\n");
    getchar();

    /* Configure PB.2 as Output mode and PE.5 as Input mode */
    PB->PMD = (PB->PMD & (~GPIO_PMD_PMD2_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD2_Pos);
    PE->PMD = (PE->PMD & (~GPIO_PMD_PMD5_Msk)) | (GPIO_PMD_INPUT << GPIO_PMD_PMD5_Pos);

    i32Err = 0;
    printf("GPIO PB.2(output mode) connect to PE.5(input mode) ......");

    /* Use Pin Data Input/Output Control to pull specified I/O or get I/O pin status */
    /* Pull PB.2 to Low and check PE.5 status */
    PB2 = 0;
    if(PE5 != 0) {
        i32Err = 1;
    }

    /* Pull PB.2 to High and check PE.5 status */
    PB2 = 1;
    if(PE5 != 1) {
        i32Err = 1;
    }

    if(i32Err) {
        printf("  [FAIL].\n");
    } else {
        printf("  [OK].\n");
    }

    /* Configure PB.2 and PE.5 to default Quasi-bidirectional mode */
    PB->PMD = (PB->PMD & (~GPIO_PMD_PMD2_Msk)) | (GPIO_PMD_QUASI << GPIO_PMD_PMD2_Pos);
    PE->PMD = (PE->PMD & (~GPIO_PMD_PMD5_Msk)) | (GPIO_PMD_QUASI << GPIO_PMD_PMD5_Pos);

    while(1);
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
