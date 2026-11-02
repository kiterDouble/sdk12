/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2023. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef COMPILERASM_H
#define COMPILERASM_H

#ifdef __cplusplus
extern "C" {
#endif
/* C/C++ compiler macros for placing code or variable in coretest Library defined section */
#if defined (__IAR_SYSTEMS_ICC__)
#define ASM_PLACE_IN_SECTION_HELPER_0(X) #X
#define ASM_PLACE_IN_SECTION_HELPER_1(SECTION_NAME) ASM_PLACE_IN_SECTION_HELPER_0(location = #SECTION_NAME)
#define ASM_PLACE_IN_SECTION_HELPER_2(SECTION_NAME) ASM_PLACE_IN_SECTION_HELPER_1(.SECTION_NAME)
#define ASM_PLACE_IN_SECTION(SECTION_NAME) _Pragma(ASM_PLACE_IN_SECTION_HELPER_2(SECTION_NAME))
#endif  /* __IAR_SYSTEMS_ICC__*/

#if defined (__ghs__) || defined (__GNUC__) || defined (__DCC__)
#define ASM_PLACE_IN_SECTION_HELPER(SECTION_NAME) __attribute__ (( section(#SECTION_NAME) ))
#define ASM_PLACE_IN_SECTION(SECTION_NAME) ASM_PLACE_IN_SECTION_HELPER(.SECTION_NAME)
#endif /* __ghs__ __GNUC__ */

/****************************** Wind River Diab ******************************/
#if defined (__DCC__)
    #define ASM_THUMB .thumb
    #define ASM_THUMB2 .thumb ; .syntax unified

    #define function @tfunc
    #define label @object

    #define ASM_TYPE(name,param) .type name, param

    #define ASM_WORD .long
    #define ASM_SHORT .short

    #define ASM_EXPORT .global
    #define ASM_EXTERN .global

    #define ASM_ALIGN_POWER_OF_TWO(POWER) .align POWER
    #define ASM_LTORG .ltorg

    /* reserves block of data initialized to zero in bytes */
    #define ASM_ALLOC_BYTES(SIZE) .skip SIZE

    #define ASM_SET(NAME,VALUE) .set NAME, VALUE
    #define ASM_DEFINE(NAME,VALUE) .set NAME, VALUE

    #define ASM_SECTION_EXEC(SECTION_NAME) .section .SECTION_NAME,rx
    #define ASM_SECTION_DATA(SECTION_NAME) .section .SECTION_NAME,rw
    #define ASM_SECTION_CONST(SECTION_NAME) .section .SECTION_NAME,r
    #define ASM_SECTION_EXEC_W(SECTION_NAME) .section .SECTION_NAME,rwx
    #define ASM_SECTION_DATA_UNINIT(SECTION_NAME) .section .SECTION_NAME,rwb

    #define ASM_SECTION_DATA_ALIGN(SECTION_NAME, ALIGNMENT) .section .SECTION_NAME,ALIGNMENT,rw
    #define ASM_SECTION_CONST_ALIGN(SECTION_NAME, ALIGNMENT) .section .SECTION_NAME,ALIGNMENT,r
    #define ASM_SECTION_EXEC_W_ALIGN(SECTION_NAME, ALIGNMENT) .section .SECTION_NAME,ALIGNMENT,rwx
    #define ASM_SECTION_DATA_UNINIT_ALIGN(SECTION_NAME, ALIGNMENT) .section .SECTION_NAME,ALIGNMENT,rwb

    #define ASM_BYTES_4 4
    #define ASM_BYTES_256 256

    /* default option is -Xalign-power2 */
    #define ASM_ALIGN_BYTES_4 .align 2
    #define ASM_ALIGN_BYTES_256 .align 8

    #define ASM_OPCODE_START /*opcode start*/
    #define ASM_OPCODE_END /*opcode end*/
    #define ASM_OPCODE16(OPCODE) ASM_SHORT OPCODE

    #define ASM_FILE_END

/******************************** IAR Systems ********************************/
#elif defined (__IAR_SYSTEMS_ICC__)
#elif defined (__IAR_SYSTEMS_ASM__)
    #define ASM_THUMB THUMB
    #define ASM_THUMB2 THUMB
    /* Labels within a Thumb area have bit 0 set to 1 */
    #define ASM_TYPE(name, ident) /* not needed */

    #define ASM_WORD DC32
    #define ASM_SHORT DC16

    #define ASM_EXPORT PUBLIC
    #define ASM_EXTERN EXTERN

    #define ASM_LTORG LTORG

    /* reserves block of data initialized to zero in bytes */
    #define ASM_ALLOC_BYTES(N) DS8 N

    #define ASM_DEFINE(NAME,VALUE) NAME: DEFINE VALUE
    #define ASM_SET(NAME,VALUE) NAME: SET VALUE

    #define ASM_SECTION_EXEC(SECTION_NAME) section .SECTION_NAME :CODE
    #define ASM_SECTION_DATA(SECTION_NAME) section .SECTION_NAME :DATA
    #define ASM_SECTION_CONST(SECTION_NAME) section .SECTION_NAME :CONST
    #define ASM_SECTION_EXEC_W(SECTION_NAME) section .SECTION_NAME :DATA
    #define ASM_SECTION_DATA_UNINIT(SECTION_NAME) section .SECTION_NAME :DATA

    #define ASM_SECTION_DATA_ALIGN(SECTION_NAME, ALIGNMENT) ASM_SECTION_DATA(SECTION_NAME) (ALIGNMENT)
    #define ASM_SECTION_CONST_ALIGN(SECTION_NAME, ALIGNMENT) ASM_SECTION_CONST(SECTION_NAME) (ALIGNMENT)
    #define ASM_SECTION_EXEC_W_ALIGN(SECTION_NAME, ALIGNMENT) ASM_SECTION_EXEC_W(SECTION_NAME) (ALIGNMENT)
    #define ASM_SECTION_DATA_UNINIT_ALIGN(SECTION_NAME, ALIGNMENT) ASM_SECTION_DATA_UNINIT(SECTION_NAME) (ALIGNMENT)

    #define ASM_BYTES_4   2
    #define ASM_BYTES_256 8

    #define ASM_ALIGN_BYTES_4 DATA ALIGNROM 2
    #define ASM_ALIGN_BYTES_256 DATA ALIGNROM 8

    #define ASM_OPCODE_START DATA
    #define ASM_OPCODE_END THUMB
    #define ASM_OPCODE16(OPCODE) ASM_SHORT OPCODE

    #define ASM_FILE_END END

/******************************** Green Hills *********************************/
#elif defined (__ghs__)
    #define function $function
    #define label $object

    #define ASM_THUMB .thumb
    #define ASM_THUMB2 .thumb2

    #define ASM_TYPE(name, ident) .type name, ident

    #define ASM_WORD .long
    #define ASM_SHORT .short

    #define ASM_EXPORT .global
    #define ASM_EXTERN .global

    #define ASM_ALIGN_POWER_OF_TWO(POWER) .align (1 << POWER)
    #define ASM_LTORG .ltorg

    #define ASM_ALLOC_BYTES(N) .space N

    /* has to be at the beginning of a line */
    #define ASM_SET(NAME,VALUE) .set NAME, VALUE
    /* has to be at the beginning of a line */
    #define ASM_DEFINE(NAME,VALUE) .set NAME, VALUE

    #define ASM_SECTION_EXEC(SECTION_NAME) .section .SECTION_NAME,"ax"
    #define ASM_SECTION_DATA(SECTION_NAME) .section .SECTION_NAME,"aw"
    #define ASM_SECTION_CONST(SECTION_NAME) .section .SECTION_NAME,"a"
    #define ASM_SECTION_EXEC_W(SECTION_NAME) .section .SECTION_NAME,"awx"
    #define ASM_SECTION_DATA_UNINIT(SECTION_NAME) .section .SECTION_NAME,"awb"

    #define ASM_SECTION_DATA_ALIGN(SECTION_NAME, ALIGNMENT) ASM_SECTION_DATA(SECTION_NAME)
    #define ASM_SECTION_CONST_ALIGN(SECTION_NAME, ALIGNMENT) ASM_SECTION_CONST(SECTION_NAME)
    #define ASM_SECTION_EXEC_W_ALIGN(SECTION_NAME, ALIGNMENT) ASM_SECTION_EXEC_W(SECTION_NAME)
    #define ASM_SECTION_DATA_UNINIT_ALIGN(SECTION_NAME, ALIGNMENT) ASM_SECTION_DATA_UNINIT(SECTION_NAME)

    #define ASM_BYTES_4   2
    #define ASM_BYTES_256 8

    #define ASM_ALIGN_BYTES_4 ASM_ALIGN_POWER_OF_TWO(2)
    #define ASM_ALIGN_BYTES_256 ASM_ALIGN_POWER_OF_TWO(8)

    #define ASM_OPCODE_START /*opcode start*/
    #define ASM_OPCODE_END /*opcode end*/
    #define ASM_OPCODE16(OPCODE) ASM_SHORT OPCODE

    #define APSR_nzcvq APSR

    #define ASM_FILE_END

/********************************** GNU GCC ***********************************/
#elif (defined (__GNUC__) && !defined (__ghs__))
    #define ASM_THUMB .thumb
    #define ASM_THUMB2 .thumb ; .syntax unified   /* fix Thumb16 mode error*/

    #define ASM_TYPE(name, ident) .type name, %ident

    #define ASM_WORD .long
    #define ASM_SHORT .short

    #define ASM_EXPORT .global
    #define ASM_EXTERN .global

    #define label object

    #define ASM_ALIGN_POWER_OF_TWO(POWER) .align POWER
    #define ASM_LTORG .ltorg

    #define ASM_ALLOC_BYTES(N) .space N

    /* has to be at the beginning of a line */
    #define ASM_SET(NAME,VALUE) .set NAME, VALUE
    /* has to be at the beginning of a line */
    #define ASM_DEFINE(NAME,VALUE) .set NAME, VALUE

    #define ASM_SECTION_EXEC(SECTION_NAME) .section .SECTION_NAME,"ax"
    #define ASM_SECTION_DATA(SECTION_NAME) .section .SECTION_NAME,"aw"
    #define ASM_SECTION_CONST(SECTION_NAME) .section .SECTION_NAME,"a"
    #define ASM_SECTION_EXEC_W(SECTION_NAME) .section .SECTION_NAME,"awx"
    #define ASM_SECTION_DATA_UNINIT(SECTION_NAME) .section .SECTION_NAME,"aw"

    #define ASM_SECTION_DATA_ALIGN(SECTION_NAME, ALIGNMENT) .section .SECTION_NAME,"aw" ; .align ALIGNMENT
    #define ASM_SECTION_CONST_ALIGN(SECTION_NAME, ALIGNMENT) .section .SECTION_NAME,"a" ; .align ALIGNMENT
    #define ASM_SECTION_EXEC_W_ALIGN(SECTION_NAME, ALIGNMENT) .section .SECTION_NAME,"awx" ; .align ALIGNMENT
    #define ASM_SECTION_DATA_UNINIT_ALIGN(SECTION_NAME, ALIGNMENT) .section .SECTION_NAME,"aw" ; .align ALIGNMENT

    #define ASM_BYTES_4   2
    #define ASM_BYTES_256 8

    #define ASM_ALIGN_BYTES_4 ASM_ALIGN_POWER_OF_TWO(2)
    #define ASM_ALIGN_BYTES_256 ASM_ALIGN_POWER_OF_TWO(8)

    #define ASM_OPCODE_START /*opcode start*/
    #define ASM_OPCODE_END /*opcode end*/
    #define ASM_OPCODE16(OPCODE) ASM_SHORT OPCODE

    #define APSR_nzcvq APSR_nzcvq

    #define ASM_FILE_END

#else
    #error "Unsupported compiler. Compiler abstraction needs to be updated to use this compiler."
#endif

#ifdef __cplusplus
}
#endif
#endif /* COMPILERASM_H */
