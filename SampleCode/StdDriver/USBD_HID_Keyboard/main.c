/******************************************************************************
 * @file     main.c
 * @brief
 *           Show how to implement a USB keyboard device. 
 *           This sample code supports to use GPIO to simulate key input.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"
#include "hid_kb.h"

#define CRYSTAL_LESS        1
#define TRIM_INIT           (GCR_BASE+0x118)

/*--------------------------------------------------------------------------*/
uint8_t volatile g_u8EP2Ready = 0;
static uint32_t s_u32LEDStatus = 0;


/*--------------------------------------------------------------------------*/

void EnableCLKO(uint32_t u32ClkSrc, uint32_t u32ClkDiv)
{
    /* CLKO = clock source / 2^(u32ClkDiv + 1) */
    CLK->FRQDIV = CLK_FRQDIV_DIVIDER_EN_Msk | u32ClkDiv;

    /* Enable CLKO clock source */
    CLK->APBCLK |= CLK_APBCLK_FDIV_EN_Msk;

    /* Select CLKO clock source */
    CLK->CLKSEL2 = (CLK->CLKSEL2 & (~CLK_CLKSEL2_FRQDIV_S_Msk)) | u32ClkSrc;
}



void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC 22.1184 MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

#if (!CRYSTAL_LESS)
    /* Enable external XTAL 12 MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for external XTAL clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    /* Set core clock */
    CLK_SetCoreClock(72000000);
    
    /* Select module clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HIRC, CLK_CLKDIV_UART(1));
    CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL0_USB_S_PLL, CLK_CLKDIV_USB(3));
#else
    /* Enable Internal RC 48MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC48M_EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC48M_STB_Msk);

    /* Set core clock */
    CLK_SetCoreClock(72000000);

    /* Select module clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HIRC, CLK_CLKDIV_UART(1));
    CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL0_USB_S_RC48M, CLK_CLKDIV_USB(1));
#endif

    /* Enable module clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(USBD_MODULE);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set GPB multi-function pins for UART0 RXD and TXD, and Clock Output */
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk | SYS_GPB_MFP_PB8_Msk);
    SYS->ALT_MFP &= ~SYS_ALT_MFP_PB8_Msk;
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD | SYS_GPB_MFP_PB8_CLKO);
    SYS->ALT_MFP |=  SYS_ALT_MFP_PB8_CLKO;

    /* Enable CLKO (PB.8) for monitor HCLK. CLKO = HCLK/8 Hz*/
    EnableCLKO((2 << CLK_CLKSEL2_FRQDIV_S_Pos), 2);
}


void UART0_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset IP */
    SYS->IPRSTC2 |=  SYS_IPRSTC2_UART0_RST_Msk;
    SYS->IPRSTC2 &= ~SYS_IPRSTC2_UART0_RST_Msk;

    /* Configure UART0 and set UART0 Baudrate */
    UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HIRC, 115200);
    UART0->LCR = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}


void HID_UpdateKbData(void)
{
    int32_t i;
    uint8_t *buf;
    uint32_t key = 0xF;
    static uint32_t preKey;

    if(g_u8EP2Ready)
    {
        buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));

        /* If GPB15 = 0, just report it is key 'a' */
        key = (PB->PIN & (1 << 15)) ? 0 : 1;

        if(key == 0)
        {
            for(i = 0; i < 8; i++)
            {
                buf[i] = 0;
            }

            if(key != preKey)
            {
                /* Trigger to note key release */
                USBD_SET_PAYLOAD_LEN(EP2, 8);
            }
        }
        else
        {
            preKey = key;
            buf[2] = 0x04; /* Key A */
            USBD_SET_PAYLOAD_LEN(EP2, 8);
        }
    }

    if(g_au8LEDStatus[0] != s_u32LEDStatus)
    {
        if((g_au8LEDStatus[0] & HID_LED_ALL) != (s_u32LEDStatus & HID_LED_ALL))
        {
            if(g_au8LEDStatus[0] & HID_LED_NumLock)
                printf("NumLock ON, ");

            else
                printf("NumLock OFF, ");

            if(g_au8LEDStatus[0] & HID_LED_CapsLock)
                printf("CapsLock ON, ");

            else
                printf("CapsLock OFF, ");

            if(g_au8LEDStatus[0] & HID_LED_ScrollLock)
                printf("ScrollLock ON, ");

            else
                printf("ScrollLock OFF, ");

            if(g_au8LEDStatus[0] & HID_LED_Compose)
                printf("Compose ON, ");

            else
                printf("Compose OFF, ");

            if(g_au8LEDStatus[0] & HID_LED_Kana)
                printf("Kana ON\n");

            else
                printf("Kana OFF\n");
        }
        s_u32LEDStatus = g_au8LEDStatus[0];
    }
}

void PowerDown()
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Wakeup Enable */
    USBD_ENABLE_INT(USBD_INTEN_WAKEUP_EN_Msk);

    CLK_PowerDown();

    /* Clear PWR_DOWN_EN if it is not clear by itself */
    if(CLK->PWRCON & CLK_PWRCON_PWR_DOWN_EN_Msk)
        CLK->PWRCON ^= CLK_PWRCON_PWR_DOWN_EN_Msk;

    /* Lock protected registers */
    SYS_LockReg();
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
#if CRYSTAL_LESS
    uint32_t u32TrimInit;
#endif

    /* Unlock protected registers */
    SYS_UnlockReg();

    SYS_Init();

    UART0_Init();

    printf("\n");
    printf("+--------------------------------------------------------+\n");
    printf("|          NuMicro USB HID Keyboard Sample Code          |\n");
    printf("+--------------------------------------------------------+\n");
    printf("If PB.15 = 0, just report it is key 'a'.\n");

    USBD_Open(&gsInfo, HID_ClassRequest, NULL);

    /* Endpoint configuration */
    HID_Init();
    USBD_Start();

    NVIC_EnableIRQ(USBD_IRQn);

#if CRYSTAL_LESS
    /* Backup default trim */
    u32TrimInit = M32(TRIM_INIT);
#endif

    /* Clear SOF */
    USBD->INTSTS = USBD_INTSTS_SOF_STS_Msk;

    /* start to IN data */
    g_u8EP2Ready = 1;

    while(1)
    {
#if CRYSTAL_LESS
        /* Start USB trim if it is not enabled. */
        if((SYS->HIRCTCTL & SYS_HIRCTCTL_FREQSEL_Msk) != 1)
        {
            /* Start USB trim only when SOF */
            if(USBD->INTSTS & USBD_INTSTS_SOF_STS_Msk)
            {
                /* Clear SOF */
                USBD->INTSTS = USBD_INTSTS_SOF_STS_Msk;

                /* Re-enable crystal-less */
                SYS->HIRCTCTL = 0x201 | (31 << SYS_HIRCTCTL_BOUNDARY_Pos);
            }
        }

        /* Disable USB Trim when error */
        if(SYS->HIRCTSTS & (SYS_HIRCTSTS_CLKERIF_Msk | SYS_HIRCTSTS_TFAILIF_Msk))
        {
            /* Init TRIM */
            M32(TRIM_INIT) = u32TrimInit;

            if((u32TrimInit < 0x1E6) || (u32TrimInit > 0x253))
                /* Re-enable crystal-less */
                SYS->HIRCTCTL = 0x201 | (1 << SYS_HIRCTCTL_BOUNDARY_Pos);

            /* Clear error flags */
            SYS->HIRCTSTS = SYS_HIRCTSTS_CLKERIF_Msk | SYS_HIRCTSTS_TFAILIF_Msk;

            /* Clear SOF */
            USBD->INTSTS = USBD_INTSTS_SOF_STS_Msk;
        }
#endif

        /* Enter power down when USB suspend */
        if(g_u8Suspend)
            PowerDown();

        HID_UpdateKbData();
    }
}



/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/

