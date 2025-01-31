/***************************************************************************//**
 * @file     targetdev.h
 * @brief    ISP support function header file
 * @version  0x31
 *
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "NuMicro.h"
#include "uart_transfer.h"
#include "isp_user.h"

#define DetectPin   				PA10

extern __attribute__((aligned(4))) uint8_t response_buff[64];
extern __attribute__((aligned(4))) uint8_t usb_rcvbuf[64];
extern uint8_t bUsbDataReady;


/* rename for uart_transfer.c */
#define UART_N						UART0
#define UART_N_IRQHandler		    UART02_IRQHandler
#define UART_N_IRQn					UART02_IRQn
