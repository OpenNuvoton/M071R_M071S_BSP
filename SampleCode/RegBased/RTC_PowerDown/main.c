/**************************************************************************//**
 * @file     main.c
 * @version  V2.00
 * $Revision: 2 $
 * $Date: 15/04/10 9:30a $
 * @brief    M071R_M071S Series RTC Driver Sample Code
 *
 * @note
 * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

#define PLLCON_SETTING      CLK_PLLCON_72MHz_HXT


/*---------------------------------------------------------------------------------------------------------*/
/* Global Interface Variables Declarations                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
extern int IsDebugFifoEmpty(void);
volatile uint8_t g_u8IsRTCAlarmINT = 0;

/*---------------------------------------------------------------------------------------------------------*/
/*  Function for System Entry to Power Down Mode                                                           */
/*---------------------------------------------------------------------------------------------------------*/
void PowerDownFunction(void)
{
    uint32_t u32TimeOutCnt;

    printf("\nSystem enter to power-down mode ...\n");

    /* To check if all the debug messages are finished */
    u32TimeOutCnt = SystemCoreClock; /* 1 second time-out */
    while(IsDebugFifoEmpty() == 0)
        if(--u32TimeOutCnt == 0) break;

    SCB->SCR = 4;

    /* To program PWRCON register, it needs to disable register protection first. */
    CLK->PWRCON = (CLK->PWRCON & ~(CLK_PWRCON_PWR_DOWN_EN_Msk | CLK_PWRCON_PD_WAIT_CPU_Msk)) |
                  CLK_PWRCON_PD_WAIT_CPU_Msk | CLK_PWRCON_PD_WU_INT_EN_Msk;
    CLK->PWRCON |= CLK_PWRCON_PWR_DOWN_EN_Msk;

    __WFI();
}

/**
 * @brief       IRQ Handler for RTC Interrupt
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The RTC_IRQHandler is default IRQ of RTC, declared in startup_M071R_M071S.s.
 */
void RTC_IRQHandler(void)
{
    /* To check if RTC alarm interrupt occurred */
    if(RTC_GET_ALARM_INT_FLAG() == 1) {
        /* Clear RTC alarm interrupt flag */
        RTC_CLEAR_ALARM_INT_FLAG();

        g_u8IsRTCAlarmINT++;
    }
}

void SYS_Init(void)
{
	uint32_t u32TimeOutCnt;

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Enable IRC22M clock */
    CLK->PWRCON |= CLK_PWRCON_IRC22M_EN_Msk;

    /* Waiting for IRC22M clock ready */
    u32TimeOutCnt = __HIRC;
    while(!(CLK->CLKSTATUS & CLK_CLKSTATUS_IRC22M_STB_Msk))
		if(--u32TimeOutCnt == 0) break;

    /* Switch HCLK clock source to HIRC */
    CLK->CLKSEL0 = CLK_CLKSEL0_HCLK_S_HIRC;

    /* Set PLL to Power-down mode and PLL_STB bit in CLKSTATUS register will be cleared by hardware.*/
    CLK->PLLCON |= CLK_PLLCON_PD_Msk;

    /* Enable external 12 MHz XTAL, 32 kHz XTAL */
    CLK->PWRCON |= CLK_PWRCON_XTL12M_EN_Msk | CLK_PWRCON_XTL32K_EN_Msk;

    /* Waiting for clock ready */
    u32TimeOutCnt = __HIRC;
    while(!(CLK->CLKSTATUS & CLK_CLKSTATUS_XTL12M_STB_Msk))
		if(--u32TimeOutCnt == 0) break;
    u32TimeOutCnt = __HIRC;
    while(!(CLK->CLKSTATUS & CLK_CLKSTATUS_XTL32K_STB_Msk))
		if(--u32TimeOutCnt == 0) break;

    /* Enable PLL and Set PLL frequency */
    CLK->PLLCON = PLLCON_SETTING;

    /* Waiting for clock ready */
    u32TimeOutCnt = __HIRC;
    while(!(CLK->CLKSTATUS & CLK_CLKSTATUS_PLL_STB_Msk))
		if(--u32TimeOutCnt == 0) break;

    /* System optimization when CPU runs at 72MHz */
    FMC->FATCON |= 0x50;

    /* Switch HCLK clock source to PLL, STCLK to HCLK/2 */
    CLK->CLKSEL0 = CLK_CLKSEL0_STCLK_S_HCLK_DIV2 | CLK_CLKSEL0_HCLK_S_PLL;

    /* Enable peripheral clock */
    CLK->APBCLK = CLK_APBCLK_UART0_EN_Msk | CLK_APBCLK_RTC_EN_Msk;

    /* Peripheral clock source */
    CLK->CLKSEL1 = CLK_CLKSEL1_UART_S_PLL;

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CyclesPerUs automatically. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD, TXD */
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD);
}

void UART0_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART */
    SYS->IPRSTC2 |=  SYS_IPRSTC2_UART0_RST_Msk;
    SYS->IPRSTC2 &= ~SYS_IPRSTC2_UART0_RST_Msk;

    /* Configure UART0 and set UART0 Baudrate */
    UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(PllClock, 115200);
    UART0->LCR = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}

/*---------------------------------------------------------------------------------------------------------*/
/*  MAIN function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{
    uint32_t u32TimeOutCnt;

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();

    printf("\n\nCPU @ %dHz\n", SystemCoreClock);
    printf("+----------------------------------------------------+\n");
    printf("|    RTC Power-down and Alarm Wake-up Sample Code    |\n");
    printf("+----------------------------------------------------+\n\n");

    /* Initial RTC and stay in normal state */
    if(RTC->INIR != 0x1) {
        RTC->INIR = RTC_INIT_KEY;
        u32TimeOutCnt = SystemCoreClock; /* 1 second time-out */
        while(RTC->INIR != 0x1) {
            if(--u32TimeOutCnt == 0) {
                printf("\n RTC initial fail!!");
                printf("\n Please check h/w setting!!");
                goto lexit;
            }
        }
    }

    /* Setting RTC current date/time */
    RTC_WaitAccessEnable();
    RTC->TSSR = RTC_CLOCK_24;
    RTC->DWR  = RTC_WEDNESDAY;
    RTC->CLR  = 0x00171101;         /* Date: 2017/11/01 */
    RTC->TLR  = 0x00153055;         /* Time: 15:30:55 */

    /* Setting RTC alarm date/time and enable alarm interrupt */
    RTC->CAR  = 0x00171101;         /* Date: 2017/11/01 */
    RTC->TAR  = 0x00153105;         /* Time: 15:31:05 */
    RTC->RIER = RTC_RIER_AIER_Msk;

    /* Enable RTC alarm interrupt and wake-up function will be enabled also */
    NVIC_EnableIRQ(RTC_IRQn);

    printf("# Set RTC current date/time: 2017/11/01 15:30:55.\n");
    printf("# Set RTC alarm date/time:   2017/11/01 15:31:05.\n");
    printf("# Wait system waken-up by RTC alarm interrupt event.\n");

    g_u8IsRTCAlarmINT = 0;

    /* System enter to Power-down */
    /* To program PWRCON register, it needs to disable register protection first. */
    SYS_UnlockReg();
    PowerDownFunction();

    u32TimeOutCnt = SystemCoreClock; /* 1 second time-out */
    while(g_u8IsRTCAlarmINT == 0) {
        if(--u32TimeOutCnt == 0) {
            printf("Wait for RTC alarm interrupt time-out!");
            break;
        }
    }

    /* Read current RTC date/time */
    printf("System has been waken-up and current date/time is:\n");
    printf("    20%02x/%02x/%02x %02x:%02x:%02x\n",
           (RTC->CLR >> RTC_CLR_1YEAR_Pos) & 0xFF, (RTC->CLR >> RTC_CLR_1MON_Pos) & 0xFF, (RTC->CLR >> RTC_CLR_1DAY_Pos) & 0xFF,
           (RTC->TLR >> RTC_TLR_1HR_Pos) & 0xFF, (RTC->TLR >> RTC_TLR_1MIN_Pos) & 0xFF, (RTC->TLR >> RTC_TLR_1SEC_Pos) & 0xFF);

lexit:

    while(1);
}

/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
