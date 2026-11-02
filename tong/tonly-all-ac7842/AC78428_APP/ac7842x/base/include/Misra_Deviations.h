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
 * AutoChips Inc. (C) 2026. All rights reserved.
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
#ifndef MISRA_DEVIATIONS_H
#define MISRA_DEVIATIONS_H
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * This page lists all approved MISRA-C:2012 deviations.
 *
 * All deviations are reviewed and approved according to
 * the project quality process.
 *
 * RuleSource:atc_mcu_c_rules_v3.0.rcf
 */

/* =============================================== COMMON ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_COMMON COMMON
 *
 * @li <b>AMCE_QAC_NoUsedApi</b>
 *   @par MISRA C Rule
 *   Rule-2.1.
 *   @par Message
 *   PRQA S 1503.
 *   @par Reason
 *   The function is an external interface or an interrupt service routine and its
 *   signature is defined by external requirements
 *   (e.g. AUTOSAR specification, hardware interrupt vector, or integration interface).
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Each case is reviewed during design and code review, and
 *   internal functions are not allowed to use this suppression.
 *
 * @li <b>AMCE_QAC_NoUsedEnum</b>
 *   @par MISRA C Rule
 *   Rule-2.3.
 *   @par Message
 *   PRQA S 1535.
 *   @par Reason
 *   The enumeration type is defined to comply with AUTOSAR 4.4.0 standard interface specifications.
 *   AUTOSAR requires complete enum definitions for API compatibility, even if some constants
 *   are not used in current implementation or configuration variant.
 *   @par Risk
 *   No risk. Unused enum constants have minimal impact (compile-time only, no runtime overhead).
 *   @par Prevention
 *   Each enum definition must be traceable to AUTOSAR specification or BSW interface requirements.
 *   Usage is reviewed during design and code review to ensure AUTOSAR compliance.
 *   Internal enums without AUTOSAR justification are not allowed to use this deviation.
 *
 * @li <b>AMCE_QAC_OneRefSymbol</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 1533.
 *   @par Reason
 *   The interface is intentionally exposed to separate configuration and implementation logic.
 *   Although the object/function is currently referenced by only one function or one translation unit,
 *   it is designed as a configuration interface to decouple configuration data from functional logic.
 *   Restricting its visibility would break the intended architectural separation and reduce configurability and maintainability.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   The interface is restricted to configuration purposes only and is controlled by architectural guidelines.
 *   All usages are reviewed during design and code review to ensure compliance with the intended separation of configuration and logic.
 *
 * @li <b>AMCE_QAC_3630</b>
 *   @par MISRA C Rule
 *   Dir-4.8.
 *   @par Message
 *   PRQA S 3630.
 *   @par Reason
 *   The structure/union is intentionally defined as a public type to maintain a stable external interface.
 *   At present, there are no users that dereference this type,
 *   but its definition is required to support future integration and avoid interface-breaking changes.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   The definition is restricted to interface purposes only.
 *   All additions or usage of this structure/union are subject to architectural design review to ensure correct encapsulation.
 *
 * @li <b>AMCE_QAC_2985</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2985.
 *   @par Reason
 *   The right-hand value is considered valid by the system design.
 *   An interrupt number of 0 is meaningful for the target hardware,
 *   and division by 1 is deliberately used to maintain consistent and configurable expression patterns.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Such usage is limited to explicitly defined hardware or configuration-related scenarios and is verified through design and code review.
 *
 * @li <b>AMCE_QAC_0316</b>
 *   @par MISRA C Rule
 *   Rule-11.5.
 *   @par Message
 *   PRQA S 0316.
 *   @par Reason
 *   In interrupt handling, context information is often passed as a void * for genericity.
 *   The conversion to a specific object pointer is required to process the interrupt data, and the target type is known and controlled by design.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   The use of this conversion is restricted to interrupt-related code paths and is verified through design documentation and code review.
 *
 * @li <b>AMCE_QAC_3006</b>
 *   @par MISRA C Rule
 *   Dir-4.3.
 *   @par Message
 *   PRQA S 3006.
 *   @par Reason
 *   Inline assembly is required for low-level hardware operations (CPU-specific instructions,
 *   memory barriers, interrupt control) that cannot be expressed in standard C.
 *   The assembly code is encapsulated within HAL functions to maintain interface stability.
 *   @par Risk
 *   No risk. Assembly code is isolated and tested on target hardware.
 *   @par Prevention
 *   All inline assembly is encapsulated in dedicated HAL functions with clear documentation.
 *   Code is reviewed by experienced developers and verified through testing.
 *
 * @li <b>AMCE_QAC_1006</b>
 *   @par MISRA C Rule
 *   Rule-1.2.
 *   @par Message
 *   PRQA S 1006.
 *   @par Reason
 *   Compiler-specific extensions (intrinsics, attributes, alignment directives) are necessary
 *   for accessing hardware features and optimizing performance that cannot be achieved with standard C.
 *   Extensions are isolated to HAL modules to minimize portability impact.
 *   @par Risk
 *   No risk. Extension usage is limited and controlled.
 *   @par Prevention
 *   Compiler extensions are restricted to HAL modules with documented justification.
 *   Conditional compilation provides alternatives where possible. All usage is reviewed during code review.
 *
 * @li <b>AMCE_QAC_1504</b>
 *   @par MISRA C Rule
 *   Rule-8.7.
 *   @par Message
 *   PRQA S 1504.
 *   @par Reason
 *   The object is intentionally defined with external linkage to maintain a stable interface
 *   for configuration or integration purposes, even though it is currently used in only one
 *   translation unit. Declaring it as static would break the intended architectural separation.
 *   @par Risk
 *   No risk. External linkage is controlled by architectural design.
 *   @par Prevention
 *   Interface definitions are reviewed during design phase to ensure proper visibility scope.
 *   Objects with external linkage are limited to configuration and interface purposes.
 *
 * @li <b>AMCE_QAC_3408</b>
 *   @par MISRA C Rule
 *   Rule-8.4.
 *   @par Message
 *   PRQA S 3408.
 *   @par Reason
 *   The object/function is defined with external linkage as part of a generated or configuration module.
 *   External declaration is intentionally omitted to avoid exposing internal implementation details
 *   while maintaining interface encapsulation.
 *   @par Risk
 *   No risk. Usage is controlled within the module boundary.
 *   @par Prevention
 *   Object/function scope and linkage are reviewed during design phase.
 *   External interfaces are properly declared in header files.
 *
 * @li <b>AMCE_QAC_5087</b>
 *   @par MISRA C Rule
 *   Rule-20.1.
 *   @par Message
 *   PRQA S 5087.
 *   @par Reason
 *   MemMap.h include file with multiple includes for control value in specific section.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   None
 *
 * @li <b>AMCE_QAC_2053</b>
 *   @par MISRA C Rule
 *   Rule-5.7.
 *   @par Message
 *   PRQA S 2053.
 *   @par Reason
 *   Tag identifiers are reused in different scopes for consistency in hardware register definitions.
 *   This follows hardware specification conventions and maintains structural similarity with reference manuals.
 *   @par Risk
 *   Low risk. Different scopes prevent actual conflicts.
 *   @par Prevention
 *   Tag naming follows hardware register naming conventions from the reference manual.
 *   Scopes are reviewed to ensure no actual conflicts occur.
 *
 * @li <b>AMCE_QAC_3673</b>
 *   @par MISRA C Rule
 *   Rule-8.13.
 *   @par Message
 *   PRQA S 3673.
 *   @par Reason
 *   The function parameter is defined as non-const pointer to match the external interface specification
 *   required by the interrupt callback mechanism. The callback function signature is dictated by
 *   the interrupt controller interface and cannot be changed to add const qualification, even though the
 *   parameter is not modified within the function.
 *   @par Risk
 *   No risk. The parameter is not modified within the function implementation.
 *   @par Prevention
 *   Interface compliance is verified during integration testing. Code review ensures the parameter
 *   is treated as read-only within the function body.
 *
 * @li <b>AMCE_QAC_0311</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 0311.
 *   @par Reason
 *   Casting from pointer to pointer is required for generic data handling in hardware abstraction layer.
 *   The cast is performed to convert between related data types while maintaining memory layout compatibility.
 *   Target type is verified to be compatible with source type and underlying hardware requirements.
 *   @par Risk
 *   Low risk. Memory layout compatibility is ensured by design.
 *   @par Prevention
 *   Casts are limited to related data types with compatible memory layouts.
 *   All casts are reviewed during design and code review to ensure type compatibility.
 *
 * @li <b>AMCE_QAC_0488</b>
 *   @par MISRA C Rule
 *   Rule-17.3.
 *   @par Message
 *   PRQA S 0488.
 *   @par Reason
 *   Array indexing is performed using parameter which is validated by DEVICE_ASSERT
 *   at the function entry point. The upper layer API ensures that the parameter value is always
 *   within valid bounds before calling HAL functions.
 *   @par Risk
 *   No risk. Array bounds are enforced by parameter validation in upper layers.
 *   @par Prevention
 *   DEVICE_ASSERT checks are placed at function entry points to validate parameter.
 *   Code review ensures proper validation in calling functions.
 *
 * @li <b>AMCE_QAC_2877</b>
 *   @par MISRA C Rule
 *   Dir-4.1.
 *   @par Message
 *   PRQA S 2877.
 *   @par Reason
 *   Loop iteration count is determined by configuration parameters.
 *   The loop bounds are set at compile-time based on validated configuration data
 *   and are guaranteed to be within safe limits by configuration constraints.
 *   @par Risk
 *   No risk. Loop bounds are controlled by validated configuration parameters
 *   and cannot exceed the defined hardware limits.
 *   @par Prevention
 *   Configuration parameters are validated during generation phase.
 *   All loop counters are bounded by configuration-defined maximum values.
 *
 * @li <b>AMCE_QAC_0489</b>
 *   @par MISRA C Rule
 *   Rule-17.5.
 *   @par Message
 *   PRQA S 0489.
 *   @par Reason
 *   The function performs operations that could potentially modify memory locations pointed to by its arguments.
 *   This is necessary for implementing hardware register access functions where direct memory writes are required.
 *   The implementation is safe as the memory addresses are controlled and validated by the system design.
 *   @par Risk
 *   No risk. Memory accesses are controlled and validated by design.
 *   @par Prevention
 *   Memory access functions are reviewed during design and code review.
 *   Address parameters are validated to ensure they point to valid memory regions.
 *
 * @li <b>AMCE_QAC_2983</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2983.
 *   @par Reason
 *   The right-hand value is considered valid by the system design.
 *   The expression is intentionally used to maintain consistent code patterns
 *   or for future extensibility, even though the value may not be used immediately.
 *   @par Risk
 *   No risk. The operation has no side effects and does not impact functionality.
 *   @par Prevention
 *   Such usage is limited to explicitly defined hardware or configuration-related scenarios and is verified through design and code review.
 *
 * @li <b>AMCE_QAC_4342</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_QAC_0326</b>
 *   @par MISRA C Rule
 *   Rule-11.6.
 *   @par Message
 *   PRQA S 0326.
 *   @par Reason
 *   Conversion between void* and unsigned integer is required for interrupt context handling.
 *   The conversion is necessary to pass integer identifiers (like instance IDs) through
 *   generic callback interfaces that accept void* parameters.
 *   @par Risk
 *   No risk. The conversion is controlled and validated by design.
 *   @par Prevention
 *   Conversions are reviewed during design and code review.
 *   Values are validated to ensure they fit within the target type range.
 *
 * @li <b>AMCE_QAC_4332</b>
 *   @par MISRA C Rule
 *   Rule-10.4.
 *   @par Message
 *   PRQA S 4332.
 *   @par Reason
 *   The operands of this operator have different types and the implicit conversion ensures consistent behavior.
 *   This is necessary for hardware register manipulation where different typed values need to be combined
 *   to form a complete register value or mask. The conversion is safe and does not affect functionality.
 *   @par Risk
 *   No risk. Type conversions are validated and safe.
 *   @par Prevention
 *   All implicit conversions are reviewed to ensure they maintain expected behavior.
 *   Unit testing validates the correct operation of converted values.
 *
 * @li <b>AMCE_QAC_4394</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4394.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_QAC_3206</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 3206.
 *   @par Reason
 *   The operation result is intentionally assigned to maintain consistent code patterns
 *   or for future extensibility. Although the value may not be used immediately,
 *   the assignment ensures uniform handling across similar operations.
 *   @par Risk
 *   No risk. The operation has no side effects and does not impact functionality.
 *   @par Prevention
 *   Code is reviewed to ensure intentional usage. Unused assignments are removed unless justified.
 *
 */

/* ================================================ EEP ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_EEP EEP
 *
 * @li <b>AMCE_EEP_1503</b>
 *   @par MISRA C Rule
 *   Rule-2.1.
 *   @par Message
 *   PRQA S 1503.
 *   @par Reason
 *   The function is an external interface or an interrupt service routine and its
 *   signature is defined by external requirements (e.g. AUTOSAR specification, hardware interrupt vector,
 *   or integration interface).
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Each case is reviewed during design and code review, and internal functions are not allowed to use this suppression.
 *
 * @li <b>AMCE_EEP_0771</b>
 *   @par MISRA C Rule
 *   Rule-14.2.
 *   @par Message
 *   PRQA S 0771.
 *   @par Reason
 *   The loop-counter is initialized inside the for loop because the counter value is required for error handling.
 *   This is necessary for implementing multi-block operations in EEP driver.
 *   @par Risk
 *   No risk. The loop counter behavior is well-defined and controlled within the EEP HAL implementation.
 *   @par Prevention
 *   All loop counter operations are reviewed and tested to ensure correct behavior.
 *
 * @li <b>AMCE_EEP_2812</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Pointer dereference is performed on Config pointer which is validated by DEVICE_ASSERT
 *   for NULL_PTR at function entry point. The upper layer API guarantees that valid pointers
 *   are passed to HAL functions.
 *   @par Risk
 *   No risk. Null pointer checks are enforced by parameter validation.
 *   @par Prevention
 *   DEVICE_ASSERT checks for NULL_PTR are placed at function entry points.
 *   All pointer parameters are validated before use.
 *
 * @li <b>AMCE_EEP_2985</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2985.
 *   @par Reason
 *   The right-hand value is considered valid by the system design.
 *   An interrupt number of 0 is meaningful for the target hardware,
 *   and division by 1 is deliberately used to maintain consistent and configurable expression patterns.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Such usage is limited to explicitly defined hardware or configuration-related scenarios and is verified through design and code review.
 *
 * @li <b>AMCE_EEP_3206</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 3206.
 *   @par Reason
 *   The operation result is intentionally assigned to maintain consistent code patterns
 *   or for future extensibility. Although the value may not be used immediately,
 *   the assignment ensures uniform handling across similar operations.
 *   @par Risk
 *   No risk. The operation has no side effects and does not impact functionality.
 *   @par Prevention
 *   Code is reviewed to ensure intentional usage. Unused assignments are removed unless justified.
 *
 * @li <b>AMCE_EEP_3218</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 3218.
 *   @par Reason
 *   Used to define large arrays as static global variables to avoid stack overflow in functions.
 *   @par Risk
 *   Potential confusion with local variable declarations, though the actual risk is low since this is a well-defined usage pattern.
 *   @par Prevention
 *   Ensure these static variables are properly initialized and accessed only within their intended scope.
 *
 * @li <b>AMCE_EEP_3678</b>
 *   @par MISRA C Rule
 *   Dir-4.7.
 *   @par Message
 *   PRQA S 3678.
 *   @par Reason
 *   The function needs to modify input parameters for data buffer operations in EEP driver.
 *   @par Risk
 *   No risk. Parameter modifications are controlled and validated within the EEP HAL implementation.
 *   @par Prevention
 *   All parameter modifications are reviewed and tested to ensure correct behavior.
 *
 * @li <b>AMCE_EEP_0488</b>
 *   @par MISRA C Rule
 *   Rule-17.3.
 *   @par Message
 *   PRQA S 0488.
 *   @par Reason
 *   Array indexing is performed using Instance parameter which is validated by DEVICE_ASSERT
 *   at the function entry point. The upper layer API ensures that the Instance value is always
 *   within valid bounds before calling HAL functions.
 *   @par Risk
 *   No risk. Array bounds are enforced by parameter validation in upper layers.
 *   @par Prevention
 *   DEVICE_ASSERT checks are placed at function entry points to validate Instance parameter.
 *   Code review ensures proper validation in calling functions.
 *
 * @li <b>AMCE_EEP_1840</b>
 *   @par MISRA C Rule
 *   Rule-14.3.
 *   @par Message
 *   PRQA S 1840.
 *   @par Reason
 *   The conditional expression is always true due to parameter values determined by configuration.
 *   Configuration constraints ensure the condition evaluates consistently based on validated parameters.
 *   @par Risk
 *   No risk. Parameter values are controlled by validated configuration data
 *   and the constant condition is intentional based on configuration constraints.
 *   @par Prevention
 *   Configuration parameters are validated during generation phase.
 *   Code review ensures constant conditions are justified by configuration requirements.
 *
 * @li <b>AMCE_EEP_1891</b>
 *   @par MISRA C Rule
 *   Rule-10.5.
 *   @par Message
 *   PRQA S 1891.
 *   @par Reason
 *   The shift operation on unsigned integer is used for bit position calculation when
 *   configuring EEP group mask. The result is used to calculate memory address offsets
 *   based on the group mask (1U << i converts group index to mask).
 *   @par Risk
 *   No risk. The shift count is bounded by the group definition (maximum value ensures no overflow).
 *   @par Prevention
 *   The group values are constrained by design to safe shift ranges.
 *   All calculations are verified during configuration validation.
 *
 * @li <b>AMCE_EEP_2832</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 2832.
 *   @par Reason
 *   The composite expression ensures proper type handling in value assignment for division operations.
 *   This is necessary for calculating block numbers in EEP driver operations.
 *   @par Risk
 *   No risk. Type conversions are validated.
 *   @par Prevention
 *   Expressions are tested for correct type behavior.
 *
 * @li <b>AMCE_EEP_3200</b>
 *   @par MISRA C Rule
 *   Rule-17.7.
 *   @par Message
 *   PRQA S 3200.
 *   @par Reason
 *   The result of the function call is not used because the function is called to release resources
 *   and the return value is not required for subsequent operations.
 *   @par Risk
 *   No risk. The function is called solely for its side effects.
 *   @par Prevention
 *   The function call is carefully reviewed to ensure it doesn't impact functionality.
 *
 */

/* ================================================ ADC ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_ADC ADC
 *
 * @li <b>AMCE_ADC_NoUsedEnum</b>
 *   @par MISRA C Rule
 *   Rule-2.3.
 *   @par Message
 *   PRQA S 1535.
 *   @par Reason
 *   The enumeration type itself is not directly used as a function parameter or return type,
 *   but individual enum members are used as symbolic constants in the code.
 *   This approach provides type-safe constant definitions and maintains AUTOSAR interface compatibility.
 *   @par Risk
 *   No risk. The enum type provides namespace and type checking for constants.
 *   @par Prevention
 *   Enum type usage is reviewed during design phase to ensure members are properly utilized.
 *   Direct integer constants without enum type wrapper are not allowed for AUTOSAR-defined values.
 *
 * @li <b>AMCE_ADC_4342</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_ADC_4394</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4394.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_ADC_4533</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4533.
 *   @par Reason
 *   The shift operation on signed integer is used for sign extension to correctly handle negative values
 *   when extracting calibration parameters from hardware registers. This is an intentional pattern
 *   to convert unsigned bit fields into signed values while preserving the sign bit.
 *   @par Risk
 *   No risk. The shift operations are carefully designed to maintain correct sign extension behavior.
 *   @par Prevention
 *   This pattern is limited to calibration data extraction only and is verified through unit testing
 *   to ensure correct handling of both positive and negative values.
 *
 * @li <b>AMCE_ADC_4391</b>
 *   @par MISRA C Rule
 *   Rule-10.3.
 *   @par Message
 *   PRQA S 4391.
 *   @par Reason
 *   The expression involves bitwise operations on unsigned integers to compute register positions
 *   or bit masks. These operations are necessary for hardware register manipulation and the result
 *   is used in a context where the computed value is guaranteed to be within valid range.
 *   @par Risk
 *   No risk. The bitwise operations are bounded by hardware register specifications.
 *   @par Prevention
 *   All register access patterns are verified against hardware specifications during design review.
 *   Boundary checks ensure values remain within valid ranges.
 *
 * @li <b>AMCE_ADC_3673</b>
 *   @par MISRA C Rule
 *   Rule-8.13.
 *   @par Message
 *   PRQA S 3673.
 *   @par Reason
 *   The function parameter is defined as non-const pointer to match the external interface specification
 *   required by the DMA module callback mechanism. The callback function signature is dictated by
 *   the DMA driver interface and cannot be changed to add const qualification, even though the
 *   parameter is not modified within the function.
 *   @par Risk
 *   No risk. The parameter is not modified within the function implementation.
 *   @par Prevention
 *   Interface compliance is verified during integration testing. Code review ensures the parameter
 *   is treated as read-only within the function body.
 *
 * @li <b>AMCE_ADC_4461</b>
 *   @par MISRA C Rule
 *   Rule-10.1.
 *   @par Message
 *   PRQA S 4461.
 *   @par Reason
 *   The arithmetic operation on enum type is used for calculating buffer indices or offsets
 *   in array access patterns. The enum values are used as integer indices for array operations,
 *   which is necessary for accessing sequential hardware channels or buffer positions.
 *   The operation is bounded by the actual array size and channel count.
 *   @par Risk
 *   No risk. All arithmetic operations are bounded by configuration limits and array sizes.
 *   @par Prevention
 *   Array bounds are verified through configuration validation during initialization.
 *   All calculations are reviewed to ensure they remain within valid ranges.
 *
 * @li <b>AMCE_ADC_1891</b>
 *   @par MISRA C Rule
 *   Rule-10.5.
 *   @par Message
 *   PRQA S 1891.
 *   @par Reason
 *   The shift operation on unsigned char (uint8) is used for bit position calculation when
 *   configuring DMA transfer units. The result is used to calculate memory address offsets
 *   based on the transfer unit size (1 << transfer_unit converts enum to byte count).
 *   This is a standard pattern for translating transfer unit enums to actual byte counts.
 *   @par Risk
 *   No risk. The shift count is bounded by the enum definition (maximum value ensures no overflow).
 *   @par Prevention
 *   The transfer unit enum values are constrained by design to safe shift ranges.
 *   All calculations are verified during configuration validation.
 *
 * @li <b>AMCE_ADC_1535</b>
 *   @par MISRA C Rule
 *   Rule-8.7.
 *   @par Message
 *   PRQA S 1535.
 *   @par Reason
 *   The enum member is intentionally defined in the type definition for completeness
 *   and to provide a comprehensive enumeration of all possible values, even if some
 *   members are not directly referenced in the current implementation. This ensures
 *   that the interface remains stable and extensible for future use cases.
 *   @par Risk
 *   No risk. Unused enum members do not affect code generation or runtime behavior.
 *   @par Prevention
 *   Enum definitions are reviewed during design phase to ensure all members are intentional
 *   and serve a documented purpose in the interface specification.
 *
 * @li <b>AMCE_ADC_2842</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2842.
 *   @par Reason
 *   Array indexing is performed using Instance parameter which is validated by DEVICE_ASSERT
 *   at the function entry point. The upper layer API ensures that the Instance value is always
 *   within valid bounds (Instance < ADC_INSTANCE_MAX) before calling HAL functions.
 *   @par Risk
 *   No risk. Array bounds are enforced by parameter validation in upper layers.
 *   @par Prevention
 *   DEVICE_ASSERT checks are placed at function entry points to validate Instance parameter.
 *   Code review ensures proper validation in calling functions.
 *
 * @li <b>AMCE_ADC_2812</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Pointer dereference is performed on Config pointer which is validated by DEVICE_ASSERT
 *   for NULL_PTR at function entry point. The upper layer API guarantees that valid pointers
 *   are passed to HAL functions.
 *   @par Risk
 *   No risk. Null pointer checks are enforced by parameter validation.
 *   @par Prevention
 *   DEVICE_ASSERT checks for NULL_PTR are placed at function entry points.
 *   All pointer parameters are validated before use.
 *
 * @li <b>AMCE_ADC_4395</b>
 *   @par MISRA C Rule
 *   Rule-10.5.
 *   @par Message
 *   PRQA S 4395.
 *   @par Reason
 *   Conversion from float to unsigned integer is required for ADC calibration calculations
 *   where floating-point results need to be stored in hardware registers. The conversion
 *   is intentional and the floating-point value is within the valid range of the target type.
 *   @par Risk
 *   No risk. Floating-point values are validated to be within valid range before conversion.
 *   @par Prevention
 *   Range checks ensure floating-point values are within target type bounds before conversion.
 *
 * @li <b>AMCE_ADC_3396</b>
 *   @par MISRA C Rule
 *   Rule-12.2.
 *   @par Message
 *   PRQA S 3396.
 *   @par Reason
 *   The expression evaluation order is well-defined for calibration calculations involving
 *   floating-point operations. The order of operations is intentional to maintain precision.
 *   @par Risk
 *   No risk. Expression evaluation follows standard C rules and maintains calculation accuracy.
 *   @par Prevention
 *   Calculation formulas are reviewed and tested to ensure correct results.
 *
 * @li <b>AMCE_ADC_1802</b>
 *   @par MISRA C Rule
 *   Rule-10.5.
 *   @par Message
 *   PRQA S 1802.
 *   @par Reason
 *   Type conversion in calibration formula is necessary to convert floating-point calculation
 *   results to integer format for hardware register programming. The conversion maintains
 *   sufficient precision for ADC calibration requirements.
 *   @par Risk
 *   No risk. Precision loss is acceptable for ADC calibration specifications.
 *   @par Prevention
 *   Calibration formulas are verified against hardware specifications and tested.
 *
 * @li <b>AMCE_ADC_2986</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2986.
 *   @par Reason
 *   The operation result is intentionally assigned to maintain consistent code patterns
 *   or for future extensibility. Although the value may not be used immediately,
 *   the assignment ensures uniform handling across similar operations.
 *   @par Risk
 *   No risk. The operation has no side effects and does not impact functionality.
 *   @par Prevention
 *   Code is reviewed to ensure intentional usage. Unused assignments are removed unless justified.
 *
 * @li <b>AMCE_ADC_2877</b>
 *   @par MISRA C Rule
 *   Dir-4.1.
 *   @par Message
 *   PRQA S 2877.
 *   @par Reason
 *   Loop iteration count is determined by EB Tresos configuration parameters.
 *   The loop bounds are set at compile-time based on validated configuration data
 *   and are guaranteed to be within safe limits by configuration constraints.
 *   @par Risk
 *   No risk. Loop bounds are controlled by validated configuration parameters
 *   and cannot exceed the defined hardware limits.
 *   @par Prevention
 *   Configuration parameters are validated during EB Tresos generation phase.
 *   All loop counters are bounded by configuration-defined maximum values.
 *
 * @li <b>AMCE_ADC_2995</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2995.
 *   @par Reason
 *   The logical condition is always true due to parameter values determined by EB Tresos configuration.
 *   Configuration constraints ensure the condition evaluates consistently based on validated parameters.
 *   @par Risk
 *   No risk. Parameter values are controlled by validated configuration data
 *   and the constant condition is intentional based on configuration constraints.
 *   @par Prevention
 *   Configuration parameters are validated during EB Tresos generation phase.
 *   Code review ensures constant conditions are justified by configuration requirements.
 *
 * @li <b>AMCE_ADC_2991</b>
 *   @par MISRA C Rule
 *   Rule-14.1.
 *   @par Message
 *   PRQA S 2991.
 *   @par Reason
 *   The if condition is always true due to parameter values determined by EB Tresos configuration.
 *   Configuration constraints ensure the condition evaluates consistently based on validated parameters.
 *   @par Risk
 *   No risk. Parameter values are controlled by validated configuration data
 *   and the constant condition is intentional based on configuration constraints.
 *   @par Prevention
 *   Configuration parameters are validated during EB Tresos generation phase.
 *   Code review ensures constant conditions are justified by configuration requirements.
 *
 */

/* ================================================ LIN ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_LIN LIN
 *
 * @li <b>AMCE_LIN_3218</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 3218.
 *   @par Reason
 *   Used to define large arrays as static global variables to avoid stack overflow in functions.
 *   @par Risk
 *   Potential confusion with local variable declarations, though the actual risk is low since this is a well-defined usage pattern.
 *   @par Prevention
 *   Ensure these static variables are properly initialized and accessed only within their intended scope.
 *
 * @li <b>AMCE_LIN_2842</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2842.
 *   @par Reason
 *   Array indexing is performed using Instance parameter which is validated by DEVICE_ASSERT
 *   at the function entry point. The upper layer API ensures that the Instance value is always
 *   within valid bounds (Instance < ADC_INSTANCE_MAX) before calling HAL functions.
 *   @par Risk
 *   No risk. Array bounds are enforced by parameter validation in upper layers.
 *   @par Prevention
 *   DEVICE_ASSERT checks are placed at function entry points to validate Instance parameter.
 *   Code review ensures proper validation in calling functions.
 *
 * @li <b>AMCE_LIN_3387</b>
 *   @par MISRA C Rule
 *   Rule-13.3.
 *   @par Message
 *   PRQA S 3387.
 *   @par Reason
 *   A full expression containing an increment(++) or decrement(--) operator should have not other
 *   potential side effects other than that caused by the increment or decrement operator.
 *   @par Risk
 *   Minimal risk if the expressions are simple and side-effect free.
 *   @par Prevention
 *   Expressions are reviewed to ensure they do not have side effects.
 *
 * @li <b>AMCE_LIN_2877</b>
 *   @par MISRA C Rule
 *   Dir-4.1.
 *   @par Message
 *   PRQA S 2877.
 *   @par Reason
 *   Loop iteration count is determined by EB Tresos configuration parameters.
 *   The loop bounds are set at compile-time based on validated configuration data
 *   and are guaranteed to be within safe limits by configuration constraints.
 *   @par Risk
 *   No risk. Loop bounds are controlled by validated configuration parameters
 *   and cannot exceed the defined hardware limits.
 *   @par Prevention
 *   Configuration parameters are validated during EB Tresos generation phase.
 *   All loop counters are bounded by configuration-defined maximum values.
 *
 */

/* ================================================ SYS ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_SYS SYS
 *
 * @li <b>AMCE_SYS_0791</b>
 *   @par MISRA C Rule
 *   Rule-5.4.
 *   @par Message
 *   PRQA S 0791.
 *   @par Reason
 *   Macro or tag identifier reuses name from outer scope for local abstraction purposes.
 *   The naming is intentional and controlled within limited scope to avoid conflicts.
 *   @par Risk
 *   Low risk. Identifier shadowing is limited to specific contexts and does not cause confusion.
 *   @par Prevention
 *   Naming conventions are reviewed to minimize shadowing. Code review ensures clarity.
 *
 * @li <b>AMCE_SYS_0602</b>
 *   @par MISRA C Rule
 *   Rule-5.1.
 *   @par Message
 *   PRQA S 0602.
 *   @par Reason
 *   External identifier naming follows project conventions with necessary length for clarity.
 *   Truncation risks are managed through naming review process.
 *   @par Risk
 *   No risk. Unused type declarations do not affect code generation or runtime behavior.
 *   @par Prevention
 *   Type declarations are reviewed during design phase and retained only when necessary for interface specification.
 *
 * @li <b>AMCE_SYS_0603</b>
 *   @par MISRA C Rule
 *   Rule-5.1.
 *   @par Message
 *   PRQA S 0603.
 *   @par Reason
 *   Identifier naming follows project-specific conventions ensuring uniqueness.
 *   All identifiers are reviewed for naming conflicts during design phase.
 *   @par Risk
 *   No risk. Unused type declarations do not affect code generation or runtime behavior.
 *   @par Prevention
 *   Type declarations are reviewed during design phase and retained only when necessary for interface specification.
 *
* @li <b>AMCE_SYS_0784</b>
 *   @par MISRA C Rule
 *   Rule-5.5.
 *   @par Message
 *   PRQA S 0784.
 *   @par Reason
 *   Identifier name is reused in different namespace for structural consistency.
 *   Naming pattern follows hardware register definition conventions from datasheet.
 *   @par Risk
 *   Low risk. Different namespaces prevent actual conflicts.
 *   @par Prevention
 *   Naming is reviewed against coding standards. Identifiers are scoped appropriately.
 *
 * @li <b>AMCE_SYS_3006</b>
 *   @par MISRA C Rule
 *   Dir-4.3.
 *   @par Message
 *   PRQA S 3006.
 *   @par Reason
 *   Inline assembly is required for CPU-specific low-level operations that cannot be expressed in C.
 *   Assembly code is isolated and encapsulated in dedicated functions.
 *   @par Risk
 *   No risk. Unused type declarations do not affect code generation or runtime behavior.
 *   @par Prevention
 *   Type declarations are reviewed during design phase and retained only when necessary for interface specification.
 *
 * @li <b>AMCE_SYS_3008</b>
 *   @par MISRA C Rule
 *   Dir-4.3.
 *   @par Message
 *   PRQA S 3008.
 *   @par Reason
 *   Inline assembly is necessary for atomic operations or CPU-specific instructions.
 *   These operations cannot be reliably implemented in standard C.
 *   @par Risk
 *   Low risk. Assembly is isolated, tested, and platform-specific.
 *   @par Prevention
 *   Assembly code is encapsulated in dedicated functions with clear documentation.
 *
 * @li <b>AMCE_SYS_1006</b>
 *   @par MISRA C Rule
 *   Rule-1.2.
 *   @par Message
 *   PRQA S 1006.
 *   @par Reason
 *   Compiler-specific language extensions are necessary for hardware access and optimization.
 *   Extensions are isolated to HAL modules to minimize portability impact.
 *   @par Risk
 *   No risk. Unused type declarations do not affect code generation or runtime behavior.
 *   @par Prevention
 *   Type declarations are reviewed during design phase and retained only when necessary for interface specification.
 *
 * @li <b>AMCE_SYS_5130</b>
 *   @par MISRA C Rule
 *   Rule-17.1.
 *   @par Message
 *   PRQA S 5130.
 *   @par Reason
 *   Variadic function usage is limited to specific wrapper functions for standard library calls.
 *   Parameter types and counts are controlled by design.
 *   @par Risk
 *   Low risk. Variadic functions are used only for well-defined wrappers with type checking.
 *   @par Prevention
 *   Variadic function usage is minimized and limited to specific abstraction layers.
 *
 * @li <b>AMCE_SYS_5209</b>
 *   @par MISRA C Rule
 *   Dir-4.6.
 *   @par Message
 *   PRQA S 5209.
 *   @par Reason
 *   Basic numeric type usage is required for hardware register definitions matching datasheet.
 *   Type definitions ensure correct bit-width for hardware interface.
 *   @par Risk
 *   Low risk. Basic types are used only for hardware interface alignment with specifications.
 *   @par Prevention
 *   Basic type usage is limited to hardware definitions. Typedef usage is preferred elsewhere.
 *
 * @li <b>AMCE_SYS_1337</b>
 *   @par MISRA C Rule
 *   Rule-17.1.
 *   @par Message
 *   PRQA S 1337.
 *   @par Reason
 *   Variadic macro is used for controlled abstraction of debugging or logging interfaces.
 *   Macro usage is limited and reviewed for type safety.
 *   @par Risk
 *   Low risk. Variadic macros are used only for logging/debug with controlled expansion.
 *   @par Prevention
 *   Variadic macro usage is limited to logging abstraction. All expansions are reviewed.
 *
 * @li <b>AMCE_SYS_5140</b>
 *   @par MISRA C Rule
 *   Rule-17.1.
 *   @par Message
 *   PRQA S 5140.
 *   @par Reason
 *   Variadic function wrapper is necessary for platform abstraction layer.
 *   All calls are validated and parameter handling is type-safe.
 *   @par Risk
 *   Low risk. Wrapper functions are carefully designed with parameter validation.
 *   @par Prevention
 *   Variadic wrappers are limited to platform abstraction. All calls are type-checked.
 *
 * @li <b>AMCE_SYS_4434</b>
 *   @par MISRA C Rule
 *   Rule-10.3.
 *   @par Message
 *   PRQA S 4434.
 *   @par Reason
 *   Implicit conversion is bounded and verified for hardware register manipulation.
 *   Value ranges are validated during configuration.
 *   @par Risk
 *   No risk. Implicit conversions are bounded by hardware specifications.
 *   @par Prevention
 *   All conversions are validated. Range checks ensure values fit target type.
 *
 * @li <b>AMCE_SYS_3684</b>
 *   @par MISRA C Rule
 *   Rule-8.11.
 *   @par Message
 *   PRQA S 3684.
 *   @par Reason
 *   Array declared without size is part of flexible array pattern for configuration structures.
 *   Array bounds are managed through associated size field.
 *   @par Risk
 *   Low risk. Array size is controlled by associated length field.
 *   @par Prevention
 *   Flexible arrays are used only in defined patterns. Bounds are always validated.
 *
 * @li <b>AMCE_SYS_3218</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 3218.
 *   @par Reason
 *   Object is defined at file scope to avoid stack overflow or for data persistence.
 *   Scope is intentionally broader than single function usage.
 *   @par Risk
 *   Low risk. File scope is necessary for large data or persistent state.
 *   @par Prevention
 *   File-scope objects are justified and documented. Access is controlled.
 *
 * @li <b>AMCE_SYS_2880</b>
 *   @par MISRA C Rule
 *   Rule-2.1.
 *   @par Message
 *   PRQA S 2880.
 *   @par Reason
 *   Code section is intentionally unreachable under current configuration but retained for completeness.
 *   Configuration-dependent paths are documented and verified.
 *   @par Risk
 *   No risk. Unreachable code is configuration-dependent and documented.This For debug purpose.
 *   @par Prevention
 *   Configuration-dependent code is clearly marked. Reachability is verified per config.
 *
 * @li <b>AMCE_SYS_1503</b>
 *   @par MISRA C Rule
 *   Rule-2.1.
 *   @par Message
 *   PRQA S 1503.
 *   @par Reason
 *   Function is external interface or ISR with signature defined by external requirements.
 *   Not called directly within module but invoked by external system or hardware.
 *   @par Risk
 *   No risk. Function is called externally as documented in interface specification.
 *   @par Prevention
 *   External interfaces are documented. ISR vector tables are verified.
 *
 * @li <b>AMCE_SYS_1535</b>
 *   @par MISRA C Rule
 *   Rule-2.3.
 *   @par Message
 *   PRQA S 1535.
 *   @par Reason
 *   The type declaration is part of the interface specification and maintained for API completeness.
 *   Although currently unused, it ensures interface stability and supports future extensibility.
 *   @par Risk
 *   No risk. Unused type declarations do not affect code generation or runtime behavior.
 *   @par Prevention
 *   Type declarations are reviewed during design phase and retained only when necessary for interface specification.
 *
 * @li <b>AMCE_SYS_1505</b>
 *   @par MISRA C Rule
 *   Rule-8.7.
 *   @par Message
 *   PRQA S 1505.
 *   @par Reason
 *   The function is defined with external linkage for configuration interface purposes.
 *   @par Risk
 *   No risk. External linkage is controlled by design.
 *   @par Prevention
 *   Interface definitions are reviewed during design phase.
 * @li <b>AMCE_SYS_0883</b>
 *   @par MISRA C Rule
 *   Dir-4.10.
 *   @par Message
 *   PRQA S 0883.
 *   @par Reason
 *   Precautions are taken to prevent multiple inclusions through include guards.
 *   @par Risk
 *   No risk. Include guards are properly implemented.
 *   @par Prevention
 *   All header files use standard include guard pattern.
 * @li <b>AMCE_SYS_0841</b>
 *   @par MISRA C Rule
 *   Rule-20.5.
 *   @par Message
 *   PRQA S 0841.
 *   @par Reason
 *   Undef is required for controlled macro redefinition in configuration files.
 *   @par Risk
 *   No risk. Macro undef usage is restricted and controlled.
 *   @par Prevention
 *   Macro redefinition patterns are reviewed during code review.
 * @li <b>AMCE_SYS_0310</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 0310.
 *   @par Reason
 *   Type cast is required for hardware register access with proper alignment guarantees.
 *   @par Risk
 *   No risk. All casts are verified for correct alignment and type compatibility.
 *   @par Prevention
 *   Register access patterns are reviewed against hardware specifications.
  * @li <b>AMCE_SYS_2053</b>
 *   @par MISRA C Rule
 *   Rule-8.7.
 *   @par Message
 *   PRQA S 2053.
 *   @par Reason
 *   The function is defined with external linkage for module integration purposes.
 *   @par Risk
 *   No risk. Interface design is controlled by architecture.
 *   @par Prevention
 *   External linkage is limited to defined interfaces only.
 * @li <b>AMCE_SYS_0342</b>
 *   @par MISRA C Rule
 *   Rule-8.7.
 *   @par Message
 *   PRQA S 0342.
 *   @par Reason
 *   This for example for user and decrease the code size
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   None
 *
 * @li <b>AMCE_SYS_3305</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 3305.
 *   @par Reason
 *   Pointer cast is necessary for hardware abstraction layer implementation.
 *   @par Risk
 *   No risk. Type compatibility is ensured by design.
 *   @par Prevention
 *   All pointer conversions are verified during design review.
 * @li <b>AMCE_SYS_0499</b>
 *   @par MISRA C Rule
 *   Rule-12.2.
 *   @par Message
 *   PRQA S 0499.
 *   @par Reason
 *   Expression evaluation order is well-defined and intentional for this operation.
 *   @par Risk
 *   No risk. Evaluation order follows C standard rules.
 *   @par Prevention
 *   Complex expressions are reviewed to ensure correct evaluation order.
 * @li <b>AMCE_SYS_4342</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   Type conversion is required for register field manipulation with proper type safety.
 *   @par Risk
 *   No risk. Conversions are bounded and verified.
 *   @par Prevention
 *   All type conversions are validated against hardware specifications.
 */
/* ================================================ MCU ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_MCU MCU
 *
 * @li <b>AMCE_MCU_1535</b>
 *   @par MISRA C Rule
 *   Rule-2.3.
 *   @par Message
 *   PRQA S 1535.
 *   @par Reason
 *   The type declaration is part of interface specification for API completeness.
 *   @par Risk
 *   No risk. Unused type declarations do not affect runtime behavior.
 *   @par Prevention
 *   Type declarations are reviewed and retained only when necessary for interface.
 *
 * @li <b>AMCE_MCU_4342</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   Type cast is necessary for MCU peripheral register configuration.
 *   @par Risk
 *   No risk. Type compatibility is verified by design.
 *   @par Prevention
 *   All register access patterns follow hardware specifications.
 * @li <b>AMCE_MCU_3006</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 3006.
 *   @par Reason
 *   Inline assembly is required for CPU-specific low-level operations.
 *   @par Risk
 *   No risk. Assembly code is isolated and tested on target hardware.
 *   @par Prevention
 *   All inline assembly is encapsulated in HAL functions with clear documentation.
 * @li <b>AMCE_MCU_3415</b>
 *   @par MISRA C Rule
 *   Rule-13.5.
 *   @par Message
 *   PRQA S 3415.
 *   @par Reason
 *   Expression with side effects is carefully controlled with well-defined evaluation order.
 *   @par Risk
 *   No risk. Side effect behavior is intentional and documented.
 *   @par Prevention
 *   Expressions with side effects are minimized and reviewed during code review.
  * @li <b>AMCE_MCU_1505</b>
 *   @par MISRA C Rule
 *   Rule-8.7.
 *   @par Message
 *   PRQA S 1505.
 *   @par Reason
 *   The function is defined with external linkage for MCU driver integration.
 *   @par Risk
 *   No risk. Interface visibility is controlled by design.
 *   @par Prevention
 *   External functions are limited to documented driver interfaces.
 *
 */
/* ================================================ MCL ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_MCL MCL
 *
 * @li <b>AMCE_MCL_2812</b>
 *   @par MISRA C Rule
 *   Rule-13.5.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Expression with side effects has well-defined behavior in MCL context.
 *   @par Risk
 *   No risk. Side effects are controlled and intentional.
 *   @par Prevention
 *   Complex expressions are reviewed to ensure predictable behavior.
 * @li <b>AMCE_MCL_2877</b>
 *   @par MISRA C Rule
 *   Rule-13.5.
 *   @par Message
 *   PRQA S 2877.
 *   @par Reason
 *   Loop control expression has well-defined side effect behavior.
 *   @par Risk
 *   No risk. Expression evaluation order is controlled.
 *   @par Prevention
 *   Loop constructs are reviewed for correctness during code review.
 * @li <b>AMCE_MCL_4342</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   Type conversion is required for MCL hardware abstraction.
 *   @par Risk
 *   No risk. Type safety is maintained through proper design.
 *   @par Prevention
 *   All type casts are verified against hardware requirements.
  * @li <b>AMCE_MCL_4394</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4394.
 *   @par Reason
 *   Composite expression ensures proper type handling in value assignment.
 *   @par Risk
 *   No risk. Type conversions are validated.
 *   @par Prevention
 *   All composite expressions are tested for correct behavior.
 * @li <b>AMCE_MCL_2985</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2985.
 *   @par Reason
 *   Operation result is used for side effects or configuration consistency.
 *   @par Risk
 *   No risk. Operation has well-defined purpose.
 *   @par Prevention
 *   Redundant operations are reviewed and removed unless justified.
 */
/* ================================================ GPT ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_GPT GPT
 *
 * @li <b>AMCE_GPT_2842</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2842.
 *   @par Reason
 *   Array access is bounded by validated index within defined limits.
 *   @par Risk
 *   No risk. Index validation ensures bounds compliance.
 *   @par Prevention
 *   All array indices are validated at function entry points.
 * @li <b>AMCE_GPT_0326</b>
 *   @par MISRA C Rule
 *   Rule-11.6.
 *   @par Message
 *   PRQA S 0326.
 *   @par Reason
 *   Cast from pointer to integer type is required for address calculation.
 *   @par Risk
 *   No risk. Address calculations follow hardware memory map.
 *   @par Prevention
 *   Pointer-to-integer casts are limited to address arithmetic only.
 * @li <b>AMCE_GPT_2812</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Pointer dereference is validated for NULL at function entry.
 *   @par Risk
 *   No risk. NULL checks are enforced before dereference.
 *   @par Prevention
 *   All pointer parameters are validated before use.
 * @li <b>AMCE_GPT_2986</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2986.
 *   @par Reason
 *   Operation result is intentionally used for code consistency or future extension.
 *   @par Risk
 *   No risk. Operation has no harmful side effects.
 *   @par Prevention
 *   Redundant operations are reviewed and justified.
 * @li <b>AMCE_GPT_2985</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2985.
 *   @par Reason
 *   Right-hand operand is meaningful for hardware configuration patterns.
 *   @par Risk
 *   No risk. Operation is intentional and verified.
 *   @par Prevention
 *   Expressions are reviewed for correct hardware behavior.
 * @li <b>AMCE_GPT_1505</b>
 *   @par MISRA C Rule
 *   Rule-8.7.
 *   @par Message
 *   PRQA S 1505.
 *   @par Reason
 *   The function is defined with external linkage for GPT driver interface.
 *   @par Risk
 *   No risk. External linkage is required by driver architecture.
 *   @par Prevention
 *   Interface functions are limited to documented APIs.
 * @li <b>AMCE_GPT_1535</b>
 *   @par MISRA C Rule
 *   Rule-2.3.
 *   @par Message
 *   PRQA S 1535.
 *   @par Reason
 *   Type declaration is maintained for API completeness and future extensibility.
 *   @par Risk
 *   No risk. Unused types do not affect runtime.
 *   @par Prevention
 *   Type definitions are reviewed during design phase.
 * @li <b>AMCE_GPT_0686</b>
 *   @par MISRA C Rule
 *   Rule-9.3.
 *   @par Message
 *   PRQA S 0686.
 *   @par Reason
 *   Array initialization pattern follows configuration structure requirements.
 *   @par Risk
 *   No risk. Initialization is verified for completeness.
 *   @par Prevention
 *   Array initializers are reviewed against structure definitions.
 * @li <b>AMCE_GPT_4342</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   Type conversion is required for GPT register access patterns.
 *   @par Risk
 *   No risk. Type compatibility is ensured by hardware design.
 *   @par Prevention
 *   Register access patterns follow hardware specifications.
 * @li <b>AMCE_GPT_4304</b>
 *   @par MISRA C Rule
 *   Rule-10.5.
 *   @par Message
 *   PRQA S 4304.
 *   @par Reason
 *   Type cast is necessary for timer value calculations.
 *   @par Risk
 *   No risk. Value ranges are verified before conversion.
 *   @par Prevention
 *   Range checks ensure values are within target type bounds.
 * @li <b>AMCE_GPT_4394</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4394.
 *   @par Reason
 *   Composite expression ensures correct type handling in assignments.
 *   @par Risk
 *   No risk. Type conversions are validated.
 *   @par Prevention
 *   Expressions are tested for correct type behavior.
  * @li <b>AMCE_GPT_4391</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4391.
 *   @par Reason
 *   Composite expression with implicit conversion is bounded by design.
 *   @par Risk
 *   No risk. Value ranges are verified.
 *   @par Prevention
 *   All implicit conversions are reviewed for correctness.
 * @li <b>AMCE_GPT_4404</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4404.
 *   @par Reason
 *   Composite expression with cast is required for timer configuration.
 *   @par Risk
 *   No risk. Expression result is validated.
 *   @par Prevention
 *   Configuration values are verified during initialization.
 *
 */
/* ================================================ CRC ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_CRC CRC
 *
 * @li <b>AMCE_CRC_4342</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   Type conversion is required for CRC hardware register access.
 *   @par Risk
 *   No risk. Type compatibility is verified.
 *   @par Prevention
 *   Register operations follow hardware specifications.
 * @li <b>AMCE_CRC_2877</b>
 *   @par MISRA C Rule
 *   Rule-13.5.
 *   @par Message
 *   PRQA S 2877.
 *   @par Reason
 *   Expression side effects are controlled and well-defined.
 *   @par Risk
 *   No risk. Evaluation order is intentional.
 *   @par Prevention
 *   Complex expressions are reviewed during code review.
 * @li <b>AMCE_CRC_3218</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 3218.
 *   @par Reason
 *   Object is defined at file scope to avoid stack overflow.
 *   @par Risk
 *   No risk. Object lifetime is properly managed.
 *   @par Prevention
 *   Static objects are initialized and accessed within defined scope.
 *
 */

/* ================================================ FLS ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_FLS FLS
 *
 * @li <b>AMCE_FLS_3218</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 3218.
 *   @par Reason
 *   Used to define large arrays as static global variables to avoid stack overflow in functions.
 *   @par Risk
 *   Potential confusion with local variable declarations, though the actual risk is low since this is a well-defined usage pattern.
 *   @par Prevention
 *   Ensure these static variables are properly initialized and accessed only within their intended scope.
 *
 * @li <b>AMCE_FLS_3408</b>
 *   @par MISRA C Rule
 *   Rule-8.4.
 *   @par Message
 *   PRQA S 3408.
 *   @par Reason
 *   The object/function is defined with external linkage as part of a generated or configuration module.
 *   External declaration is intentionally omitted to avoid exposing internal implementation details
 *   while maintaining interface encapsulation.
 *   @par Risk
 *   No risk. Usage is controlled within the module boundary.
 *   @par Prevention
 *   Object/function scope and linkage are reviewed during design phase.
 *   External interfaces are properly declared in header files.
 *
 * @li <b>AMCE_FLS_1503</b>
 *   @par MISRA C Rule
 *   Rule-2.1.
 *   @par Message
 *   PRQA S 1503.
 *   @par Reason
 *   The function is an external interface or an interrupt service routine and its
 *   signature is defined by external requirements
 *   (e.g. AUTOSAR specification, hardware interrupt vector, or integration interface).
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Each case is reviewed during design and code review, and
 *   internal functions are not allowed to use this suppression.
 *
 * @li <b>AMCE_FLS_3415</b>
 *   @par MISRA C Rule
 *   Rule-13.5.
 *   @par Message
 *   PRQA S 3415.
 *   @par Reason
 *   Expression with side effects is carefully controlled with well-defined evaluation order.
 *   @par Risk
 *   No risk. Side effect behavior is intentional and documented.
 *   @par Prevention
 *   Expressions with side effects are minimized and reviewed during code review.
 *
 * @li <b>AMCE_FLS_3006</b>
 *   @par MISRA C Rule
 *   Dir-4.3.
 *   @par Message
 *   PRQA S 3006.
 *   @par Reason
 *   Inline assembly is required for low-level hardware operations (CPU-specific instructions,
 *   memory barriers, interrupt control) that cannot be expressed in standard C.
 *   The assembly code is encapsulated within HAL functions to maintain interface stability.
 *   @par Risk
 *   No risk. Assembly code is isolated and tested on target hardware.
 *   @par Prevention
 *   All inline assembly is encapsulated in dedicated HAL functions with clear documentation.
 *   Code is reviewed by experienced developers and verified through testing.
 *
 * @li <b>AMCE_FLS_1006</b>
 *   @par MISRA C Rule
 *   Rule-1.2.
 *   @par Message
 *   PRQA S 1006.
 *   @par Reason
 *   Compiler-specific extensions (intrinsics, attributes, alignment directives) are necessary
 *   for accessing hardware features and optimizing performance that cannot be achieved with standard C.
 *   Extensions are isolated to HAL modules to minimize portability impact.
 *   @par Risk
 *   No risk. Extension usage is limited and controlled.
 *   @par Prevention
 *   Compiler extensions are restricted to HAL modules with documented justification.
 *   Conditional compilation provides alternatives where possible. All usage is reviewed during code review.
 *
 * @li <b>AMCE_FLS_2889</b>
 *   @par MISRA C Rule
 *   Rule-13.2.
 *   @par Message
 *   PRQA S 2889.
 *   @par Reason
 *   The value of an expression and its persistent side effects shall be the same under all permitted evaluation orders.
 *   This deviation is allowed for performance optimization in flash operations where specific order of operations is not affecting the final result.
 *   @par Risk
 *   Minimal risk if expressions are carefully designed to be order-independent.
 *   @par Prevention
 *   Careful code reviews ensure that the expression evaluation order does not affect the intended functionality.
 *
 * @li <b>AMCE_FLS_0326</b>
 *   @par MISRA C Rule
 *   Rule-11.6.
 *   @par Message
 *   PRQA S 0326.
 *   @par Reason
 *   Cast from pointer to integer type is required for address calculation in flash operations.
 *   @par Risk
 *   No risk. Address calculations follow hardware memory map.
 *   @par Prevention
 *   Pointer-to-integer casts are limited to address arithmetic only.
 *
 * @li <b>AMCE_FLS_0305</b>
 *   @par MISRA C Rule
 *   Rule-11.1.
 *   @par Message
 *   PRQA S 0305.
 *   @par Reason
 *   Conversion from pointer to object type to an arithmetic type is required for address calculation in flash operations.
 *   @par Risk
 *   No risk. Type compatibility is ensured by design.
 *   @par Prevention
 *   All pointer conversions are verified during design review.
 *
 * @li <b>AMCE_FLS_3387</b>
 *   @par MISRA C Rule
 *   Rule-13.6.
 *   @par Message
 *   PRQA S 3387.
 *   @par Reason
 *   The operand of the sizeof operator shall not contain any expression which has potential side-effects.
 *   This is required in flash operations where we need to measure the size of structures that contain elements initialized with expressions.
 *   @par Risk
 *   Minimal risk if the expressions are simple and side-effect free.
 *   @par Prevention
 *   Expressions are reviewed to ensure they do not have side effects.
 *
 * @li <b>AMCE_FLS_3673</b>
 *   @par MISRA C Rule
 *   Rule-8.13.
 *   @par Message
 *   PRQA S 3673.
 *   @par Reason
 *   The function parameter is defined as non-const pointer to match the external interface specification
 *   required by the flash module callback mechanism. The callback function signature is dictated by
 *   the flash driver interface and cannot be changed to add const qualification, even though the
 *   parameter is not modified within the function.
 *   @par Risk
 *   No risk. The parameter is not modified within the function implementation.
 *   @par Prevention
 *   Interface compliance is verified during integration testing. Code review ensures the parameter
 *   is treated as read-only within the function body.
 *
 * @li <b>AMCE_FLS_2812</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Pointer dereference is performed on Config pointer which is validated by DEVICE_ASSERT
 *   for NULL_PTR at function entry point. The upper layer API guarantees that valid pointers
 *   are passed to HAL functions.
 *   @par Risk
 *   No risk. Null pointer checks are enforced by parameter validation.
 *   @par Prevention
 *   DEVICE_ASSERT checks for NULL_PTR are placed at function entry points.
 *   All pointer parameters are validated before use.
 *
 * @li <b>AMCE_FLS_2986</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2986.
 *   @par Reason
 *   The operation result is intentionally assigned to maintain consistent code patterns
 *   or for future extensibility. Although the value may not be used immediately,
 *   the assignment ensures uniform handling across similar operations.
 *   @par Risk
 *   No risk. The operation has no side effects and does not impact functionality.
 *   @par Prevention
 *   Code is reviewed to ensure intentional usage. Unused assignments are removed unless justified.
 *
 * @li <b>AMCE_FLS_0310</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 0310.
 *   @par Reason
 *   Type cast is required for hardware register access with proper alignment guarantees.
 *   @par Risk
 *   No risk. All casts are verified for correct alignment and type compatibility.
 *   @par Prevention
 *   Register access patterns are reviewed against hardware specifications.
 *
 * @li <b>AMCE_FLS_3305</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 3305.
 *   @par Reason
 *   Pointer cast is necessary for hardware abstraction layer implementation.
 *   @par Risk
 *   No risk. Type compatibility is ensured by design.
 *   @par Prevention
 *   All pointer conversions are verified during design review.
 */

/* ================================================ ICU ============================================ */
/**
 * @li <b>AMCE_ICU_1514</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 1514.
 *   @par Reason
 *   The object 'Icu_DmaTransValues' is only referenced by function 'Icu_Ipw_ConfigDmaCh', in the translation unit where it is defined.
 *   This is intentional as the object serves as a configuration parameter specifically for this function and keeping it file-local
 *   enhances encapsulation and prevents inappropriate usage elsewhere in the codebase.
 *   @par Risk
 *   No risk. The limited scope reduces potential for misuse.
 *   @par Prevention
 *   The object's scope is intentionally limited to the defining translation unit.
 */
/* ================================================ PWM ============================================ */
/**
 * @li <b>AMCE_PWM_1535</b>
 *   @par MISRA C Rule
 *   Rule-8.7.
 *   @par Message
 *   PRQA S 1535.
 *   @par Reason
 *   The enum member is intentionally defined in the type definition for completeness
 *   and to provide a comprehensive enumeration of all possible values, even if some
 *   members are not directly referenced in the current implementation. This ensures
 *   that the interface remains stable and extensible for future use cases.
 *   @par Risk
 *   No risk. Unused enum members do not affect code generation or runtime behavior.
 *   @par Prevention
 *   Enum definitions are reviewed during design phase to ensure all members are intentional
 *   and serve a documented purpose in the interface specification.
 *
 * @li <b>AMCE_PWM_0779</b>
 *   @par MISRA C Rule
 *   Rule-5.1.
 *   @par Message
 *   PRQA S 0779.
 *   @par Reason
 *   Project-specific naming or legacy identifier is required for compatibility with external tools or auto-generated code.
 *   @par Risk
 *   Low risk. Naming is controlled and reviewed to avoid collisions.
 *   @par Prevention
 *   Naming conventions and any deviations are documented and reviewed during design.
 *
 * @li <b>AMCE_PWM_3678</b>
 *   @par MISRA C Rule
 *   Rule-8.13.
 *   @par Message
 *   PRQA S 3678.
 *   @par Reason
 *   The object referenced by 'PwmInputCfg' is not modified through it, so 'PwmInputCfg' could be declared with type
 *   'Pwm_Hal_InputCg const[1]'.
 *   @par Risk
 *   Low risk. This structure is defined in the Hal layer, and the structure defined in the Hal layer is writable.
 *   @par Prevention
 *   Cannot remove its writable attribute.
 *
 */

/* ================================================ SENT ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_SENT SENT
 *
 * @li <b>AMCE_SENT_3218</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 3218.
 *   @par Reason
 *   Used to define large arrays as static global variables to avoid stack overflow in functions.
 *   @par Risk
 *   Potential confusion with local variable declarations, though the actual risk is low since this is a well-defined usage pattern.
 *   @par Prevention
 *   Ensure these static variables are properly initialized and accessed only within their intended scope.
 *
 * @li <b>AMCE_SENT_4342</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_SENT_2812</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Pointer dereference is performed on Config pointer which is validated by DEVICE_ASSERT
 *   for NULL_PTR at function entry point. The upper layer API guarantees that valid pointers
 *   are passed to HAL functions.
 *   @par Risk
 *   No risk. Null pointer checks are enforced by parameter validation.
 *   @par Prevention
 *   DEVICE_ASSERT checks for NULL_PTR are placed at function entry points.
 *   All pointer parameters are validated before use.
 */

/* ================================================ UART ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_UART UART
 *
 * @li <b>AMCE_UART_2877</b>
 *   @par MISRA C Rule
 *   Dir-4.1.
 *   @par Message
 *   PRQA S 2877.
 *   @par Reason
 *   Loop iteration count is determined by EB Tresos configuration parameters.
 *   The loop bounds are set at compile-time based on validated configuration data
 *   and are guaranteed to be within safe limits by configuration constraints.
 *   @par Risk
 *   No risk. Loop bounds are controlled by validated configuration parameters
 *   and cannot exceed the defined hardware limits.
 *   @par Prevention
 *   Configuration parameters are validated during EB Tresos generation phase.
 *   All loop counters are bounded by configuration-defined maximum values.
 *
 * @li <b>AMCE_UART_4395</b>
 *   @par MISRA C Rule
 *   Rule-10.5.
 *   @par Message
 *   PRQA S 4395.
 *   @par Reason
 *   Conversion from float to unsigned integer is required for ADC calibration calculations
 *   where floating-point results need to be stored in hardware registers. The conversion
 *   is intentional and the floating-point value is within the valid range of the target type.
 *   @par Risk
 *   No risk. Floating-point values are validated to be within valid range before conversion.
 *   @par Prevention
 *   Range checks ensure floating-point values are within target type bounds before conversion.
 *
 * @li <b>AMCE_UART_3218</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 3218.
 *   @par Reason
 *   Used to define large arrays as static global variables to avoid stack overflow in functions.
 *   @par Risk
 *   Potential confusion with local variable declarations, though the actual risk is low since this is a well-defined usage pattern.
 *   @par Prevention
 *   Ensure these static variables are properly initialized and accessed only within their intended scope.
 *
 * @li <b>AMCE_UART_2842</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2842.
 *   @par Reason
 *   Array indexing is performed using Instance parameter which is validated by DEVICE_ASSERT
 *   at the function entry point. The upper layer API ensures that the Instance value is always
 *   within valid bounds (Instance < ADC_INSTANCE_MAX) before calling HAL functions.
 *   @par Risk
 *   No risk. Array bounds are enforced by parameter validation in upper layers.
 *   @par Prevention
 *   DEVICE_ASSERT checks are placed at function entry points to validate Instance parameter.
 *   Code review ensures proper validation in calling functions.
 *
 * @li <b>AMCE_UART_2812</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Pointer dereference is performed on Config pointer which is validated by DEVICE_ASSERT
 *   for NULL_PTR at function entry point. The upper layer API guarantees that valid pointers
 *   are passed to HAL functions.
 *   @par Risk
 *   No risk. Null pointer checks are enforced by parameter validation.
 *   @par Prevention
 *   DEVICE_ASSERT checks for NULL_PTR are placed at function entry points.
 *   All pointer parameters are validated before use.
 *
 * @li <b>AMCE_UART_3673</b>
 *   @par MISRA C Rule
 *   Rule-8.13.
 *   @par Message
 *   PRQA S 3673.
 *   @par Reason
 *   Unmodified pointer types are allowed in interrupt functions without const modification
 *   because the interrupt callback signature is fixed and cannot be changed.
 *   @par Risk
 *   No risk. The pointers are used safely within the interrupt context.
 *   @par Prevention
 *   All interrupt handlers are reviewed to ensure safe access to non-const parameters.
 *
 * @li <b>AMCE_UART_0326</b>
 *   @par MISRA C Rule
 *   Rule-11.6.
 *   @par Message
 *   PRQA S 0326.
 *   @par Reason
 *   Cast from pointer to integer type is required for address calculation.
 *   @par Risk
 *   No risk. Address calculations follow hardware memory map.
 *   @par Prevention
 *   Pointer-to-integer casts are limited to address arithmetic only.
 *
 * @li <b>AMCE_UART_4342</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_UART_0488</b>
 *   @par MISRA C Rule
 *   Rule-18.4.
 *   @par Message
 *   PRQA S 0488.
 *   @par Reason
 *   Pointer arithmetic is required to update the current position of the transmit buffer.
 *   The pointer is derived from a valid memory region, and the offset is strictly controlled
 *   within the buffer length.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Buffer boundaries are ensured by design. The total transmission length is tracked,
 *   and pointer updates are performed only within valid limits.Code reviews and testing
 *   verify that no invalid pointer access occurs.
 */

/* ================================================ ACMP ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_ACMP ACMP
 *
 * @li <b>AMCE_ACMP_4342</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_ACMP_2842</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2842.
 *   @par Reason
 *   Array indexing is performed using Instance parameter which is validated by DEVICE_ASSERT
 *   at the function entry point. The upper layer API ensures that the Instance value is always
 *   within valid bounds (Instance < ADC_INSTANCE_MAX) before calling HAL functions.
 *   @par Risk
 *   No risk. Array bounds are enforced by parameter validation in upper layers.
 *   @par Prevention
 *   DEVICE_ASSERT checks are placed at function entry points to validate Instance parameter.
 *   Code review ensures proper validation in calling functions.
 *
 * @li <b>AMCE_ACMP_2812</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Pointer dereference is performed on Config pointer which is validated by DEVICE_ASSERT
 *   for NULL_PTR at function entry point. The upper layer API guarantees that valid pointers
 *   are passed to HAL functions.
 *   @par Risk
 *   No risk. Null pointer checks are enforced by parameter validation.
 *   @par Prevention
 *   DEVICE_ASSERT checks for NULL_PTR are placed at function entry points.
 *   All pointer parameters are validated before use.
 */

/* ================================================ EIO ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_EIO EIO
 *
 * @li <b>AMCE_EIO_4342</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_EIO_4394</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4394.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_EIO_2842</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2842.
 *   @par Reason
 *   Array indexing is performed using Instance parameter which is validated by DEVICE_ASSERT
 *   at the function entry point. The upper layer API ensures that the Instance value is always
 *   within valid bounds (Instance < ADC_INSTANCE_MAX) before calling HAL functions.
 *   @par Risk
 *   No risk. Array bounds are enforced by parameter validation in upper layers.
 *   @par Prevention
 *   DEVICE_ASSERT checks are placed at function entry points to validate Instance parameter.
 *   Code review ensures proper validation in calling functions.
 *
 * @li <b>AMCE_EIO_2812</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Pointer dereference is performed on Config pointer which is validated by DEVICE_ASSERT
 *   for NULL_PTR at function entry point. The upper layer API guarantees that valid pointers
 *   are passed to HAL functions.
 *   @par Risk
 *   No risk. Null pointer checks are enforced by parameter validation.
 *   @par Prevention
 *   DEVICE_ASSERT checks for NULL_PTR are placed at function entry points.
 *   All pointer parameters are validated before use.
 *
 * @li <b>AMCE_EIO_3218</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 3218.
 *   @par Reason
 *   Used to define large arrays as static global variables to avoid stack overflow in functions.
 *   @par Risk
 *   Potential confusion with local variable declarations, though the actual risk is low since this is a well-defined usage pattern.
 *   @par Prevention
 *   Ensure these static variables are properly initialized and accessed only within their intended scope.
 *
 * @li <b>AMCE_EIO_0310</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 0310.
 *   @par Reason
 *   Cast pointer value to different data type to ensure proper alignment for direct memory access to hardware registers.
 *   @par Risk
 *   Pointer cast may cause undefined behavior if the target data type has stricter alignment requirements.
 *   @par Prevention
 *   Verify that the target data type alignment requirements are met and that the memory region is properly allocated.
 *
 * @li <b>AMCE_EIO_3305</b>
 *   @par MISRA C Rule
 *   Rule-11.8.
 *   @par Message
 *   PRQA S 3305.
 *   @par Reason
 *   Casting away const/volatile to allow direct memory access to hardware registers for DMA operations.
 *   @par Risk
 *   Potential modification of constant data or improper access to volatile data leading to undefined behavior.
 *   @par Prevention
 *   Ensure that the underlying data is appropriately mutable and that volatile qualifiers are correctly handled.
 *
 * @li <b>AMCE_EIO_3673</b>
 *   @par MISRA C Rule
 *   Rule-8.13.
 *   @par Message
 *   PRQA S 3673.
 *   @par Reason
 *   Unmodified pointer types are allowed in interrupt functions without const modification
 *   because the interrupt callback signature is fixed and cannot be changed.
 *   @par Risk
 *   No risk. The pointers are used safely within the interrupt context.
 *   @par Prevention
 *   All interrupt handlers are reviewed to ensure safe access to non-const parameters.
 *
 * @li <b>AMCE_EIO_0326</b>
 *   @par MISRA C Rule
 *   Rule-11.6.
 *   @par Message
 *   PRQA S 0326.
 *   @par Reason
 *   Cast from pointer to integer type is required for address calculation.
 *   @par Risk
 *   No risk. Address calculations follow hardware memory map.
 *   @par Prevention
 *   Pointer-to-integer casts are limited to address arithmetic only.
 */

/* ================================================ GPIO ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_GPIO GPIO
 *
 * @li <b>AMCE_GPIO_4342</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_GPIO_2842</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2842.
 *   @par Reason
 *   Array indexing is performed using Instance parameter which is validated by DEVICE_ASSERT
 *   at the function entry point. The upper layer API ensures that the Instance value is always
 *   within valid bounds (Instance < ADC_INSTANCE_MAX) before calling HAL functions.
 *   @par Risk
 *   No risk. Array bounds are enforced by parameter validation in upper layers.
 *   @par Prevention
 *   DEVICE_ASSERT checks are placed at function entry points to validate Instance parameter.
 *   Code review ensures proper validation in calling functions.
 *
 * @li <b>AMCE_GPIO_4394</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4394.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_GPIO_2812</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Pointer dereference is performed on Config pointer which is validated by DEVICE_ASSERT
 *   for NULL_PTR at function entry point. The upper layer API guarantees that valid pointers
 *   are passed to HAL functions.
 *   @par Risk
 *   No risk. Null pointer checks are enforced by parameter validation.
 *   @par Prevention
 *   DEVICE_ASSERT checks for NULL_PTR are placed at function entry points.
 *   All pointer parameters are validated before use.
 */

/* ================================================ I2C ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_I2C I2C
 *
 * @li <b>AMCE_I2C_3415</b>
 *   @par MISRA C Rule
 *   Rule-13.5.
 *   @par Message
 *   PRQA S 3415.
 *   @par Reason
 *   Expression with side effects is carefully controlled with well-defined evaluation order.
 *   @par Risk
 *   No risk. Side effect behavior is intentional and documented.
 *   @par Prevention
 *   Expressions with side effects are minimized and reviewed during code review.
 *
 * @li <b>AMCE_I2C_4432</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_I2C_2982</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2985.
 *   @par Reason
 *   The right-hand value is considered valid by the system design.
 *   An interrupt number of 0 is meaningful for the target hardware,
 *   and division by 1 is deliberately used to maintain consistent and configurable expression patterns.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Such usage is limited to explicitly defined hardware or configuration-related scenarios and is verified through design and code review.
 *
 * @li <b>AMCE_I2C_3673</b>
 *   @par MISRA C Rule
 *   Rule-8.13.
 *   @par Message
 *   PRQA S 3673.
 *   @par Reason
 *   The function parameter is defined as non-const pointer to match the external interface specification
 *   required by the DMA module callback mechanism. The callback function signature is dictated by
 *   the DMA driver interface and cannot be changed to add const qualification, even though the
 *   parameter is not modified within the function.
 *   @par Risk
 *   No risk. The parameter is not modified within the function implementation.
 *   @par Prevention
 *   Interface compliance is verified during integration testing. Code review ensures the parameter
 *   is treated as read-only within the function body.
 *
 * @li <b>AMCE_I2C_0326</b>
 *   @par MISRA C Rule
 *   Rule-11.6.
 *   @par Message
 *   PRQA S 0326.
 *   @par Reason
 *   Cast from pointer to integer type is required for address calculation.
 *   @par Risk
 *   No risk. Address calculations follow hardware memory map.
 *   @par Prevention
 *   Pointer-to-integer casts are limited to address arithmetic only.
 *
 * @li <b>AMCE_I2C_3387</b>
 *   @par MISRA C Rule
 *   Rule-13.6.
 *   @par Message
 *   PRQA S 3387.
 *   @par Reason
 *   The operand of the sizeof operator shall not contain any expression which has potential side-effects.
 *   This is required in flash operations where we need to measure the size of structures that contain elements initialized with expressions.
 *   @par Risk
 *   Minimal risk if the expressions are simple and side-effect free.
 *   @par Prevention
 *   Expressions are reviewed to ensure they do not have side effects.
 *
 * @li <b>AMCE_I2C_1505</b>
 *   @par MISRA C Rule
 *   Rule-8.7.
 *   @par Message
 *   PRQA S 1505.
 *   @par Reason
 *   The function is defined with external linkage for configuration interface purposes.
 *   @par Risk
 *   No risk. External linkage is controlled by design.
 *   @par Prevention
 *   Interface definitions are reviewed during design phase.
 */

/* ================================================ SPI ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_SPI SPI
 *
 * @li <b>AMCE_SPI_3415</b>
 *   @par MISRA C Rule
 *   Rule-13.5.
 *   @par Message
 *   PRQA S 3415.
 *   @par Reason
 *   Expression with side effects is carefully controlled with well-defined evaluation order.
 *   @par Risk
 *   No risk. Side effect behavior is intentional and documented.
 *   @par Prevention
 *   Expressions with side effects are minimized and reviewed during code review.
 *
 * @li <b>AMCE_SPI_2812</b>
 *   @par MISRA C Rule
 *   Rule-18.1.
 *   @par Message
 *   PRQA S 2812.
 *   @par Reason
 *   Pointer dereference is performed on Config pointer which is validated by DEVICE_ASSERT
 *   for NULL_PTR at function entry point. The upper layer API guarantees that valid pointers
 *   are passed to HAL functions.
 *   @par Risk
 *   No risk. Null pointer checks are enforced by parameter validation.
 *   @par Prevention
 *   DEVICE_ASSERT checks for NULL_PTR are placed at function entry points.
 *   All pointer parameters are validated before use.
 *
 *
 * @li <b>AMCE_SPI_0326</b>
 *   @par MISRA C Rule
 *   Rule-11.6.
 *   @par Message
 *   PRQA S 0326.
 *   @par Reason
 *   Cast from pointer to integer type is required for address calculation.
 *   @par Risk
 *   No risk. Address calculations follow hardware memory map.
 *   @par Prevention
 *   Pointer-to-integer casts are limited to address arithmetic only.
 *
 * @li <b>AMCE_SPI_2995</b>
 *   @par MISRA C Rule
 *   Rule-2.2.
 *   @par Message
 *   PRQA S 2995.
 *   @par Reason
 *   The logical condition is always true due to parameter values determined by EB Tresos configuration.
 *   Configuration constraints ensure the condition evaluates consistently based on validated parameters.
 *   @par Risk
 *   No risk. Parameter values are controlled by validated configuration data
 *   and the constant condition is intentional based on configuration constraints.
 *   @par Prevention
 *   Configuration parameters are validated during EB Tresos generation phase.
 *   Code review ensures constant conditions are justified by configuration requirements.
 *
 * @li <b>AMCE_SPI_1891</b>
 *   @par MISRA C Rule
 *   Rule-10.5.
 *   @par Message
 *   PRQA S 1891.
 *   @par Reason
 *   The shift operation on unsigned integer is used for bit position calculation when
 *   configuring EEP group mask. The result is used to calculate memory address offsets
 *   based on the group mask (1U << i converts group index to mask).
 *   @par Risk
 *   No risk. The shift count is bounded by the group definition (maximum value ensures no overflow).
 *   @par Prevention
 *   The group values are constrained by design to safe shift ranges.
 *   All calculations are verified during configuration validation.
 *
 * @li <b>AMCE_SPI_3387</b>
 *   @par MISRA C Rule
 *   Rule-13.6.
 *   @par Message
 *   PRQA S 3387.
 *   @par Reason
 *   The operand of the sizeof operator shall not contain any expression which has potential side-effects.
 *   This is required in flash operations where we need to measure the size of structures that contain elements initialized with expressions.
 *   @par Risk
 *   Minimal risk if the expressions are simple and side-effect free.
 *   @par Prevention
 *   Expressions are reviewed to ensure they do not have side effects.
 *
 * @li <b>AMCE_SPI_3673</b>
 *   @par MISRA C Rule
 *   Rule-8.13.
 *   @par Message
 *   PRQA S 3673.
 *   @par Reason
 *   The function parameter is defined as non-const pointer to match the external interface specification
 *   required by the DMA module callback mechanism. The callback function signature is dictated by
 *   the DMA driver interface and cannot be changed to add const qualification, even though the
 *   parameter is not modified within the function.
 *   @par Risk
 *   No risk. The parameter is not modified within the function implementation.
 *   @par Prevention
 *   Interface compliance is verified during integration testing. Code review ensures the parameter
 *   is treated as read-only within the function body.
 *
 * @li <b>AMCE_SPI_3218</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 3218.
 *   @par Reason
 *   Certain large objects are required to be defined at file scope to avoid
 *   excessive stack usage in performance- or memory-constrained contexts.
 *   These objects are intentionally made static globals to guarantee
 *   deterministic lifetime and to prevent stack overflow in interrupt or
 *   deep-call contexts.
 *   @par Risk
 *   If such objects were placed on the stack, stack overflow or runtime
 *   instability could occur under constrained stack conditions.
 *   @par Prevention
 *   The size and usage of file-scope objects are reviewed; allocation is
 *   minimized and documented. Static globals are initialized and accessed
 *   in a controlled manner and covered by unit tests.
 */

/* ================================================ PORT ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_PORT PORT
 *
 * @li <b>AMCE_PORT_4342</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_PORT_4394</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4394.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 */
/* ================================================ CAN ============================================ */
/**
 * @page Autochips_MISRA_Exceptions Autochips MISRA-C:2012 Compliance Exceptions
 *
 * @section Autochips_MISRA_CAN CAN
 *
 * @li <b>AMCE_CAN_4342</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4342.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_CAN_4394</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4394.
 *   @par Reason
 *   The forced conversion of unsigned to enum ensures that there will be no problems (data type) in the current code.
 *   Only suppression of unsigned conversion to enum is allowed, and addition is prohibited in other cases.
 *   @par Risk
 *   No risk.
 *   @par Prevention
 *   Ensure that the project is working properly through unit testing.
 *
 * @li <b>AMCE_CAN_0310</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 0310.
 *   @par Reason
 *   Type cast is required for hardware register access with proper alignment guarantees.
 *   @par Risk
 *   No risk. All casts are verified for correct alignment and type compatibility.
 *   @par Prevention
 *   Register access patterns are reviewed against hardware specifications.
 *
 * @li <b>AMCE_CAN_3305</b>
 *   @par MISRA C Rule
 *   Rule-11.3.
 *   @par Message
 *   PRQA S 3305.
 *   @par Reason
 *   Pointer cast is necessary for hardware abstraction layer implementation.
 *   @par Risk
 *   No risk. Type compatibility is ensured by design.
 *   @par Prevention
 *   All pointer conversions are verified during design review.
 *
 * @li <b>AMCE_CAN_0311</b>
 *   @par MISRA C Rule
 *   Rule-11.2.
 *   @par Message
 *   PRQA S 0311.
 *   @par Reason
 *   Hardware access patterns occasionally require pointer casts within a
 *   tightly controlled context; these casts are verified by design.
 *   @par Risk
 *   Unchecked casts can produce undefined behavior if misused.
 *   @par Prevention
 *   Restrict casting to HAL modules; add comments and unit/integration
 *   tests to validate runtime behavior.
 *
 * @li <b>AMCE_CAN_4322</b>
 *   @par MISRA C Rule
 *   Rule-10.8.
 *   @par Message
 *   PRQA S 4322.
 *   @par Reason
 *   Conversions between different enumeration types are required to interoperate
 *   with generated configuration data and hardware-specific descriptor fields.
 *   These conversions are limited to well-reviewed mapping points where the
 *   source and target enum value sets are compatible by design.
 *   @par Risk
 *   Minimal risk when conversions are limited and validated; incorrect mappings
 *   could lead to logic errors if not reviewed.
 *   @par Prevention
 *   All enum conversions are confined to the HAL/IPW boundary, documented,
 *   and covered by unit/integration tests and code review.
 *
 * @li <b>AMCE_CAN_3673</b>
 *   @par MISRA C Rule
 *   Rule-8.13.
 *   @par Message
 *   PRQA S 3673.
 *   @par Reason
 *   Interrupt and callback APIs often prescribe non-const pointer parameters.
 *   The function signature must match the required callback prototype even when
 *   the parameter is not modified within the implementation.
 *   @par Risk
 *   No risk. The parameter is not modified and usage is documented.
 *   @par Prevention
 *   Document the API contract and ensure implementations treat the parameter
 *   as read-only; verify through code review and testing.
 *
 * @li <b>AMCE_CAN_0488</b>
 *   @par MISRA C Rule
 *   Rule-11.4.
 *   @par Message
 *   PRQA S 0488.
 *   @par Reason
 *   Pointer arithmetic is required for message RAM address calculations in CAN driver.
 *   @par Risk
 *   No risk. Address calculations follow hardware specifications.
 *   @par Prevention
 *   All address calculations are verified against hardware specifications.
 *
 * @li <b>AMCE_CAN_3440</b>
 *   @par MISRA C Rule
 *   Rule-11.4.
 *   @par Message
 *   PRQA S 3440.
 *   @par Reason
 *   Conversion between integer and pointer types is required for memory-mapped register access.
 *   @par Risk
 *   No risk. All conversions are verified for correctness.
 *   @par Prevention
 *   Memory access patterns are reviewed against hardware specifications.
 *
 * @li <b>AMCE_CAN_3387</b>
 *   @par MISRA C Rule
 *   Rule-13.6.
 *   @par Message
 *   PRQA S 3387.
 *   @par Reason
 *   The operand of the sizeof operator shall not contain any expression which has potential side-effects.
 *   This is required in CAN operations where we need to measure the size of structures that contain elements initialized with expressions.
 *   @par Risk
 *   Minimal risk if the expressions are simple and side-effect free.
 *   @par Prevention
 *   Expressions are reviewed to ensure they do not have side effects.
 *
 * @li <b>AMCE_CAN_0404</b>
 *   @par MISRA C Rule
 *   Rule-14.4.
 *   @par Message
 *   PRQA S 0404.
 *   @par Reason
 *   Conditional checks in the CAN IPW/driver inspect runtime status flags and buffer state
 *   (including volatile or hardware-updated fields). Static analysis may report such
 *   expressions as suspicious or potentially redundant; however these checks are required
 *   by the driver logic to correctly determine transmit/receive buffer status and to
 *   perform hardware-specific state transitions.
 *   @par Risk
 *   Minimal risk when conditions are kept simple and reviewed; an incorrect condition may
 *   cause missed notifications or incorrect buffer handling.
 *   @par Prevention
 *   Keep conditional expressions simple and well-documented; validate behavior by unit
 *   tests and code review. Where volatile or hardware-updated fields are involved, ensure
 *   access patterns are consistent with the hardware specification.
 *
 * @li <b>AMCE_CAN_3218</b>
 *   @par MISRA C Rule
 *   Rule-8.9.
 *   @par Message
 *   PRQA S 3218.
 *   @par Reason
 *   Certain large objects are required to be defined at file scope to avoid
 *   excessive stack usage in performance- or memory-constrained contexts.
 *   These objects are intentionally made static globals to guarantee
 *   deterministic lifetime and to prevent stack overflow in interrupt or
 *   deep-call contexts.
 *   @par Risk
 *   If such objects were placed on the stack, stack overflow or runtime
 *   instability could occur under constrained stack conditions.
 *   @par Prevention
 *   The size and usage of file-scope objects are reviewed; allocation is
 *   minimized and documented. Static globals are initialized and accessed
 *   in a controlled manner and covered by unit tests.
 *
 * @li <b>AMCE_CAN_1505</b>
 *   @par MISRA C Rule
 *   Rule-8.7.
 *   @par Message
 *   PRQA S 1505.
 *   @par Reason
 *   Some functions or interfaces are required by the SDK or startup
 *   integration and must remain with external linkage even if their use is
 *   not visible within this translation unit.
 *   @par Risk
 *   Restricting linkage may break integration with external startup or SDK
 *   components, causing missing symbols at link time.
 *   @par Prevention
 *   Such externally-visible functions are documented, reviewed, and limited
 *   to integration points only.
 *
 * @li <b>AMCE_CAN_3415</b>
 *   @par MISRA C Rule
 *   Rule-14.4.
 *   @par Message
 *   PRQA S 3415.
 *   @par Reason
 *   Expressions with potentially reordered evaluation are used where the
 *   right-hand operand is side-effect-free and the overall expression is
 *   validated by design.
 *   @par Risk
 *   Assuming evaluation order when side effects exist can lead to
 *   unpredictable behaviour.
 *   @par Prevention
 *   Ensure operands are side-effect-free or refactor into separate
 *   statements; document intent in comments.
 *
 * @li <b>AMCE_CAN_2877</b>
 *   @par MISRA C Rule
 *   Rule-15.1.
 *   @par Message
 *   PRQA S 2877.
 *   @par Reason
 *   Loops with device-dependent iteration counts are required to support
 *   multiple hardware variants; the number of iterations is safe for the
 *   intended devices.
 *   @par Risk
 *   Incorrect loop bounds may cause performance issues or incorrect
 *   behaviour on unsupported hardware.
 *   @par Prevention
 *   Document variant-specific behavior and guard loops with compile-time
 *   configuration checks.
 *
 * @li <b>AMCE_CAN_1503</b>
 *   @par MISRA C Rule
 *   Rule-2.1.
 *   @par Message
 *   PRQA S 1503.
 *   @par Reason
 *   The symbol is required as an externally-visible entry point (e.g.,
 *   interrupt vector) as mandated by the startup code; it may appear
 *   unused within this translation unit.
 *   @par Risk
 *   Removing or hiding the symbol may lead to missing vector table
 *   entries and system failure at startup.
 *   @par Prevention
 *   Keep such symbols documented and localized to integration modules.
 *
 * @li <b>AMCE_CAN_3408</b>
 *   @par MISRA C Rule
 *   Rule-8.4.
 *   @par Message
 *   PRQA S 3408.
 *   @par Reason
 *   Some functions/objects intentionally omit external declarations to
 *   avoid exposing implementation details while still being referenced by
 *   linker or startup scripts.
 *   @par Risk
 *   Incorrect visibility may lead to unintended symbol collisions or
 *   missing references.
 *   @par Prevention
 *   Keep visibility decisions documented; restrict such patterns to
 *   well-audited modules.s
 */
#ifdef __cplusplus
}
#endif
#endif
/*============================================EOF===================================================*/
