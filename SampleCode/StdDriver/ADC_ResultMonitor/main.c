/****************************************************************************
 * @file     main.c
 * @version  V2.0
 * $Revision: 2 $
 * $Date: 15/04/10 4:35p $
 * @brief    M071R_M071S Series ADC Interface Controller Driver Sample Code
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"


#define PLL_CLOCK       72000000



/*---------------------------------------------------------------------------------------------------------*/
/* Define Function Prototypes                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void SYS_Init(void);
void UART0_Init(void);
void AdcResultMonitorTest(void);


/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_u32AdcCmp0IntFlag;
volatile uint32_t g_u32AdcCmp1IntFlag;


void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC 22.1184MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external XTAL 12MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for external XTAL clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(PLL_CLOCK);

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Enable ADC module clock */
    CLK_EnableModuleClock(ADC_MODULE);

    /* Select UART module clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HIRC, CLK_CLKDIV_UART(1));

    /* ADC clock source is 22.1184MHz, set divider to 7, ADC clock is 22.1184/7 MHz */
    CLK_SetModuleClock(ADC_MODULE, CLK_CLKSEL1_ADC_S_HIRC, CLK_CLKDIV_ADC(7));

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);
    SYS->GPB_MFP |= SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD;

    /* Disable the GPA0 - GPA3 digital input path to avoid the leakage current. */
    GPIO_DISABLE_DIGITAL_PATH(PA, 0xF);

    /* Configure the GPA0 - GPA3 ADC analog input pins */
    SYS->GPA_MFP &= ~(SYS_GPA_MFP_PA0_Msk | SYS_GPA_MFP_PA1_Msk | SYS_GPA_MFP_PA2_Msk | SYS_GPA_MFP_PA3_Msk) ;
    SYS->GPA_MFP |= SYS_GPA_MFP_PA0_ADC0 | SYS_GPA_MFP_PA1_ADC1 | SYS_GPA_MFP_PA2_ADC2 | SYS_GPA_MFP_PA3_ADC3 ;
    SYS->ALT_MFP &= ~(SYS_ALT_MFP_PA1_Msk | SYS_ALT_MFP_PA2_Msk | SYS_ALT_MFP_PA3_Msk);
    SYS->ALT_MFP |= SYS_ALT_MFP_PA1_ADC1 | SYS_ALT_MFP_PA2_ADC2 | SYS_ALT_MFP_PA3_ADC3;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Init UART                                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void UART0_Init()
{
    /* Reset IP */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: AdcResultMonitorTest                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*   None.                                                                                                 */
/*                                                                                                         */
/* Returns:                                                                                                */
/*   None.                                                                                                 */
/*                                                                                                         */
/* Description:                                                                                            */
/*   ADC result monitor function test.                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
void AdcResultMonitorTest()
{
    uint32_t u32TimeOutCnt;

    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|           ADC compare function (result monitor) sample code          |\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("\nIn this test, software will compare the conversion result of channel 2.\n");

    /* Power on ADC module */
    ADC_POWER_ON(ADC);

    /* Set the ADC operation mode as continuous scan, input mode as single-end and enable the analog input channel 2 */
    ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_CONTINUOUS, 0x1 << 2);

    /* Enable ADC comparator 0. Compare condition: conversion result < 0x800; match Count=5. */
    printf("   Set the compare condition of comparator 0: channel 2 is less than 0x800; match count is 5.\n");
    ADC_ENABLE_CMP0(ADC, 2, ADC_ADCMPR_CMPCOND_LESS_THAN, 0x800, 5);

    /* Enable ADC comparator 1. Compare condition: conversion result >= 0x800; match Count=5. */
    printf("   Set the compare condition of comparator 1: channel 2 is greater than or equal to 0x800; match count is 5.\n");
    ADC_ENABLE_CMP1(ADC, 2, ADC_ADCMPR_CMPCOND_GREATER_OR_EQUAL, 0x800, 5);

    /* Clear the ADC comparator 0 interrupt flag for safe */
    ADC_CLR_INT_FLAG(ADC, ADC_CMP0_INT);
    /* Enable ADC comparator 0 interrupt */
    ADC_EnableInt(ADC, ADC_CMP0_INT);

    /* Clear the ADC comparator 1 interrupt flag for safe */
    ADC_CLR_INT_FLAG(ADC, ADC_CMP1_INT);
    /* Enable ADC comparator 1 interrupt */
    ADC_EnableInt(ADC, ADC_CMP1_INT);

    NVIC_EnableIRQ(ADC_IRQn);

    g_u32AdcCmp0IntFlag = 0;
    g_u32AdcCmp1IntFlag = 0;

    /* Clear the ADC interrupt flag */
    ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

    /* Start A/D conversion */
    ADC_START_CONV(ADC);

    /* Wait ADC compare interrupt */
    u32TimeOutCnt = SystemCoreClock; /* 1 second time-out */
    while((g_u32AdcCmp0IntFlag == 0) && (g_u32AdcCmp1IntFlag == 0)) {
        if(--u32TimeOutCnt == 0) {
            printf("Wait for ADC compare interrupt time-out!\n");
            return;
        }
    }

    /* Stop A/D conversion */
    ADC_STOP_CONV(ADC);
    /* Disable ADC comparator interrupt */
    ADC_DisableInt(ADC, ADC_CMP0_INT);
    ADC_DisableInt(ADC, ADC_CMP1_INT);
    /* Disable compare function */
    ADC_DISABLE_CMP0(ADC);
    ADC_DISABLE_CMP1(ADC);

    if(g_u32AdcCmp0IntFlag == 1) {
        printf("Comparator 0 interrupt occurs.\nThe conversion result of channel 2 is less than 0x800\n");
    } else {
        printf("Comparator 1 interrupt occurs.\nThe conversion result of channel 2 is greater than or equal to 0x800\n");
    }
}


/*---------------------------------------------------------------------------------------------------------*/
/* ADC interrupt handler                                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
void ADC_IRQHandler(void)
{
    if(ADC_GET_INT_FLAG(ADC, ADC_CMP0_INT) != 0) {
        g_u32AdcCmp0IntFlag = 1;
        ADC_CLR_INT_FLAG(ADC, ADC_CMP0_INT);     /* clear the A/D compare flag 0 */
    }

    if(ADC_GET_INT_FLAG(ADC, ADC_CMP1_INT) != 0) {
        g_u32AdcCmp1IntFlag = 1;
        ADC_CLR_INT_FLAG(ADC, ADC_CMP1_INT);     /* clear the A/D compare flag 1 */
    }
}
/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/

int32_t main(void)
{

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();

    /*---------------------------------------------------------------------------------------------------------*/
    /* SAMPLE CODE                                                                                             */
    /*---------------------------------------------------------------------------------------------------------*/

    printf("\nSystem clock rate: %d Hz", SystemCoreClock);

    /* Result monitor test */
    AdcResultMonitorTest();

    /* Disable ADC module */
    ADC_Close(ADC);

    /* Disable ADC IP clock */
    CLK_DisableModuleClock(ADC_MODULE);

    /* Disable External Interrupt */
    NVIC_DisableIRQ(ADC_IRQn);

    printf("\nExit ADC sample code\n");

    while(1);

}

