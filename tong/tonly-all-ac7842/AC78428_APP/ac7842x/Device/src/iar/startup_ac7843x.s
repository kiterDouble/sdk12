;/* Copyright Statement:
; *
; * This software/firmware and related documentation ("AutoChips Software") are
; * protected under relevant copyright laws. The information contained herein is
; * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
; * the prior written permission of AutoChips inc. and/or its licensors, any
; * reproduction, modification, use or disclosure of AutoChips Software, and
; * information contained herein, in whole or in part, shall be strictly
; * prohibited.
; *
; * AutoChips Inc. (C) 2024. All rights reserved.
; *
; * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
; * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
; * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
; * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
; * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
; * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
; * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
; * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
; * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
; * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
; * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER''S SOLE RESPONSIBILITY TO
; * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
; * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
; * RELEASES MADE TO RECEIVER''S SPECIFICATION OR TO CONFORM TO A PARTICULAR
; * STANDARD OR OPEN FORUM. RECEIVER''S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS''S
; * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
; * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS''S OPTION, TO REVISE OR REPLACE THE
; * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
; * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
; */
;* File Name          : startup_ac7843x.s
;*
;* @file startup_ac7843x.s
;*
;* @author AutoChips
;*
;* @version 0.0.1
;*
;* @date April.31.2023
;* Description        : AC7843x Devices vector table for EWARM
;*                      toolchain.
;*                      This module performs:
;*                      - Set the initial SP
;*                      - Configure the clock system
;*                      - Set the initial PC == __iar_program_start,
;*                      - Set the vector table entries with the exceptions ISR
;*                        address.
;*                      After Reset the Cortex-M4 processor is in Thread mode,
;*                      priority is Privileged, and the Stack is set to Main.
;********************************************************************************
;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        EXTERN  SystemInitRam
        EXTERN  SystemInit
        PUBLIC  __vector_table
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

        DATA
__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler              ; Reset Handler

        DCD     NMI_Handler                ; NMI Handler
        DCD     HardFault_Handler          ; Hard Fault Handler
        DCD     MemManage_Handler          ; MPU Fault Handler
        DCD     BusFault_Handler           ; Bus Fault Handler
        DCD     UsageFault_Handler         ; Usage Fault Handler
        DCD     0                          ; Reserved
        DCD     0                          ; Reserved
        DCD     0                          ; Reserved
        DCD     0                          ; Reserved
        DCD     SVC_Handler                ; SVCall Handler
        DCD     DebugMon_Handler           ; Debug Monitor Handler
        DCD     0                          ; Reserved
        DCD     PendSV_Handler             ; PendSV Handler
        DCD     SysTick_Handler            ; SysTick Handler

        ;External Interrupts
        DCD     DMA0_Channel0_IRQHandler            ;DMA channel 0 Interrupt
        DCD     DMA0_Channel1_IRQHandler            ;DMA channel 1 Interrupt
        DCD     DMA0_Channel2_IRQHandler            ;DMA channel 2 Interrupt
        DCD     DMA0_Channel3_IRQHandler            ;DMA channel 3 Interrupt
        DCD     DMA0_Channel4_IRQHandler            ;DMA channel 4 Interrupt
        DCD     DMA0_Channel5_IRQHandler            ;DMA channel 5 Interrupt
        DCD     DMA0_Channel6_IRQHandler            ;DMA channel 6 Interrupt
        DCD     DMA0_Channel7_IRQHandler            ;DMA channel 7 Interrupt
        DCD     DMA0_Channel8_IRQHandler            ;DMA channel 8 Interrupt
        DCD     DMA0_Channel9_IRQHandler            ;DMA channel 9 Interrupt
        DCD     DMA0_Channel10_IRQHandler           ;DMA channel 10 Interrupt
        DCD     DMA0_Channel11_IRQHandler           ;DMA channel 11 Interrupt
        DCD     DMA0_Channel12_IRQHandler           ;DMA channel 12 Interrupt
        DCD     DMA0_Channel13_IRQHandler           ;DMA channel 13 Interrupt
        DCD     DMA0_Channel14_IRQHandler           ;DMA channel 14 Interrupt
        DCD     DMA0_Channel15_IRQHandler           ;DMA channel 15 Interrupt
        DCD     0                                   ;Reserved
        DCD     0                                   ;Reserved
        DCD     0                                   ;Reserved
        DCD     0                                   ;Reserved
        DCD     0                                   ;Reserved
        DCD     0                                   ;Reserved
        DCD     0                                   ;Reserved
        DCD     0                                   ;Reserved
        DCD     PORTA_IRQHandler                    ;PortA Interrupt
        DCD     PORTB_IRQHandler                    ;PortB Interrupt
        DCD     PORTC_IRQHandler                    ;PortC Interrupt
        DCD     PORTD_IRQHandler                    ;PortD Interrupt
        DCD     PORTE_IRQHandler                    ;PortE Interrupt
        DCD     UART0_IRQHandler                    ;UART0 Interrupt
        DCD     UART1_IRQHandler                    ;UART1 Interrupt
        DCD     UART2_IRQHandler                    ;UART2 Interrupt
        DCD     UART3_IRQHandler                    ;UART3 Interrupt
        DCD     UART4_IRQHandler                    ;UART4 Interrupt
        DCD     UART5_IRQHandler                    ;UART5 Interrupt
        DCD     UART6_IRQHandler                    ;UART6 Interrupt
        DCD     UART7_IRQHandler                    ;UART7 Interrupt
        DCD     SPI0_IRQHandler                     ;SPI0 Interrupt
        DCD     SPI1_IRQHandler                     ;SPI1 Interrupt
        DCD     SPI2_IRQHandler                     ;SPI2 Interrupt
        DCD     SPI3_IRQHandler                     ;SPI3 Interrupt
        DCD     SPI4_IRQHandler                     ;SPI4 Interrupt
        DCD     I2C0_IRQHandler                     ;I2C0 Interrupt
        DCD     I2C1_IRQHandler                     ;I2C1 Interrupt
        DCD     I2C2_IRQHandler                     ;I2C2 Interrupt
        DCD     EIO_IRQHandler                      ;EIO Interrupt
        DCD     CAN0_IRQHandler                     ;CAN0 Interrupt
        DCD     CAN0_Wakeup_IRQHandler              ;CAN0 Wakeup Interrupt
        DCD     CAN0_DMU_IRQHandler                 ;CAN0 DMU Interrupt
        DCD     CAN1_IRQHandler                     ;CAN1 Interrupt
        DCD     CAN1_Wakeup_IRQHandler              ;CAN1 Wakeup Interrupt
        DCD     CAN1_DMU_IRQHandler                 ;CAN1 DMU Interrupt
        DCD     CAN2_IRQHandler                     ;CAN2 Interrupt
        DCD     CAN2_Wakeup_IRQHandler              ;CAN2 Wakeup Interrupt
        DCD     CAN2_DMU_IRQHandler                 ;CAN2 DMU Interrupt
        DCD     CAN3_IRQHandler                     ;CAN3 Interrupt
        DCD     CAN3_Wakeup_IRQHandler              ;CAN3 Wakeup Interrupt
        DCD     CAN3_DMU_IRQHandler                 ;CAN3 DMU Interrupt
        DCD     CAN4_IRQHandler                     ;CAN4 Interrupt
        DCD     CAN4_Wakeup_IRQHandler              ;CAN4 Wakeup Interrupt
        DCD     CAN4_DMU_IRQHandler                 ;CAN4 DMU Interrupt
        DCD     CAN5_IRQHandler                     ;CAN5 Interrupt
        DCD     CAN5_Wakeup_IRQHandler              ;CAN5 Wakeup Interrupt
        DCD     CAN5_DMU_IRQHandler                 ;CAN5 DMU Interrupt
        DCD     PDT0_IRQHandler                     ;PDT0 Interrupt
        DCD     PDT1_IRQHandler                     ;PDT1 Interrupt
        DCD     ADC0_IRQHandler                     ;ADC0 Interrupt
        DCD     ADC1_IRQHandler                     ;ADC1 Interrupt
        DCD     ACMP0_IRQHandler                    ;ACMP0 Interrupt
        DCD     ACMP1_IRQHandler                    ;ACMP1 Interrupt
        DCD     WDG_IRQHandler                      ;WDG Interrupt
        DCD     EWDG_IRQHandler                     ;EWDG Interrupt
        DCD     MCM_IRQHandler                      ;MCM Interrupt
        DCD     LVD_IRQHandler                      ;LVD Interrupt
        DCD     SPM_IRQHandler                      ;SPM Interrupt
        DCD     RCM_IRQHandler                      ;RCM Interrupt
        DCD     PWM0_Overflow_IRQHandler            ;PWM0 Overflow Interrupt
        DCD     PWM0_Channel_IRQHandler             ;PWM0 Channel Interrupt
        DCD     PWM0_Fault_IRQHandler               ;PWM0 Fault Interrupt
        DCD     PWM0_Detect_IRQHandler              ;PWM0 Detect Interrupt
        DCD     PWM1_Overflow_IRQHandler            ;PWM1 Overflow Interrupt
        DCD     PWM1_Channel_IRQHandler             ;PWM1 Channel Interrupt
        DCD     PWM1_Fault_IRQHandler               ;PWM1 Fault Interrupt
        DCD     PWM1_Detect_IRQHandler              ;PWM1 Detect Interrupt
        DCD     PWM2_Overflow_IRQHandler            ;PWM2 Overflow Interrupt
        DCD     PWM2_Channel_IRQHandler             ;PWM2 Channel Interrupt
        DCD     PWM2_Fault_IRQHandler               ;PWM2 Fault Interrupt
        DCD     PWM2_Detect_IRQHandler              ;PWM2 Detect Interrupt
        DCD     PWM3_Overflow_IRQHandler            ;PWM3 Overflow Interrupt
        DCD     PWM3_Channel_IRQHandler             ;PWM3 Channel Interrupt
        DCD     PWM3_Fault_IRQHandler               ;PWM3 Fault Interrupt
        DCD     PWM3_Detect_IRQHandler              ;PWM3 Detect Interrupt
        DCD     PWM4_Overflow_IRQHandler            ;PWM4 Overflow Interrupt
        DCD     PWM4_Channel_IRQHandler             ;PWM4 Channel Interrupt
        DCD     PWM4_Fault_IRQHandler               ;PWM4 Fault Interrupt
        DCD     PWM4_Detect_IRQHandler              ;PWM4 Detect Interrupt
        DCD     PWM5_Overflow_IRQHandler            ;PWM5 Overflow Interrupt
        DCD     PWM5_Channel_IRQHandler             ;PWM5 Channel Interrupt
        DCD     PWM5_Fault_IRQHandler               ;PWM5 Fault Interrupt
        DCD     PWM5_Detect_IRQHandler              ;PWM5 Detect Interrupt
        DCD     PWM6_Overflow_IRQHandler            ;PWM6 Overflow Interrupt
        DCD     PWM6_Channel_IRQHandler             ;PWM6 Channel Interrupt
        DCD     PWM6_Fault_IRQHandler               ;PWM6 Fault Interrupt
        DCD     PWM6_Detect_IRQHandler              ;PWM6 Detect Interrupt
        DCD     PWM7_Overflow_IRQHandler            ;PWM7 Overflow Interrupt
        DCD     PWM7_Channel_IRQHandler             ;PWM7 Channel Interrupt
        DCD     PWM7_Fault_IRQHandler               ;PWM7 Fault Interrupt
        DCD     PWM7_Detect_IRQHandler              ;PWM7 Detect Interrupt
        DCD     RTC_IRQHandler                      ;RTC Interrupt
        DCD     PCT_IRQHandler                      ;PCT Interrupt
        DCD     TIMER_Channel0_IRQHandler           ;TIMER Channel 0 Interrupt
        DCD     TIMER_Channel1_IRQHandler           ;TIMER Channel 1 Interrupt
        DCD     TIMER_Channel2_IRQHandler           ;TIMER Channel 2 Interrupt
        DCD     TIMER_Channel3_IRQHandler           ;TIMER Channel 3 Interrupt
        DCD     HSM_IRQHandler                      ;HSM Interrupt
        DCD     FLASH_ECC_IRQHandler                ;Flash ECC 2BIT Interrupt
        DCD     FLASH_IRQHandler                    ;Flash Interrupt
        DCD     FLASH_Collision_IRQHandler          ;Flash Collision Interrupt
        DCD     ECC_1BIT_IRQHandler                 ;ECC 1BIT Interrupt
        DCD     ECC_2BIT_IRQHandler                 ;ECC 2BIT Interrupt
        DCD     SENT_Channel0_IRQHandler            ;SENT Channel 0 Interrupt
        DCD     SENT_Channel1_IRQHandler            ;SENT Channel 1 Interrupt
        DCD     SENT_Channel2_IRQHandler            ;SENT Channel 2 Interrupt
        DCD     SENT_Channel3_IRQHandler            ;SENT Channel 3 Interrupt
        DCD     SMU_IRQHandler                      ;SMU Interrupt
        DCD     STB_WU_IRQHandler                   ;Standby Wakeup Interrupt
__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB
        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER:NOROOT(2)
Reset_Handler
        MRS     R10, PRIMASK    ; Save PRIMASK
        CPSID   I               ; Mask interrupts

        ; Init ECC SRAM
        LDR     R0, =sfb(CSTACK)
        LDR     R1, =sfe(CSTACK)
        LDR     R3, =SystemInitRam
        BLX     R3

        ; Call the CMSIS system init routine
        LDR     R0, =SystemInit
        BLX     R0

        MSR     PRIMASK, R10    ; Restore PRIMASK
        LDR     R0, =__iar_program_start
        BX      R0

; Dummy Exception Handlers (infinite loops which can be modified)

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
HardFault_Handler
        B HardFault_Handler

        PUBWEAK MemManage_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
UsageFault_Handler
        B UsageFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SVC_Handler
        B SVC_Handler

        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
DebugMon_Handler
        B DebugMon_Handler

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
PendSV_Handler
        B PendSV_Handler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SysTick_Handler
        B SysTick_Handler

        PUBWEAK DMA0_Channel0_IRQHandler
        PUBWEAK DMA0_Channel1_IRQHandler
        PUBWEAK DMA0_Channel2_IRQHandler
        PUBWEAK DMA0_Channel3_IRQHandler
        PUBWEAK DMA0_Channel4_IRQHandler
        PUBWEAK DMA0_Channel5_IRQHandler
        PUBWEAK DMA0_Channel6_IRQHandler
        PUBWEAK DMA0_Channel7_IRQHandler
        PUBWEAK DMA0_Channel8_IRQHandler
        PUBWEAK DMA0_Channel9_IRQHandler
        PUBWEAK DMA0_Channel10_IRQHandler
        PUBWEAK DMA0_Channel11_IRQHandler
        PUBWEAK DMA0_Channel12_IRQHandler
        PUBWEAK DMA0_Channel13_IRQHandler
        PUBWEAK DMA0_Channel14_IRQHandler
        PUBWEAK DMA0_Channel15_IRQHandler
        PUBWEAK PORTA_IRQHandler
        PUBWEAK PORTB_IRQHandler
        PUBWEAK PORTC_IRQHandler
        PUBWEAK PORTD_IRQHandler
        PUBWEAK PORTE_IRQHandler
        PUBWEAK UART0_IRQHandler
        PUBWEAK UART1_IRQHandler
        PUBWEAK UART2_IRQHandler
        PUBWEAK UART3_IRQHandler
        PUBWEAK UART4_IRQHandler
        PUBWEAK UART5_IRQHandler
        PUBWEAK UART6_IRQHandler
        PUBWEAK UART7_IRQHandler
        PUBWEAK SPI0_IRQHandler
        PUBWEAK SPI1_IRQHandler
        PUBWEAK SPI2_IRQHandler
        PUBWEAK SPI3_IRQHandler
        PUBWEAK SPI4_IRQHandler
        PUBWEAK I2C0_IRQHandler
        PUBWEAK I2C1_IRQHandler
        PUBWEAK I2C2_IRQHandler
        PUBWEAK EIO_IRQHandler
        PUBWEAK CAN0_IRQHandler
        PUBWEAK CAN0_Wakeup_IRQHandler
        PUBWEAK CAN0_DMU_IRQHandler
        PUBWEAK CAN1_IRQHandler
        PUBWEAK CAN1_Wakeup_IRQHandler
        PUBWEAK CAN1_DMU_IRQHandler
        PUBWEAK CAN2_IRQHandler
        PUBWEAK CAN2_Wakeup_IRQHandler
        PUBWEAK CAN2_DMU_IRQHandler
        PUBWEAK CAN3_IRQHandler
        PUBWEAK CAN3_Wakeup_IRQHandler
        PUBWEAK CAN3_DMU_IRQHandler
        PUBWEAK CAN4_IRQHandler
        PUBWEAK CAN4_Wakeup_IRQHandler
        PUBWEAK CAN4_DMU_IRQHandler
        PUBWEAK CAN5_IRQHandler
        PUBWEAK CAN5_Wakeup_IRQHandler
        PUBWEAK CAN5_DMU_IRQHandler
        PUBWEAK PDT0_IRQHandler
        PUBWEAK PDT1_IRQHandler
        PUBWEAK ADC0_IRQHandler
        PUBWEAK ADC1_IRQHandler
        PUBWEAK ACMP0_IRQHandler
        PUBWEAK ACMP1_IRQHandler
        PUBWEAK WDG_IRQHandler
        PUBWEAK EWDG_IRQHandler
        PUBWEAK MCM_IRQHandler
        PUBWEAK LVD_IRQHandler
        PUBWEAK SPM_IRQHandler
        PUBWEAK RCM_IRQHandler
        PUBWEAK PWM0_Overflow_IRQHandler
        PUBWEAK PWM0_Channel_IRQHandler
        PUBWEAK PWM0_Fault_IRQHandler
        PUBWEAK PWM0_Detect_IRQHandler
        PUBWEAK PWM1_Overflow_IRQHandler
        PUBWEAK PWM1_Channel_IRQHandler
        PUBWEAK PWM1_Fault_IRQHandler
        PUBWEAK PWM1_Detect_IRQHandler
        PUBWEAK PWM2_Overflow_IRQHandler
        PUBWEAK PWM2_Channel_IRQHandler
        PUBWEAK PWM2_Fault_IRQHandler
        PUBWEAK PWM2_Detect_IRQHandler
        PUBWEAK PWM3_Overflow_IRQHandler
        PUBWEAK PWM3_Channel_IRQHandler
        PUBWEAK PWM3_Fault_IRQHandler
        PUBWEAK PWM3_Detect_IRQHandler
        PUBWEAK PWM4_Overflow_IRQHandler
        PUBWEAK PWM4_Channel_IRQHandler
        PUBWEAK PWM4_Fault_IRQHandler
        PUBWEAK PWM4_Detect_IRQHandler
        PUBWEAK PWM5_Overflow_IRQHandler
        PUBWEAK PWM5_Channel_IRQHandler
        PUBWEAK PWM5_Fault_IRQHandler
        PUBWEAK PWM5_Detect_IRQHandler
        PUBWEAK PWM6_Overflow_IRQHandler
        PUBWEAK PWM6_Channel_IRQHandler
        PUBWEAK PWM6_Fault_IRQHandler
        PUBWEAK PWM6_Detect_IRQHandler
        PUBWEAK PWM7_Overflow_IRQHandler
        PUBWEAK PWM7_Channel_IRQHandler
        PUBWEAK PWM7_Fault_IRQHandler
        PUBWEAK PWM7_Detect_IRQHandler
        PUBWEAK RTC_IRQHandler
        PUBWEAK PCT_IRQHandler
        PUBWEAK TIMER_Channel0_IRQHandler
        PUBWEAK TIMER_Channel1_IRQHandler
        PUBWEAK TIMER_Channel2_IRQHandler
        PUBWEAK TIMER_Channel3_IRQHandler
        PUBWEAK HSM_IRQHandler
        PUBWEAK FLASH_ECC_IRQHandler
        PUBWEAK FLASH_IRQHandler
        PUBWEAK FLASH_Collision_IRQHandler
        PUBWEAK ECC_1BIT_IRQHandler
        PUBWEAK ECC_2BIT_IRQHandler
        PUBWEAK SENT_Channel0_IRQHandler
        PUBWEAK SENT_Channel1_IRQHandler
        PUBWEAK SENT_Channel2_IRQHandler
        PUBWEAK SENT_Channel3_IRQHandler
        PUBWEAK SMU_IRQHandler
        PUBWEAK STB_WU_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DMA0_Channel0_IRQHandler
DMA0_Channel1_IRQHandler
DMA0_Channel2_IRQHandler
DMA0_Channel3_IRQHandler
DMA0_Channel4_IRQHandler
DMA0_Channel5_IRQHandler
DMA0_Channel6_IRQHandler
DMA0_Channel7_IRQHandler
DMA0_Channel8_IRQHandler
DMA0_Channel9_IRQHandler
DMA0_Channel10_IRQHandler
DMA0_Channel11_IRQHandler
DMA0_Channel12_IRQHandler
DMA0_Channel13_IRQHandler
DMA0_Channel14_IRQHandler
DMA0_Channel15_IRQHandler
PORTA_IRQHandler
PORTB_IRQHandler
PORTC_IRQHandler
PORTD_IRQHandler
PORTE_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
UART2_IRQHandler
UART3_IRQHandler
UART4_IRQHandler
UART5_IRQHandler
UART6_IRQHandler
UART7_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
SPI3_IRQHandler
SPI4_IRQHandler
I2C0_IRQHandler
I2C1_IRQHandler
I2C2_IRQHandler
EIO_IRQHandler
CAN0_IRQHandler
CAN0_Wakeup_IRQHandler
CAN0_DMU_IRQHandler
CAN1_IRQHandler
CAN1_Wakeup_IRQHandler
CAN1_DMU_IRQHandler
CAN2_IRQHandler
CAN2_Wakeup_IRQHandler
CAN2_DMU_IRQHandler
CAN3_IRQHandler
CAN3_Wakeup_IRQHandler
CAN3_DMU_IRQHandler
CAN4_IRQHandler
CAN4_Wakeup_IRQHandler
CAN4_DMU_IRQHandler
CAN5_IRQHandler
CAN5_Wakeup_IRQHandler
CAN5_DMU_IRQHandler
PDT0_IRQHandler
PDT1_IRQHandler
ADC0_IRQHandler
ADC1_IRQHandler
ACMP0_IRQHandler
ACMP1_IRQHandler
WDG_IRQHandler
EWDG_IRQHandler
MCM_IRQHandler
LVD_IRQHandler
SPM_IRQHandler
RCM_IRQHandler
PWM0_Overflow_IRQHandler
PWM0_Channel_IRQHandler
PWM0_Fault_IRQHandler
PWM0_Detect_IRQHandler
PWM1_Overflow_IRQHandler
PWM1_Channel_IRQHandler
PWM1_Fault_IRQHandler
PWM1_Detect_IRQHandler
PWM2_Overflow_IRQHandler
PWM2_Channel_IRQHandler
PWM2_Fault_IRQHandler
PWM2_Detect_IRQHandler
PWM3_Overflow_IRQHandler
PWM3_Channel_IRQHandler
PWM3_Fault_IRQHandler
PWM3_Detect_IRQHandler
PWM4_Overflow_IRQHandler
PWM4_Channel_IRQHandler
PWM4_Fault_IRQHandler
PWM4_Detect_IRQHandler
PWM5_Overflow_IRQHandler
PWM5_Channel_IRQHandler
PWM5_Fault_IRQHandler
PWM5_Detect_IRQHandler
PWM6_Overflow_IRQHandler
PWM6_Channel_IRQHandler
PWM6_Fault_IRQHandler
PWM6_Detect_IRQHandler
PWM7_Overflow_IRQHandler
PWM7_Channel_IRQHandler
PWM7_Fault_IRQHandler
PWM7_Detect_IRQHandler
RTC_IRQHandler
PCT_IRQHandler
TIMER_Channel0_IRQHandler
TIMER_Channel1_IRQHandler
TIMER_Channel2_IRQHandler
TIMER_Channel3_IRQHandler
HSM_IRQHandler
FLASH_ECC_IRQHandler
FLASH_IRQHandler
FLASH_Collision_IRQHandler
ECC_1BIT_IRQHandler
ECC_2BIT_IRQHandler
SENT_Channel0_IRQHandler
SENT_Channel1_IRQHandler
SENT_Channel2_IRQHandler
SENT_Channel3_IRQHandler
SMU_IRQHandler
STB_WU_IRQHandler

    B       .

    END
;/****************END OF FILE******************************************/

