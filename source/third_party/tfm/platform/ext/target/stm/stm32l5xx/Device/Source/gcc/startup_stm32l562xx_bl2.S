;/*
; * Copyright (c) 2009-2018 ARM Limited
; *
; * Licensed under the Apache License, Version 2.0 (the "License");
; * you may not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; *     http://www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an "AS IS" BASIS,
; * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; *
; *
; * This file is derivative of CMSIS V5.00 startup_ARMCM33.S
; */

    .syntax    unified
    .arch    armv8-m.base

    .section .vectors
    .align 2
    .globl    __Vectors
__Vectors:
    .long    Image$$ARM_LIB_STACK$$ZI$$Limit   /* Top of Stack */

    /* Core interrupts */
    .long    Reset_Handler         /* Reset Handler */
    .long    NMI_Handler           /* NMI Handler */
    .long    HardFault_Handler     /* Hard Fault Handler */
    .long    MemManage_Handler     /* MPU Fault Handler */
    .long    BusFault_Handler      /* Bus Fault Handler */
    .long    UsageFault_Handler    /* Usage Fault Handler */
    .long    SecureFault_Handler   /* Secure Fault Handler */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    SVC_Handler           /* SVCall Handler */
    .long    DebugMon_Handler      /* Debug Monitor Handler */
    .long    0                     /* Reserved */
    .long    PendSV_Handler        /* PendSV Handler */
    .long    SysTick_Handler       /* SysTick Handler */

    /* Core interrupts */
    .long     WWDG_IRQHandler       /* Window WatchDog */
    .long     PVD_PVM_IRQHandler        /* PVD/PVM1/PVM2/PVM3/PVM4 through EXTI Line detection */
    .long     RTC_IRQHandler            /* RTC non-secure interrupts through the EXTI line */
    .long     RTC_IRQHandler_S          /* RRTC secure interrupts through the EXTI line */
    .long     TAMP_IRQHandler           /* RTamper non-secure interrupts through the EXTI line */
    .long     TAMP_IRQHandler_S         /* RTamper secure interrupts through the EXTI line */
    .long     FLASH_IRQHandler          /* RFLASH non-secure interrupts */
    .long     FLASH_IRQHandler_S        /* RFLASH secure global interrupts */
    .long     SERR_IRQHandler           /* RSecure Error interrupts */
    .long     RCC_IRQHandler            /* RRCC non-secure global interrupts */
    .long     RCC_IRQHandler_S          /* RRCC secure global interrupts */
    .long     EXTI0_IRQHandler          /* REXTI Line0 */
    .long     EXTI1_IRQHandler          /* REXTI Line1 */
    .long     EXTI2_IRQHandler          /* REXTI Line2 */
    .long     EXTI3_IRQHandler          /* REXTI Line3 */
    .long     EXTI4_IRQHandler          /* REXTI Line4 */
    .long     EXTI5_IRQHandler          /* REXTI Line5 */
    .long     EXTI6_IRQHandler          /* REXTI Line6 */
    .long     EXTI7_IRQHandler          /* REXTI Line7 */
    .long     EXTI8_IRQHandler          /* REXTI Line8 */
    .long     EXTI9_IRQHandler          /* REXTI Line9 */
    .long     EXTI10_IRQHandler         /* EXTI Line10 */
    .long     EXTI11_IRQHandler         /* EXTI Line11 */
    .long     EXTI12_IRQHandler         /* EXTI Line12 */
    .long     EXTI13_IRQHandler         /* EXTI Line13 */
    .long     EXTI14_IRQHandler         /* EXTI Line14 */
    .long     EXTI15_IRQHandler         /* EXTI Line15 */
    .long     DMAMUX1_IRQHandler        /* DMAMUX1 non-secure */
    .long     DMAMUX1_IRQHandler_S      /* DMAMUX1 secure */
    .long     DMA1_Channel1_IRQHandler  /* DMA1 Channel 1 */
    .long     DMA1_Channel2_IRQHandler  /* DMA1 Channel 2 */
    .long     DMA1_Channel3_IRQHandler  /* DMA1 Channel 3 */
    .long     DMA1_Channel4_IRQHandler  /* DMA1 Channel 4 */
    .long     DMA1_Channel5_IRQHandler  /* DMA1 Channel 5 */
    .long     DMA1_Channel6_IRQHandler  /* DMA1 Channel 6 */
    .long     DMA1_Channel7_IRQHandler  /* DMA1 Channel 7 */
    .long     DMA1_Channel8_IRQHandler  /* DMA1 Channel 8 */
    .long     ADC1_2_IRQHandler         /* ADC1 & ADC2 */
    .long     DAC_IRQHandler            /* DAC1&2 underrun errors */
    .long     FDCAN1_IT0_IRQHandler     /* FDCAN1 Interrupt 0 */
    .long     FDCAN1_IT1_IRQHandler     /* FDCAN1 Interrupt 1 */
    .long     TIM1_BRK_IRQHandler       /* TIM1 Break */
    .long     TIM1_UP_IRQHandler        /* TIM1 Update */
    .long     TIM1_TRG_COM_IRQHandler   /* TIM1 Trigger and Commutation */
    .long     TIM1_CC_IRQHandler        /* TIM1 Capture Compare */
    .long     TIM2_IRQHandler           /* TIM2 */
    .long     TIM3_IRQHandler           /* TIM3 */
    .long     TIM4_IRQHandler           /* TIM4 */
    .long     TIM5_IRQHandler           /* TIM5 */
    .long     TIM6_IRQHandler           /* TIM6 */
    .long     TIM7_IRQHandler           /* TIM7 */
    .long     TIM8_BRK_IRQHandler       /* TIM8 Break */
    .long     TIM8_UP_IRQHandler        /* TIM8 Update */
    .long     TIM8_TRG_COM_IRQHandler   /* TIM8 Trigger and Commutation */
    .long     TIM8_CC_IRQHandler        /* TIM8 Capture Compare */
    .long     I2C1_EV_IRQHandler        /* I2C1 Event */
    .long     I2C1_ER_IRQHandler        /* I2C1 Error */
    .long     I2C2_EV_IRQHandler        /* I2C2 Event */
    .long     I2C2_ER_IRQHandler        /* I2C2 Error */
    .long     SPI1_IRQHandler           /* SPI1 */
    .long     SPI2_IRQHandler           /* SPI2 */
    .long     USART1_IRQHandler         /* USART1 */
    .long     USART2_IRQHandler         /* USART2 */
    .long     USART3_IRQHandler         /* USART3 */
    .long     UART4_IRQHandler          /* UART4 */
    .long     UART5_IRQHandler          /* UART5 */
    .long     LPUART1_IRQHandler        /* LP UART1 */
    .long     LPTIM1_IRQHandler         /* LP TIM1 */
    .long     LPTIM2_IRQHandler         /* LP TIM2 */
    .long     TIM15_IRQHandler          /* TIM15 */
    .long     TIM16_IRQHandler          /* TIM16 */
    .long     TIM17_IRQHandler          /* TIM17 */
    .long     COMP_IRQHandler           /* COMP1&2 */
    .long     USB_FS_IRQHandler         /* USB FS */
    .long     CRS_IRQHandler            /* CRS */
    .long     FMC_IRQHandler            /* FMC */
    .long     OCTOSPI1_IRQHandler       /* OctoSPI1 global interrupt */
    .long     0                         /* Reserved */
    .long     SDMMC1_IRQHandler         /* SDMMC1 */
    .long     0                         /* Reserved */
    .long     DMA2_Channel1_IRQHandler  /* DMA2 Channel 1 */
    .long     DMA2_Channel2_IRQHandler  /* DMA2 Channel 2 */
    .long     DMA2_Channel3_IRQHandler  /* DMA2 Channel 3 */
    .long     DMA2_Channel4_IRQHandler  /* DMA2 Channel 4 */
    .long     DMA2_Channel5_IRQHandler  /* DMA2 Channel 5 */
    .long     DMA2_Channel6_IRQHandler  /* DMA2 Channel 6 */
    .long     DMA2_Channel7_IRQHandler  /* DMA2 Channel 7 */
    .long     DMA2_Channel8_IRQHandler  /* DMA2 Channel 8 */
    .long     I2C3_EV_IRQHandler        /* I2C3 event */
    .long     I2C3_ER_IRQHandler        /* I2C3 error */
    .long     SAI1_IRQHandler           /* Serial Audio Interface 1 global interrupt */
    .long     SAI2_IRQHandler           /* Serial Audio Interface 2 global interrupt */
    .long     TSC_IRQHandler            /* Touch Sense Controller global interrupt */
    .long     AES_IRQHandler            /* AES global interrupt */
    .long     RNG_IRQHandler            /* RNG global interrupt */
    .long     FPU_IRQHandler            /* FPU */
    .long     HASH_IRQHandler           /* HASH global interrupt */
    .long     PKA_IRQHandler            /* PKA global interrupt */
    .long     LPTIM3_IRQHandler         /* LP TIM3 */
    .long     SPI3_IRQHandler           /* SPI3 */
    .long     I2C4_ER_IRQHandler        /* I2C4 error */
    .long     I2C4_EV_IRQHandler        /* I2C4 event */
    .long     DFSDM1_FLT0_IRQHandler    /* DFSDM1 Filter 0 global Interrupt */
    .long     DFSDM1_FLT1_IRQHandler    /* DFSDM1 Filter 1 global Interrupt */
    .long     DFSDM1_FLT2_IRQHandler    /* DFSDM1 Filter 2 global Interrupt */
    .long     DFSDM1_FLT3_IRQHandler    /* DFSDM1 Filter 3 global Interrupt */
    .long     UCPD1_IRQHandler          /* UCPD1 */
    .long     ICACHE_IRQHandler         /* ICACHE */
    .long     OTFDEC1_IRQHandler        /* OTFDEC1 */

    .size    __Vectors, . - __Vectors

    .text
    .thumb
    .thumb_func
    .align    2
    .globl    Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:
/*  Firstly it copies data from read only memory to RAM. There are two schemes
 *  to copy. One can copy more than one sections. Another can only copy
 *  one section.  The former scheme needs more instructions and read-only
 *  data to implement than the latter.
 *  Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.  */

#ifdef __STARTUP_COPY_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of triplets, each of which specify:
 *    offset 0: LMA of start of a section to copy from
 *    offset 4: VMA of start of a section to copy to
 *    offset 8: size of the section to copy. Must be multiply of 4
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
    ldr    r4, =__copy_table_start__
    ldr    r5, =__copy_table_end__

.L_loop0:
    cmp    r4, r5
    bge    .L_loop0_done
    ldr    r1, [r4]
    ldr    r2, [r4, #4]
    ldr    r3, [r4, #8]

.L_loop0_0:
    subs    r3, #4
    blt    .L_loop0_0_done
    ldr    r0, [r1, r3]
    str    r0, [r2, r3]
    b    .L_loop0_0

.L_loop0_0_done:
    adds    r4, #12
    b    .L_loop0

.L_loop0_done:
#else
/*  Single section scheme.
 *
 *  The ranges of copy from/to are specified by following symbols
 *    __etext: LMA of start of the section to copy from. Usually end of text
 *    __data_start__: VMA of start of the section to copy to
 *    __data_end__: VMA of end of the section to copy to
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
    ldr    r1, =__etext
    ldr    r2, =__data_start__
    ldr    r3, =__data_end__

    subs    r3, r2
    ble    .L_loop1_done

.L_loop1:
    subs    r3, #4
    ldr    r0, [r1,r3]
    str    r0, [r2,r3]
    bgt    .L_loop1

.L_loop1_done:
#endif /*__STARTUP_COPY_MULTIPLE */

/*  This part of work usually is done in C library startup code. Otherwise,
 *  define this macro to enable it in this startup.
 *
 *  There are two schemes too. One can clear multiple BSS sections. Another
 *  can only clear one section. The former is more size expensive than the
 *  latter.
 *
 *  Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 *  Otherwise efine macro __STARTUP_CLEAR_BSS to choose the later.
 */
#ifdef __STARTUP_CLEAR_BSS_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of tuples specifying:
 *    offset 0: Start of a BSS section
 *    offset 4: Size of this BSS section. Must be multiply of 4
 */
    ldr    r3, =__zero_table_start__
    ldr    r4, =__zero_table_end__

.L_loop2:
    cmp    r3, r4
    bge    .L_loop2_done
    ldr    r1, [r3]
    ldr    r2, [r3, #4]
    movs    r0, 0

.L_loop2_0:
    subs    r2, #4
    blt    .L_loop2_0_done
    str    r0, [r1, r2]
    b    .L_loop2_0
.L_loop2_0_done:

    adds    r3, #8
    b    .L_loop2
.L_loop2_done:
#elif defined (__STARTUP_CLEAR_BSS)
/*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    __bss_start__: start of the BSS section.
 *    __bss_end__: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
    ldr    r1, =__bss_start__
    ldr    r2, =__bss_end__

    movs    r0, 0

    subs    r2, r1
    ble    .L_loop3_done

.L_loop3:
    subs    r2, #4
    str    r0, [r1, r2]
    bgt    .L_loop3
.L_loop3_done:
#endif /* __STARTUP_CLEAR_BSS_MULTIPLE || __STARTUP_CLEAR_BSS */

    bl    SystemInit

#ifndef __START
#define __START _start
#endif
    bl    __START

    .pool
    .size    Reset_Handler, . - Reset_Handler


/*  Macro to define default handlers. */
    .macro    def_irq_handler    handler_name
    .align    1
    .thumb_func
    .weak    \handler_name
    \handler_name:
    b        \handler_name
    .endm

   def_irq_handler             NMI_Handler
   def_irq_handler             HardFault_Handler
   def_irq_handler             MemManage_Handler
   def_irq_handler             BusFault_Handler
   def_irq_handler             UsageFault_Handler
   def_irq_handler             SecureFault_Handler
   def_irq_handler             SVC_Handler
   def_irq_handler             DebugMon_Handler
   def_irq_handler             PendSV_Handler
   def_irq_handler             SysTick_Handler
   def_irq_handler             WWDG_IRQHandler
   def_irq_handler             PVD_PVM_IRQHandler
   def_irq_handler             RTC_IRQHandler
   def_irq_handler             RTC_IRQHandler_S
   def_irq_handler             TAMP_IRQHandler
   def_irq_handler             TAMP_IRQHandler_S
   def_irq_handler             FLASH_IRQHandler
   def_irq_handler             FLASH_IRQHandler_S
   def_irq_handler             SERR_IRQHandler
   def_irq_handler             RCC_IRQHandler
   def_irq_handler             RCC_IRQHandler_S
   def_irq_handler             EXTI0_IRQHandler
   def_irq_handler             EXTI1_IRQHandler
   def_irq_handler             EXTI2_IRQHandler
   def_irq_handler             EXTI3_IRQHandler
   def_irq_handler             EXTI4_IRQHandler
   def_irq_handler             EXTI5_IRQHandler
   def_irq_handler             EXTI6_IRQHandler
   def_irq_handler             EXTI7_IRQHandler
   def_irq_handler             EXTI8_IRQHandler
   def_irq_handler             EXTI9_IRQHandler
   def_irq_handler             EXTI10_IRQHandler
   def_irq_handler             EXTI11_IRQHandler
   def_irq_handler             EXTI12_IRQHandler
   def_irq_handler             EXTI13_IRQHandler
   def_irq_handler             EXTI14_IRQHandler
   def_irq_handler             EXTI15_IRQHandler
   def_irq_handler             DMAMUX1_IRQHandler
   def_irq_handler             DMAMUX1_IRQHandler_S
   def_irq_handler             DMA1_Channel1_IRQHandler
   def_irq_handler             DMA1_Channel2_IRQHandler
   def_irq_handler             DMA1_Channel3_IRQHandler
   def_irq_handler             DMA1_Channel4_IRQHandler
   def_irq_handler             DMA1_Channel5_IRQHandler
   def_irq_handler             DMA1_Channel6_IRQHandler
   def_irq_handler             DMA1_Channel7_IRQHandler
   def_irq_handler             DMA1_Channel8_IRQHandler
   def_irq_handler             ADC1_2_IRQHandler
   def_irq_handler             DAC_IRQHandler
   def_irq_handler             FDCAN1_IT0_IRQHandler
   def_irq_handler             FDCAN1_IT1_IRQHandler
   def_irq_handler             TIM1_BRK_IRQHandler
   def_irq_handler             TIM1_UP_IRQHandler
   def_irq_handler             TIM1_TRG_COM_IRQHandler
   def_irq_handler             TIM1_CC_IRQHandler
   def_irq_handler             TIM2_IRQHandler
   def_irq_handler             TIM3_IRQHandler
   def_irq_handler             TIM4_IRQHandler
   def_irq_handler             TIM5_IRQHandler
   def_irq_handler             TIM6_IRQHandler
   def_irq_handler             TIM7_IRQHandler
   def_irq_handler             TIM8_BRK_IRQHandler
   def_irq_handler             TIM8_UP_IRQHandler
   def_irq_handler             TIM8_TRG_COM_IRQHandler
   def_irq_handler             TIM8_CC_IRQHandler
   def_irq_handler             I2C1_EV_IRQHandler
   def_irq_handler             I2C1_ER_IRQHandler
   def_irq_handler             I2C2_EV_IRQHandler
   def_irq_handler             I2C2_ER_IRQHandler
   def_irq_handler             SPI1_IRQHandler
   def_irq_handler             SPI2_IRQHandler
   def_irq_handler             USART1_IRQHandler
   def_irq_handler             USART2_IRQHandler
   def_irq_handler             USART3_IRQHandler
   def_irq_handler             UART4_IRQHandler
   def_irq_handler             UART5_IRQHandler
   def_irq_handler             LPUART1_IRQHandler
   def_irq_handler             LPTIM1_IRQHandler
   def_irq_handler             LPTIM2_IRQHandler
   def_irq_handler             TIM15_IRQHandler
   def_irq_handler             TIM16_IRQHandler
   def_irq_handler             TIM17_IRQHandler
   def_irq_handler             COMP_IRQHandler
   def_irq_handler             USB_FS_IRQHandler
   def_irq_handler             CRS_IRQHandler
   def_irq_handler             FMC_IRQHandler
   def_irq_handler             OCTOSPI1_IRQHandler
   def_irq_handler             SDMMC1_IRQHandler
   def_irq_handler             DMA2_Channel1_IRQHandler
   def_irq_handler             DMA2_Channel2_IRQHandler
   def_irq_handler             DMA2_Channel3_IRQHandler
   def_irq_handler             DMA2_Channel4_IRQHandler
   def_irq_handler             DMA2_Channel5_IRQHandler
   def_irq_handler             DMA2_Channel6_IRQHandler
   def_irq_handler             DMA2_Channel7_IRQHandler
   def_irq_handler             DMA2_Channel8_IRQHandler
   def_irq_handler             I2C3_EV_IRQHandler
   def_irq_handler             I2C3_ER_IRQHandler
   def_irq_handler             SAI1_IRQHandler
   def_irq_handler             SAI2_IRQHandler
   def_irq_handler             TSC_IRQHandler
   def_irq_handler             AES_IRQHandler
   def_irq_handler             RNG_IRQHandler
   def_irq_handler             FPU_IRQHandler
   def_irq_handler             HASH_IRQHandler
   def_irq_handler             PKA_IRQHandler
   def_irq_handler             LPTIM3_IRQHandler
   def_irq_handler             SPI3_IRQHandler
   def_irq_handler             I2C4_ER_IRQHandler
   def_irq_handler             I2C4_EV_IRQHandler
   def_irq_handler             DFSDM1_FLT0_IRQHandler
   def_irq_handler             DFSDM1_FLT1_IRQHandler
   def_irq_handler             DFSDM1_FLT2_IRQHandler
   def_irq_handler             DFSDM1_FLT3_IRQHandler
   def_irq_handler             UCPD1_IRQHandler
   def_irq_handler             ICACHE_IRQHandler
   def_irq_handler             OTFDEC1_IRQHandler

    .end
