;/*---------------------------------------------------------------------------------------------------------*/
;/*                                                                                                         */
;/* SPDX-License-Identifier: Apache-2.0                                                                     */
;/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
;/*                                                                                                         */
;/*---------------------------------------------------------------------------------------------------------*/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    MODULE  ?cstartup

    ;; Forward declaration of sections.
    SECTION CSTACK:DATA:NOROOT(3) ;; 8 bytes alignment

    SECTION .intvec:CODE:NOROOT(2);; 4 bytes alignment

    EXTERN  SystemInit	
    EXTERN  ProcessHardFault
    EXTERN  __iar_program_start
    PUBLIC  __vector_table

    DATA
__vector_table
    DCD     sfe(CSTACK)
    DCD     Reset_Handler

    DCD     NMI_Handler
    DCD     HardFault_Handler
    DCD     0
    DCD     0
    DCD     0
    DCD     0
    DCD     0
    DCD     0
    DCD     0
    DCD     SVC_Handler
    DCD     0
    DCD     0
    DCD     PendSV_Handler
    DCD     SysTick_Handler

    ; External Interrupts
    DCD     BOD_IRQHandler              ; Brownout low voltage detected interrupt                 
    DCD     WDT_IRQHandler              ; Watch Dog Timer interrupt                              
    DCD     EINT0_IRQHandler            ; External signal interrupt from PB.14 pin                
    DCD     EINT1_IRQHandler            ; External signal interrupt from PB.15 pin                
    DCD     GPAB_IRQHandler             ; GPIO interrupt from PA[15:0]/PB[13:0]     
    DCD     GPCDEF_IRQHandler          	; GPIO interrupt from PC[15:0]/PD[15:0]/PE[15:0]//PF[3:0]     
    DCD     PWMA_IRQHandler             ; PWM0 or PWM2 interrupt                                 
    DCD     PWMB_IRQHandler             ; PWM1 or PWM3 interrupt                                 
    DCD     TMR0_IRQHandler             ; Timer 0 interrupt                                      
    DCD     TMR1_IRQHandler             ; Timer 1 interrupt                                      
    DCD     TMR2_IRQHandler             ; Timer 2 interrupt                                      
    DCD     TMR3_IRQHandler             ; Timer 3 interrupt                                      
    DCD     UART02_IRQHandler           ; UART0/2 interrupt                                        
    DCD     UART1_IRQHandler            ; UART1 interrupt                                        
    DCD     SPI0_IRQHandler             ; SPI0 interrupt                                         
    DCD     SPI1_IRQHandler             ; SPI1 interrupt                                         
    DCD     Default_Handler             ; Reserved                                   
    DCD     Default_Handler             ; Reserved                                
    DCD     I2C0_IRQHandler             ; I2C0 interrupt                                         
    DCD     I2C1_IRQHandler             ; I2C1 interrupt                                         
    DCD     Default_Handler             ; Reserved                                  
    DCD     Default_Handler             ; Reserved                                     
    DCD     Default_Handler             ; Reserved
    DCD     USBD_IRQHandler             ; USB FS Device interrupt                                
    DCD     Default_Handler             ; Reserved                                     
    DCD     Default_Handler             ; Reserved        
    DCD     PDMA_IRQHandler             ; PDMA interrupt  
    DCD     Default_Handler             ; Reserved
    DCD     PWRWU_IRQHandler            ; Clock controller interrupt for chip wake up from power-
    DCD     ADC_IRQHandler              ; ADC interrupt                                          
    DCD     Default_Handler             ; Reserved
    DCD     RTC_IRQHandler              ; Real time clock interrupt                              

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
    THUMB
    PUBWEAK Reset_Handler   
    SECTION .text:CODE:NOROOT:REORDER(2)       ; 4 bytes alignment
Reset_Handler
        LDR     R0, =0x50000100
        ; Unlock Register                
        LDR     R1, =0x59
        STR     R1, [R0]
        LDR     R1, =0x16
        STR     R1, [R0]
        LDR     R1, =0x88
        STR     R1, [R0]
        
        ; Init POR
        LDR     R2, =0x50000024
        LDR     R1, =0x00005AA5
        STR     R1, [R2]
        
        ; Lock register
        MOVS    R1, #0
        STR     R1, [R0]                

        LDR      R0, =SystemInit
        BLX      R0
        LDR      R0, =__iar_program_start
        BX       R0

    PUBWEAK HardFault_Handler
HardFault_Handler\

        MOV     R0, LR
        MRS     R1, MSP
        MRS     R2, PSP
        LDR     R3, =ProcessHardFault
        BLX     R3
        BX      R0

    PUBWEAK NMI_Handler       
    PUBWEAK SVC_Handler       
    PUBWEAK PendSV_Handler    
    PUBWEAK SysTick_Handler   
    PUBWEAK BOD_IRQHandler   
    PUBWEAK WDT_IRQHandler   
    PUBWEAK EINT0_IRQHandler 
    PUBWEAK EINT1_IRQHandler 
    PUBWEAK GPAB_IRQHandler  
    PUBWEAK GPCDEF_IRQHandler
    PUBWEAK PWMA_IRQHandler 
    PUBWEAK PWMB_IRQHandler 
    PUBWEAK TMR0_IRQHandler 
    PUBWEAK TMR1_IRQHandler 
    PUBWEAK TMR2_IRQHandler 
    PUBWEAK TMR3_IRQHandler 
    PUBWEAK UART02_IRQHandler
    PUBWEAK UART1_IRQHandler
    PUBWEAK SPI0_IRQHandler 
    PUBWEAK SPI1_IRQHandler 
    ;PUBWEAK Default_Handler 
    ;PUBWEAK Default_Handler
    PUBWEAK I2C0_IRQHandler 
    PUBWEAK I2C1_IRQHandler 
    ;PUBWEAK Default_Handler  
    ;PUBWEAK Default_Handler  
    ;PUBWEAK Default_Handler 	
    PUBWEAK USBD_IRQHandler   
    ;PUBWEAK Default_Handler   
    ;PUBWEAK Default_Handler 
    PUBWEAK PDMA_IRQHandler 
    ;PUBWEAK Default_Handler
    PUBWEAK PWRWU_IRQHandler  
    PUBWEAK ADC_IRQHandler    
    PUBWEAK RTC_IRQHandler 
    SECTION .text:CODE:NOROOT:REORDER(2)
;HardFault_Handler 
NMI_Handler       
SVC_Handler       
PendSV_Handler    
SysTick_Handler   
BOD_IRQHandler   
WDT_IRQHandler   
EINT0_IRQHandler 
EINT1_IRQHandler 
GPAB_IRQHandler  
GPCDEF_IRQHandler 
PWMA_IRQHandler  
PWMB_IRQHandler  
TMR0_IRQHandler  
TMR1_IRQHandler  
TMR2_IRQHandler  
TMR3_IRQHandler  
UART02_IRQHandler 
UART1_IRQHandler 
SPI0_IRQHandler  
SPI1_IRQHandler  
;Default_Handler  
;Default_Handler  
I2C0_IRQHandler  
I2C1_IRQHandler  
;Default_Handler
;Default_Handler
;Default_Handler
USBD_IRQHandler   
;Default_Handler
;Default_Handler
PDMA_IRQHandler  
;Default_Handler
PWRWU_IRQHandler
ADC_IRQHandler    
RTC_IRQHandler    
Default_Handler          
    B Default_Handler         

    
;void SH_ICE(void)
    PUBLIC    SH_ICE
SH_ICE    
    CMP   R2,#0
    BEQ   SH_End
    STR   R0,[R2]   ; Save the return value to *pn32Out_R0

;void SH_End(void)
    PUBLIC    SH_End
SH_End
    MOVS   R0,#1    ; Set return value to 1
    BX     lr       ; Return


;int32_t SH_DoCommand(int32_t n32In_R0, int32_t n32In_R1, int32_t *pn32Out_R0)
    PUBLIC    SH_DoCommand
SH_DoCommand
    BKPT   0xAB             ; This instruction will cause ICE trap or system HardFault
    B      SH_ICE
SH_HardFault                ; Captured by HardFault
    MOVS   R0,#0            ; Set return value to 0
    BX     lr               ; Return
    
    
    PUBLIC    __PC
__PC          
        MOV     r0, lr
        BLX     lr
    END

