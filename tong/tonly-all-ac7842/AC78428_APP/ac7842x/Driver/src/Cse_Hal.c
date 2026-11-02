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
 * AutoChips Inc. (C) 2021. All rights reserved.
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

/*!
 * @file CSE_Hal.c
 *
 * @brief This file provides CSE integration functions.
 *
 */
#if defined(AC7840X) || defined(AC7842X)
/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Cse_Reg.h"
#include "Cse_Hal.h"
#include "Core_Hal.h"
/* ============================================  DEFINES AND MACROS  ============================================ */
/*PRQA S 1503 EOF #AMCE_HSM_1503*/
/*PRQA S 1505 EOF #AMCE_HSM_1505*/
/*!
 * @brief Specifies how the data is transferred to/from the CSE.
 */
#define CSE_FUNC_FORMAT_COPY 0u /*!< the data by copy to memory for cmd.*/
#define CSE_FUNC_FORMAT_ADDR  1u/*!< the data by pointer for cmd.*/
/* ============================================= TYPEDEFS ================================================ */
/*!
 * @brief Specifies if the information is the first or a following function call.
 */
typedef enum
{
    CSE_CALL_SEQ_FIRST, /*!< the first sequence call CSE command.*/
    CSE_CALL_SEQ_SUBSEQUENT /*!< the second or more sequence call CSE command.*/
} Cse_CallSequence;

/*!
 * @brief CSE commands which follow the same values as the SHE command definition.
 */
typedef enum
{
    CSE_CMD_ENC_ECB = 0x1U, /*!< encrypt ecb cmd.*/
    CSE_CMD_ENC_CBC, /*!< encrypt cbc cmd.*/
    CSE_CMD_DEC_ECB, /*!< decrypt ecb cmd.*/
    CSE_CMD_DEC_CBC, /*!< decrypt cbc cmd.*/
    CSE_CMD_GENERATE_MAC, /*!< generate mac cmd.*/
    CSE_CMD_VERIFY_MAC, /*!< verify mac cmd.*/
    CSE_CMD_LOAD_KEY, /*!< load user key cmd.*/
    CSE_CMD_LOAD_PLAIN_KEY, /*!< load ram key cmd.*/
    CSE_CMD_EXPORT_RAM_KEY, /*!< export ram key cmd.*/
    CSE_CMD_INIT_RNG, /*!< init random number generator cmd.*/
    /*PRQA S 3205 ++ #AMCE_HSM_3205*/
    CSE_CMD_EXTEND_SEED, /*!< extend seed for rng cmd.*/
    /*PRQA S 3205 -- #AMCE_HSM_3205*/
    CSE_CMD_RND, /*!< get random number cmd.*/
    CSE_CMD_RESERVED_1, /*!< reserved.*/ //PRQA S 3205 # allow unused identifiers to be reserved.*/
    CSE_CMD_BOOT_FAILURE, /*!< secure boot failure cmd.*/
    CSE_CMD_BOOT_OK, /*!< secure boot pass cmd.*/
    CSE_CMD_GET_ID, /*!< get uid cmd.*/
    CSE_CMD_BOOT_DEFINE, /*!< secure boot mode cmd.*/
    CSE_CMD_DBG_CHAL, /*!< get debug challenge buffer cmd.*/
    CSE_CMD_DBG_AUTH, /*!< debug authorization flow cmd.*/
    CSE_CMD_CANCEL, /*!< cancel cmd.*/ //PRQA S 3205 # allow unused identifiers to be reserved.*/
    CSE_CMD_MP_COMPRESS, /*!< MPCompress cmd.*/
    CSE_CMD_RESERVED_2 /*!< reserved.*/ //PRQA S 3205 # allow unused identifiers to be reserved.*/
} Cse_CmdType;
/*!
 * @brief Internal driver state information.
 */
typedef struct
{
    boolean CmdInProgress; /*!< Specifies if a command is in progress */
    Cse_CmdType Cmd; /*!< Specifies the type of the command in execution */
    const uint8 *InputBuff; /*!< Specifies the inputbuffer of the command in execution */
    uint8 *OutputBuff; /*!< Specifies the outputbuffer of the command in execution */
    uint32 Index; /*!< Specifies the index in the input buffer of the command in execution */
    uint32 InputBufferSize; /*!< Specifies the size of the inputbuffer size of the command in execution */
    uint32 PartSize; /*!< Specifies the size of the chunck of the input currently processed */
    Cse_KeyId KeyId; /*!< Specifies the key used for the command in execution */
    Hal_StatusType ErrCode; /*!< Specifies the error code of the last executed command */
    const uint8
    *Iv; /*!< Specifies the IV of the command in execution (for encryption/decryption using CBC mode) */
    Cse_CallSequence Seq; /*!< Specifies if the information is the first or a following function call. */
    uint32 MsgLen; /*!< Specifies the message size (in bits) for the command in execution (for MAC generation/verification) */
    boolean *VerifStatus; /*!< Specifies the result of the last executed MAC verification command */
    boolean MacWritten; /*!< Specifies if the MAC to be verified was written in CSE_PRAM for a MAC verification command */
    const uint8 *Mac; /*!< Specifies the MAC to be verified for a MAC verification command */
    uint32 MacLen; /*!< Specifies the number of bits of the MAC to be verified for a MAC verification command */
    Hal_CallbackType Cse_IsrCallback; /*!< The callback invoked when an asynchronous command is completed */
    void *CallbackArgs; /*!< User parameter for the command completion callback */
} Cse_HalInfoType;

/* =========================================== LOCAL VARIABLES ============================================== */
/* Pointer to runtime state structure */
static Cse_HalInfoType Cse_RunningInfo =
{
    .CmdInProgress = TRUE,
};

/* =====================================  FUNCTION PROTOTYPES  ==================================== */
static Hal_StatusType CSE_Hal_InitRNG(void);
static Hal_StatusType CSE_Hal_GetRndChallenge(uint8 *Challenge);
static Hal_StatusType CSE_Hal_SetResetKeyAuth(const uint8 *Auth);
static Hal_StatusType CSE_Hal_EncryptECBAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId);
static Hal_StatusType CSE_Hal_DecryptECBAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId);
static Hal_StatusType CSE_Hal_EncryptCBCAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId);
static Hal_StatusType CSE_Hal_DecryptCBCAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId);
static Hal_StatusType CSE_Hal_GenerateMACAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId);
static Hal_StatusType CSE_Hal_VerifyMACAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId);
static void CSE_Hal_InitCommand(Cse_KeyId KeyId, Cse_CmdType Cmd, const uint8 *InBuff, uint8 *OutBuff, uint32 Length);
static void CSE_Hal_StartEncDecECBCmd(void);
static boolean CSE_Hal_ContinueEncDecECBCmd(void);
static void CSE_Hal_StartEncDecCBCCmd(void);
static boolean CSE_Hal_ContinueEncDecCBCCmd(void);
static void CSE_Hal_StartGenMACCmd(void);
static boolean CSE_Hal_ContinueGenMACCmd(void);
static void CSE_Hal_StartVerifyMACCmd(void);
static boolean CSE_Hal_ContinueVerifyMACCmd(void);
static Hal_StatusType CSE_Hal_KDF(const uint8 *AuthKey, const uint8 *Constant, uint8 *K_out);
static Hal_StatusType CSE_Hal_CalcResetKeyAuth(const uint8 *MasterEcuKey, const uint8 *Challenge, uint8 *AuthOut);
static uint32 CSE_Hal_RoundTo(uint32 Value, uint32 RoundTo);
static void MemSet(uint8 *buffer, uint8 data, uint32 size);
ISR(CSE_IRQHandler);
/* =====================================  Functions definition  ===================================== */
/*!
 * @brief Initialize CSE module
 * @note  Function ID:  DES_CSE_API_001
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_Init(void)
{
    Hal_StatusType Res;
    /*init CSE Status is free*/
    Cse_RunningInfo.CmdInProgress = FALSE;
    Res = CSE_Hal_InitRNG();
    /*enable NVIC CSEIRQ for asycn function*/
    CSE_Reg_SetInterrupt(FALSE);
    CSE_Reg_ClearCmdCompleteStatus();
    Core_Hal_EnableIrq(CSE_IRQn);

    return Res;
}

/*!
 * @brief Uninitialize CSE module
 * @note  Function ID:  DES_CSE_API_002
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
 #ifndef CSE_SDK_NON_EXTENDED_API
Hal_StatusType CSE_Hal_Deinit(void)
{
    Hal_StatusType Res = STATUS_SUCCESS;
    /*init CSE Status is Busy , not deal with cmd*/
    Cse_RunningInfo.CmdInProgress = TRUE;

    /*disable NVIC CSEIRQ for asycn function*/
    Core_Hal_DisableIrq(CSE_IRQn);

    return Res;
}
#endif
/*!
 * @brief Performs the AES-128 encryption in ECB mode of the input plain text with the Key ID and returns the cipher text.
 * @note  Function ID:  DES_CSE_API_003
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_EncryptECB(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat;
    boolean TimeOutFlag = FALSE;
    FlashDeviceType DevLock;

    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*timeout value is not 0, will execute Sync flow*/
        if (0UL != TimeoutUs)
        {
            /*Idle will going*/
            if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
            {
                /*init Cse_RunningInfo and start execute request*/
                CSE_Hal_InitCommand(KeyId, CSE_CMD_ENC_ECB, Ptr->InputPtr, Ptr->OutputPtr, Ptr->InputLength);
                CSE_Hal_StartEncDecECBCmd();

                /*wait request completion*/
                while (Cse_RunningInfo.CmdInProgress == TRUE)
                {
                    /* Wait until the execution of the command is complete */
                    Stat = CSE_Reg_WaitCommandCompletion(TimeoutUs);
                    if (Stat == STATUS_TIMEOUT)/*if timeout, return timeout error code*/
                    {
                        TimeOutFlag = TRUE;
                        Cse_RunningInfo.CmdInProgress = FALSE;
                        break;
                    }
                    Cse_RunningInfo.CmdInProgress = CSE_Hal_ContinueEncDecECBCmd();
                }
                if (TimeOutFlag == FALSE)/*if not timeout, read errcode*/
                {
                    Stat = Cse_RunningInfo.ErrCode;
                }
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
            (void)System_FlsDeviceUnlock(CSE_DEV);
        }
        else /*will execute Async flow*/
        {
            Stat = CSE_Hal_EncryptECBAsync(Ptr, KeyId);
        }
    }
    else
    {
        Stat =  STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Performs the AES-128 decryption in ECB mode of the input cipher text with the Key ID and returns the plain text.
 * @note  Function ID:  DES_CSE_API_004
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_DecryptECB(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs)

{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat;
    boolean TimeOutFlag = FALSE;
    FlashDeviceType DevLock;

    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*timeout value is not 0, will execute Sync flow*/
        if (0UL != TimeoutUs)
        {
            /*Idle will going*/
            if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
            {
                /*init Cse_RunningInfo and start execute request*/
                CSE_Hal_InitCommand(KeyId, CSE_CMD_DEC_ECB, Ptr->InputPtr, Ptr->OutputPtr, Ptr->InputLength);
                CSE_Hal_StartEncDecECBCmd();

                /*wait request completion*/
                while (Cse_RunningInfo.CmdInProgress == TRUE)
                {
                    /* Wait until the execution of the command is complete */
                    Stat = CSE_Reg_WaitCommandCompletion(TimeoutUs);
                    if (Stat == STATUS_TIMEOUT)/*if timeout, return timeout error code*/
                    {
                        TimeOutFlag = TRUE;
                        Cse_RunningInfo.CmdInProgress = FALSE;
                        break;
                    }
                    Cse_RunningInfo.CmdInProgress = CSE_Hal_ContinueEncDecECBCmd();
                }
                if (TimeOutFlag == FALSE)/*if not timeout, read errcode*/
                {
                    Stat = Cse_RunningInfo.ErrCode;
                }
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
            (void)System_FlsDeviceUnlock(CSE_DEV);
        }
        else /*will execute Async flow*/
        {
            Stat = CSE_Hal_DecryptECBAsync(Ptr, KeyId);
        }
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Performs the AES-128 encryption in CBC mode of the input plain text with the Key ID and returns the cipher text.
 * @note  Function ID:  DES_CSE_API_005
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_EncryptCBC(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat;
    boolean TimeOutFlag = FALSE;
    FlashDeviceType DevLock;

    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*timeout value is not 0, will execute Sync flow*/
        if (0UL != TimeoutUs)
        {
            /*Idle will going*/
            if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
            {
                /*init Cse_RunningInfo and start execute request*/
                CSE_Hal_InitCommand(KeyId, CSE_CMD_ENC_CBC, Ptr->InputPtr, Ptr->OutputPtr, Ptr->InputLength);

                Cse_RunningInfo.Iv = Ptr->Iv;

                CSE_Hal_StartEncDecCBCCmd();

                /*wait request completion*/
                while (Cse_RunningInfo.CmdInProgress == TRUE)
                {
                    /* Wait until the execution of the command is complete */
                    Stat = CSE_Reg_WaitCommandCompletion(TimeoutUs);
                    if (Stat == STATUS_TIMEOUT)/*if timeout, return timeout error code*/
                    {
                        TimeOutFlag = TRUE;
                        Cse_RunningInfo.CmdInProgress = FALSE;
                        break;
                    }
                    Cse_RunningInfo.CmdInProgress = CSE_Hal_ContinueEncDecCBCCmd();
                }

                if (TimeOutFlag == FALSE)/*if not timeout, read errcode*/
                {
                    Stat = Cse_RunningInfo.ErrCode;
                }
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
            (void)System_FlsDeviceUnlock(CSE_DEV);
        }
        else /*will execute Async flow*/
        {
            Stat = CSE_Hal_EncryptCBCAsync(Ptr, KeyId);
        }
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Performs the AES-128 decryption in CBC mode of the input cipher text with the Key ID and returns the plain text.
 * @note  Function ID:  DES_CSE_API_006
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_DecryptCBC(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat;
    boolean TimeOutFlag = FALSE;
    FlashDeviceType DevLock;

    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*timeout value is not 0, will execute Sync flow*/
        if (0UL != TimeoutUs)
        {
            /*Idle will going*/
            if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
            {
                /*init Cse_RunningInfo and start execute request*/
                CSE_Hal_InitCommand(KeyId, CSE_CMD_DEC_CBC, Ptr->InputPtr, Ptr->OutputPtr, Ptr->InputLength);

                Cse_RunningInfo.Iv = Ptr->Iv;

                CSE_Hal_StartEncDecCBCCmd();

                /*wait request completion*/
                while (Cse_RunningInfo.CmdInProgress == TRUE)
                {
                    /* Wait until the execution of the command is complete */
                    Stat = CSE_Reg_WaitCommandCompletion(TimeoutUs);
                    if (Stat == STATUS_TIMEOUT)/*if timeout, return timeout error code*/
                    {
                        TimeOutFlag = TRUE;
                        Cse_RunningInfo.CmdInProgress = FALSE;
                        break;
                    }
                    Cse_RunningInfo.CmdInProgress = CSE_Hal_ContinueEncDecCBCCmd();
                }
                if (TimeOutFlag == FALSE)/*if not timeout, read errcode*/
                {
                    Stat = Cse_RunningInfo.ErrCode;
                }
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
            (void)System_FlsDeviceUnlock(CSE_DEV);
        }
        else /*will execute Async flow*/
        {
            Stat = CSE_Hal_DecryptCBCAsync(Ptr, KeyId);
        }
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Generate the MAC of a given message using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_007
 * @param [in] Ptr: a structure contatining input and output. eg.message/cmac.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_GenerateMAC(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat;
    boolean TimeOutFlag = FALSE;
    uint32 MsgLenTemp = 0u;
    FlashDeviceType DevLock;

    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*timeout value is not 0, will execute Sync flow*/
        if (0UL != TimeoutUs)
        {
            /*Idle will going*/
            if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
            {
                MsgLenTemp = Ptr->InputLength << CSE_BYTES_TO_FROM_BITS_SHIFT;

                /*init Cse_RunningInfo and start execute request, MsgLenTemp will div 8 from bit to bytes*/
                CSE_Hal_InitCommand(KeyId, CSE_CMD_GENERATE_MAC, Ptr->InputPtr, Ptr->OutputPtr, \
                                    CSE_Hal_RoundTo(MsgLenTemp, 0x8U) >> CSE_BYTES_TO_FROM_BITS_SHIFT);
                Cse_RunningInfo.MsgLen = MsgLenTemp;

                CSE_Hal_StartGenMACCmd();

                /*wait request completion*/
                while (Cse_RunningInfo.CmdInProgress == TRUE)
                {
                    /* Wait until the execution of the command is complete */
                    Stat = CSE_Reg_WaitCommandCompletion(TimeoutUs);
                    if (STATUS_TIMEOUT == Stat)/*if timeout, return timeout error code*/
                    {
                        TimeOutFlag = TRUE;
                        Cse_RunningInfo.CmdInProgress = FALSE;
                        break;
                    }
                    /*continue Gen Mac and update Cse Stat*/
                    Cse_RunningInfo.CmdInProgress = CSE_Hal_ContinueGenMACCmd();
                }
                /*not Timeout will read ErrCode*/
                if (TimeOutFlag == FALSE)
                {
                    Stat = Cse_RunningInfo.ErrCode;
                }
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
            (void)System_FlsDeviceUnlock(CSE_DEV);
        }
        else /*will execute Async flow*/
        {
            Stat = CSE_Hal_GenerateMACAsync(Ptr, KeyId);
        }
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Verifies the MAC of a given message using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_008
 * @param [in] Ptr: a structure contatining input and output. eg.message/cmac.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_VerifyMAC(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat;
    boolean TimeOutFlag = FALSE;
    FlashDeviceType DevLock;

    uint32 MsgLenTemp = 0u;
    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*timeout value is not 0, will execute Sync flow*/
        if (0UL != TimeoutUs)
        {
            /*Idle will going*/
            if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
            {
                MsgLenTemp = Ptr->InputLength << CSE_BYTES_TO_FROM_BITS_SHIFT;

                /*init Cse_RunningInfo and start execute request*/
                CSE_Hal_InitCommand(KeyId, CSE_CMD_VERIFY_MAC, Ptr->InputPtr, NULL_PTR, \
                                    CSE_Hal_RoundTo(MsgLenTemp, 0x8U) >> CSE_BYTES_TO_FROM_BITS_SHIFT);
                Cse_RunningInfo.MsgLen = MsgLenTemp;
                Cse_RunningInfo.VerifStatus = Ptr->VerifyStatus;
                Cse_RunningInfo.MacWritten = FALSE;
                Cse_RunningInfo.Mac = Ptr->SecondaryInputPtr;
                Cse_RunningInfo.MacLen = Ptr->SecondaryInputLength << CSE_BYTES_TO_FROM_BITS_SHIFT;
                CSE_Hal_StartVerifyMACCmd();

                /*wait request completion*/
                while (Cse_RunningInfo.CmdInProgress == TRUE)
                {
                    /* Wait until the execution of the command is complete */
                    Stat = CSE_Reg_WaitCommandCompletion(TimeoutUs);
                    if (STATUS_TIMEOUT == Stat)/*if timeout, return timeout error code*/
                    {
                        TimeOutFlag = TRUE;
                        Cse_RunningInfo.CmdInProgress = FALSE;
                        break;
                    }
                    Cse_RunningInfo.CmdInProgress = CSE_Hal_ContinueVerifyMACCmd();
                }
                if (TimeOutFlag == FALSE)/*if not timeout, read error code*/
                {
                    Stat = Cse_RunningInfo.ErrCode;
                }
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
            (void)System_FlsDeviceUnlock(CSE_DEV);
        }
        else
        {
            Stat = CSE_Hal_VerifyMACAsync(Ptr, KeyId);
        }
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Exports the RAM_KEY into a format protected by SECRET_KEY.
 * @note  Function ID:  DES_CSE_API_010
 * @param [out] Ptr: store M1~M5 value.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_GetRamKey(const Cse_SheKeyInfoType *Ptr)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat;
    FlashDeviceType DevLock;

    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*Cse Module free will going*/
        if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
        {
            /*set Cse module is busy*/
            Cse_RunningInfo.CmdInProgress = TRUE;

            /* Write the command header. This will trigger the command execution */
            CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_EXPORT_RAM_KEY, (uint8)CSE_FUNC_FORMAT_COPY, \
                                       (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_RAM_KEY);
            Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
            if (STATUS_SUCCESS == Stat)
            {
                Stat = CSE_Reg_ReadErrorBits();
                if (STATUS_SUCCESS == Stat)
                {
                    /* Read the M1-M5 values associated with the key */
                    CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, Ptr->M1, CSE_M1_SIZE_IN_BYTES);
                    CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_2_OFFSET, Ptr->M2, CSE_M2_SIZE_IN_BYTES);
                    CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_4_OFFSET, Ptr->M3, CSE_M3_SIZE_IN_BYTES);
                    CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_5_OFFSET, Ptr->M4, CSE_M4_SIZE_IN_BYTES);
                    CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_7_OFFSET, Ptr->M5, CSE_M5_SIZE_IN_BYTES);
                }
            }
            /*free Cse module*/
            Cse_RunningInfo.CmdInProgress = FALSE;

        }
        else /*check CSE Module Status, Busy will return*/
        {
            Stat = STATUS_BUSY;
        }
        (void)System_FlsDeviceUnlock(CSE_DEV);
    }
    else
    {
        Stat =  STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Extends the seed of the PRNG.
 * @note  Function ID:  DES_CSE_API_011
 * @param [out] Rnd: pointer to a 128-bit buffer containing the PRNG value
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_GenerateRnd(uint8 *Rnd)
{
    DEVICE_ASSERT(NULL_PTR != Rnd);

    Hal_StatusType Stat;
    FlashDeviceType DevLock;

    if (Rnd != NULL_PTR)
    {
        /*Cse Module free will going*/
        DevLock = System_FlsDeviceTryLock(CSE_DEV);
        if (DevLock == CSE_DEV)
        {
            /*Cse Module free will going*/
            if (Cse_RunningInfo.CmdInProgress == FALSE)
            {
                /*set Cse module is busy*/
                Cse_RunningInfo.CmdInProgress = TRUE;

                /* Write the command header. This will trigger the command execution */
                CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_RND, (uint8)CSE_FUNC_FORMAT_COPY, \
                                           (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_SECRET_KEY);
                Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
                if (STATUS_SUCCESS == Stat)
                {
                    Stat = CSE_Reg_ReadErrorBits();
                    /* Read the resulted random bytes */
                    if (STATUS_SUCCESS == Stat)
                    {
                        CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, Rnd, CSE_PAGE_SIZE_IN_BYTES);
                    }
                }

                /*free Cse module*/
                Cse_RunningInfo.CmdInProgress = FALSE;

            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
            (void)System_FlsDeviceUnlock(CSE_DEV);
        }
        else
        {
            Stat = STATUS_ERROR;
        }
    }
    else
    {
        Stat = STATUS_ERROR;
    }

    return Stat;
}

/*!
 * @brief Generate the MAC of a given message(locate in FLASH) using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_012
 * @param [in] Ptr: a structure contatining input and output, Message and Addr must be 16bytes align. eg.message/cmac.
 * @param [in] KeyId: the index of the key used in cse command.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_GenerateMACAddrMode(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId)
{
    Hal_StatusType Stat;
    uint32 MsgLenTemp = 0U;
    FlashDeviceType DevLock;
    if (Ptr != NULL_PTR)
    {
        /*Cse Module free will going*/
        DevLock = System_FlsDeviceTryLock(CSE_DEV);
        if (DevLock == CSE_DEV)
        {
            /*Cse Module free will going*/
            if ((Cse_RunningInfo.CmdInProgress == FALSE))
            {
                /*set cse module status is busy*/
                Cse_RunningInfo.CmdInProgress = TRUE;
                MsgLenTemp = Ptr->InputLength << CSE_BYTES_TO_FROM_BITS_SHIFT;

                /*set GenMAC Cmd and start execute request*/
                /*PRQA S 0310 ++ # make sure there are no alignment issues.*/
                CSE_Reg_WriteCommandWords(FEATURE_CSE_FLASH_START_ADDRESS_OFFSET,
                        (const uint32 *) & (Ptr->InputPtr), 1U);
                /*PRQA S 0310 -- # make sure there are no alignment issues.*/
                CSE_Reg_WriteCommandWords(FEATURE_CSE_MESSAGE_LENGTH_OFFSET, &MsgLenTemp, 1U);
                Stat = CSE_Reg_WriteCmdAndWait((uint8)CSE_CMD_GENERATE_MAC, (uint8)CSE_FUNC_FORMAT_ADDR, \
                                               (uint8)CSE_CALL_SEQ_FIRST, (uint8)KeyId);
                if (STATUS_SUCCESS == Stat)
                {
                    /*Read Cmd execute status*/
                    Stat = CSE_Reg_ReadErrorBits();
                    if (STATUS_SUCCESS == Stat)
                    {
                        /* Read the resulted MAC */
                        CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_2_OFFSET, Ptr->OutputPtr, CSE_PAGE_SIZE_IN_BYTES);
                    }
                }

                Cse_RunningInfo.CmdInProgress = FALSE;
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
            (void)System_FlsDeviceUnlock(CSE_DEV);
        }
        else
        {
            Stat = STATUS_ERROR;
        }
    }
    else
    {
        Stat = STATUS_ERROR;
    }

    return Stat;
}

/*!
 * @brief Verifies the MAC of a given message(locate in FLASH) using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_013
 * @param [in] Ptr: a structure contatining input and output. eg.message/cmac.
 * @param [in] KeyId: the index of the key used in cse command.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
#ifndef CSE_SDK_NON_EXTENDED_API
Hal_StatusType CSE_Hal_VerifyMACAddrMode(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId)
{
    Hal_StatusType Stat;
    uint32 MsgLenTemp = 0U;
    uint32 MacLenTemp = 0U;
    FlashDeviceType DevLock;

    if (Ptr != NULL_PTR)
    {
        DevLock = System_FlsDeviceTryLock(CSE_DEV);
        if (DevLock == CSE_DEV)
        {
            /*Cse Module free will going*/
            if (Cse_RunningInfo.CmdInProgress == FALSE)
            {
                MsgLenTemp = Ptr->InputLength << CSE_BYTES_TO_FROM_BITS_SHIFT;
                MacLenTemp = Ptr->SecondaryInputLength << CSE_BYTES_TO_FROM_BITS_SHIFT;
                /*set Cse module is busy*/
                Cse_RunningInfo.CmdInProgress = TRUE;

                /* Write the flash start address to PRAM Page1 Word0 */
                /*PRQA S 0310 ++ # make sure there are no alignment issues.*/
                CSE_Reg_WriteCommandWords(FEATURE_CSE_FLASH_START_ADDRESS_OFFSET, (const uint32 *)&Ptr->InputPtr, 1U);
                /*PRQA S 0310 -- # make sure there are no alignment issues.*/
                /* Write the MAC Value to PRAM Page2 */
                CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_2_OFFSET, Ptr->SecondaryInputPtr, CSE_PAGE_SIZE_IN_BYTES);
                /* Write the Msg Length to PRAM Page0 Word3*/
                CSE_Reg_WriteCommandWords(FEATURE_CSE_MESSAGE_LENGTH_OFFSET, &MsgLenTemp, 1U);
                /* Write Mac Len to PRAM Page0 Word2 low 16bit */
                CSE_Reg_WriteCommandHalfWord(FEATURE_CSE_MAC_LENGTH_OFFSET, (uint16)MacLenTemp);
                /* Write the command header. This will trigger the command execution */
                Stat = CSE_Reg_WriteCmdAndWait((uint8)CSE_CMD_VERIFY_MAC, (uint8)CSE_FUNC_FORMAT_ADDR, \
                                               (uint8)CSE_CALL_SEQ_FIRST, (uint8)KeyId);
                if (STATUS_SUCCESS == Stat)
                {
                    Stat = CSE_Reg_ReadErrorBits();
                    if (STATUS_SUCCESS == Stat)
                    {
                        /* Read the result of the MAC verification */
                        *(Ptr->VerifyStatus) =
                        (CSE_Reg_ReadCommandHalfWord(FEATURE_CSE_VERIFICATION_STATUS_OFFSET) == 0U)?1u:0u;
                    }
                }
                Cse_RunningInfo.CmdInProgress = FALSE;
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
        }
        else /*check CSE Module Status, Busy will return*/
        {
            Stat = STATUS_BUSY;
        }
        (void)System_FlsDeviceUnlock(CSE_DEV);
    }
    else
    {
        Stat = STATUS_ERROR;
    }

    return Stat;
}

/*!
 * @brief Extends the seed of the PRNG.
 * @note  Function ID:  DES_CSE_API_014
 * @param [in] Entropy: pointer to a 128-bit buffer containing the entropy
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_ExtendRNGSeed(const uint8 *Entropy)
{
    DEVICE_ASSERT(NULL_PTR != Entropy);

    Hal_StatusType Stat;
    FlashDeviceType DevLock;
    if (Entropy != NULL_PTR)
    {
        DevLock = System_FlsDeviceTryLock(CSE_DEV);
        if (DevLock == CSE_DEV)
        {
            /*Cse Module free will going*/
            if (Cse_RunningInfo.CmdInProgress == FALSE)
            {
                /*set Cse module is busy*/
                Cse_RunningInfo.CmdInProgress = TRUE;

                /* Write the bytes of the Entropy */
                CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, Entropy, CSE_PAGE_SIZE_IN_BYTES);
                /* Write the command header. This will trigger the command execution */
                CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_EXTEND_SEED, (uint8)CSE_FUNC_FORMAT_COPY, \
                                           (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_SECRET_KEY);
                Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
                if (STATUS_SUCCESS == Stat)
                {
                    Stat = CSE_Reg_ReadErrorBits();
                }

                /*free Cse module*/
                Cse_RunningInfo.CmdInProgress = FALSE;
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
        }
        else
        {
            Stat = STATUS_BUSY;
        }
        (void)System_FlsDeviceUnlock(CSE_DEV);
    }
    else
    {
        Stat = STATUS_ERROR;
    }

    return Stat;
}
#endif

/*!
 * @brief Get the UID from CSE module.
 * @note  Function ID:  DES_CSE_API_015
 * @param [in] Challenge: pointer to the 128-bit buffer containing challenge data
 * @param [out] Uid: pointer to 120 bit buffer containing UID data
 * @param [out] Sreg: Value of the status register.
 * @param [out] Mac: pointer to the 128 bit buffer containing MAC data
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_GetID(const uint8 *Challenge, uint8 *Uid, uint8 *Sreg, uint8 *Mac)
{
    DEVICE_ASSERT(NULL_PTR != Challenge);
    DEVICE_ASSERT(NULL_PTR != Uid);
    DEVICE_ASSERT(NULL_PTR != Sreg);
    DEVICE_ASSERT(NULL_PTR != Mac);

    Hal_StatusType Stat;

    FlashDeviceType DevLock;
    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        if ((Challenge != NULL_PTR) && (Uid != NULL_PTR) && (Sreg != NULL_PTR) && (Mac != NULL_PTR))
        {
            /*Cse Module free will going*/
            if (Cse_RunningInfo.CmdInProgress == FALSE)
            {
                /*set Cse module is busy*/
                Cse_RunningInfo.CmdInProgress = TRUE;

                /* Write the challenge */
                CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, Challenge, CSE_PAGE_SIZE_IN_BYTES);
                /* Write the command header. This will trigger the command execution */
                CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_GET_ID, (uint8)CSE_FUNC_FORMAT_COPY, \
                                           (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_SECRET_KEY);
                Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
                if (STATUS_SUCCESS == Stat)
                {
                    Stat = CSE_Reg_ReadErrorBits();
                    if (STATUS_SUCCESS == Stat)
                    {
                        /* Read the UID */
                        CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_2_OFFSET, Uid, (uint8)(CSE_PAGE_SIZE_IN_BYTES));
                        /* Read the value of the SREG register */
                        *Sreg = CSE_Reg_ReadCommandByte(FEATURE_CSE_SREG_OFFSET);
                        /* Read the MAC over the UID and the SREG */
                        CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_3_OFFSET, Mac, CSE_PAGE_SIZE_IN_BYTES);
                    }
                }
                /*free Cse module*/
                Cse_RunningInfo.CmdInProgress = FALSE;
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
            (void)System_FlsDeviceUnlock(CSE_DEV);
        }
        else
        {
            Stat = STATUS_ERROR;
        }
    }
    else
    {
        Stat = STATUS_ERROR;
    }

    return Stat;
}

/*!
 * @brief Compresses the given messages.
 * @note  Function ID:  DES_CSE_API_016
 * @param [in] Msg: pointer to the messages which will be compressed
 * @param [in] MsgLen: number in bits of message
 * @param [out] MpCompress: pointer to the 128 bit buffer storing the compressed data
 * @param [in] TimeoutUs: timeout value in milliseconds
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_MPCompress(const uint8 *Msg, uint16 MsgLen, uint8 *MpCompress, uint32 TimeoutUs)
{
    DEVICE_ASSERT(NULL_PTR != Msg);
    DEVICE_ASSERT(NULL_PTR != MpCompress);

    Cse_CallSequence Seq = CSE_CALL_SEQ_FIRST;
    Hal_StatusType Stat = STATUS_SUCCESS;
    uint32 Index = 0U;
    uint16 NumPagesLeft = MsgLen;
    FlashDeviceType DevLock;
    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*Cse Module free will going*/
        if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Msg) && (NULL_PTR != MpCompress))
        {
            /* Loop and launch commands until the end of the message */
            while (NumPagesLeft > 0U)
            {
                /*PRAM each page is 16 bytes, NumPagesLeft for need PRAM page number*/
                uint8 numPages = (uint8)((NumPagesLeft > CSE_DATA_PAGES_AVAILABLE) ? \
                                         CSE_DATA_PAGES_AVAILABLE : NumPagesLeft);

                /* Bytes equal NumPages*16 */
                uint8 numBytes = (uint8)(numPages << CSE_BYTES_TO_FROM_PAGES_SHIFT);

                /*write the  data to PRAM(1-7)*/
                CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, &Msg[Index], numBytes);

                /*write Numpages to Pram page0 <Word3 High 16bit> for command specific, data takes up pages number of Cse memory*/
                CSE_Reg_WriteCommandHalfWord(FEATURE_CSE_PAGE_LENGTH_OFFSET, MsgLen);

                /* Write the command header. This will trigger the command execution */
                CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_MP_COMPRESS, (uint8)CSE_FUNC_FORMAT_COPY, \
                                           (uint8)Seq, (uint8)CSE_SECRET_KEY);

                /* Wait until the execution of the command is complete */
                Stat = CSE_Reg_WaitCommandCompletion(TimeoutUs);
                if (STATUS_SUCCESS == Stat)
                {
                    Stat = CSE_Reg_ReadErrorBits();
                    if (STATUS_SUCCESS == Stat)
                    {
                        /*uppdate NumPagesLeft and msg Index*/
                        NumPagesLeft = (uint16)(NumPagesLeft - numPages);
                        Index = (uint32)(Index + numBytes);

                        if (Seq == CSE_CALL_SEQ_FIRST)
                        {
                            Seq = CSE_CALL_SEQ_SUBSEQUENT;
                        }
                    }
                }

                if (Stat != STATUS_SUCCESS)
                {
                    break;
                }
            }

            /* Read the result of the compression */
            if (STATUS_SUCCESS == Stat)
            {
                CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, MpCompress, CSE_PAGE_SIZE_IN_BYTES);
            }
        }
        else /*check CSE Module Status, Busy will return*/
        {
            Stat = STATUS_BUSY;
        }
        (void)System_FlsDeviceUnlock(CSE_DEV);
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Mark failure/success boot verification.
 * @note  Function ID:  DES_CSE_API_017
 * @param [in] Status: secure boot flow status
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_SetSecureBootStatus(Cse_BootStatus Status)
{
    Hal_StatusType Stat;
    FlashDeviceType DevLock;
    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*Cse is free , will execute cmd*/
        if (Cse_RunningInfo.CmdInProgress == FALSE)
        {
            Cse_RunningInfo.CmdInProgress = TRUE;
            /* Write the command header. This will trigger the command execution */
            if (Status == CSE_BOOT_OK)
            {
                CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_BOOT_OK, (uint8)CSE_FUNC_FORMAT_COPY, \
                                           (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_SECRET_KEY);
            }
            else
            {
                CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_BOOT_FAILURE, (uint8)CSE_FUNC_FORMAT_COPY, \
                                           (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_SECRET_KEY);
            }
            Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
            if (STATUS_SUCCESS == Stat)
            {
                Stat = CSE_Reg_ReadErrorBits();
            }

            Cse_RunningInfo.CmdInProgress = FALSE;
        }
        else /*Check CSE Module Status, Busy will return*/
        {
            Stat = STATUS_BUSY;
        }
        (void)System_FlsDeviceUnlock(CSE_DEV);
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;

}

/*!
 * @brief Define bootcode size and secure boot type.
 * @note  Function ID:  DES_CSE_API_018
 * @param [in] BootSize: number of blocks of 128-bit data to check on boot
 * @param [in] BootMode: secure boot type
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_SetSecureBootMode(uint32 BootSize, Cse_BootMode BootMode)
{
    uint8 Mode = (uint8)BootMode;
    Hal_StatusType Stat;
    uint32 BootSizeBits = BootSize << CSE_BYTES_TO_FROM_BITS_SHIFT;

    uint32 PSize = CSE_Reg_GetPFlashSize();
    FlashDeviceType DevLock;
    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        if(BootSize <= PSize)
        {
            /*Check CSE Module Status, Busy will return*/
            if (Cse_RunningInfo.CmdInProgress == FALSE)
            {
                Cse_RunningInfo.CmdInProgress = TRUE;
                CSE_Reg_WriteCommandWords(FEATURE_CSE_BOOT_SIZE_OFFSET, &BootSizeBits, 1U);
#if defined(AC7840X)
                CSE_Reg_WriteCommandByte(FEATURE_CSE_BOOT_FLAVOR_OFFSET, Mode);
#elif defined(AC7842X)
                CSE_Reg_WriteBootDefinedByte(FEATURE_CSE_BOOT_FLAVOR_OFFSET, Mode);
#endif
                /* Write the command header. This will trigger the command execution */
                CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_BOOT_DEFINE, (uint8)CSE_FUNC_FORMAT_COPY, \
                                           (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_SECRET_KEY);
                Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
                if (STATUS_SUCCESS == Stat)
                {
                    Stat = CSE_Reg_ReadErrorBits();
                }

                Cse_RunningInfo.CmdInProgress = FALSE;
            }
            else /*Cse is free , will execute cmd*/
            {
                Stat = STATUS_BUSY;
            }
        }
        else
        {
            Stat = STATUS_ERROR;
        }
        (void)System_FlsDeviceUnlock(CSE_DEV);
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Cancels a previously launched asynchronous command
 * @note  Function ID:  DES_CSE_API_019
 * @return none
 */
void CSE_Hal_CancelCommand(void)
{
    if (Cse_RunningInfo.CmdInProgress == TRUE)
    {
        (void)CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);

        if ((CSE_CMD_ENC_ECB != Cse_RunningInfo.Cmd) && (CSE_CMD_DEC_ECB != Cse_RunningInfo.Cmd))
        {
            /* Was there any command already launched? If so, break the sequence */
            if (Cse_RunningInfo.InputBufferSize != Cse_RunningInfo.PartSize)
            {
                /* Write the command header. CallSeq is set to 0 in order to trigger a command
                 * that will generate a sequence error, breaking the chain of calls */
                CSE_Reg_WriteCommandHeader((uint8)Cse_RunningInfo.Cmd, (uint8)CSE_FUNC_FORMAT_COPY, \
                                           (uint8)CSE_CALL_SEQ_FIRST, (uint8)Cse_RunningInfo.KeyId);

                /* Wait until the execution of the command is complete */
                (void)CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
            }
        }
        /*Free Cse module*/
        Cse_RunningInfo.CmdInProgress = FALSE;
    }
}

/*!
 * @brief Installs a callback function for CSE driver.
 * @note  Function ID:  DES_CSE_API_020
 * @param [in] Func: The function to be invoked
 * @param [in] Args: The parameter to be passed to the callback function
 * @return none
 */
void CSE_Hal_InstallCallback(const Hal_CallbackType Func,
                             void *Args)/*cstat !MISRAC2012-Rule-8.13 Input/output parameter*/
{
    /*Init callback function*/
    Cse_RunningInfo.Cse_IsrCallback = Func;
    /*Init callback paramter*/
    Cse_RunningInfo.CallbackArgs = Args;/*cstat !MISRAC2012-Rule-8.13 Input/output parameter*/
}

/*!
 * @brief Asynchronously get operation status.
 * @note  Function ID:  DES_CSE_API_021
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
#ifndef CSE_SDK_NON_EXTENDED_API
Hal_StatusType CSE_Hal_GetAsyncCmdStatus(void)
{
    Hal_StatusType Stat;

    /*check CSE Module Status, Busy will return*/
    if (Cse_RunningInfo.CmdInProgress == TRUE)
    {
        Stat = STATUS_BUSY;
    }
    else /*Cse Module free will going*/
    {
        Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_SYNC_TIME_ZERO);
        if (STATUS_SUCCESS == Stat)
        {
            Stat = CSE_Reg_ReadErrorBits();
        }
    }

    return Stat;
}
#endif

/*!
 * @brief Erase all keys stored in the CSE.
 * @note  Function ID:  DES_CSE_API_022
 * @param [in] MasterKey: Master Key in chip.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_ResetKey(const uint8 *MasterKey)
{
    DEVICE_ASSERT(NULL_PTR != MasterKey);
    Hal_StatusType Res;
    uint8 Challenge[16U];
    uint8 Auth[16U];
    if (MasterKey != NULL_PTR)
    {
        /*default:Master Key is Ready, Cse Module OK*/
        Res = CSE_Hal_GetRndChallenge(Challenge);
        if (STATUS_SUCCESS == Res)
        {
            Res = CSE_Hal_CalcResetKeyAuth(MasterKey, Challenge, Auth);
            if (STATUS_SUCCESS == Res)
            {
                Res = CSE_Hal_SetResetKeyAuth(Auth);
            }
        }
    }
    else
    {
        Res = STATUS_ERROR;
    }

    return Res;
}

/*!
 * @brief Updates an internal key to CSE module.
 * @note  Function ID:  DES_CSE_API_023
 * @param [in] KeyId: the key to be updated
 * @param [in] InPtr: store M1~M5 value.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_LoadKey(Cse_KeyId KeyId, const Cse_SheKeyInfoType *InPtr)
{
    DEVICE_ASSERT(NULL_PTR != InPtr);

    Hal_StatusType Stat;
    FlashDeviceType DevLock;
    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*Cse Module free will going*/
        if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != InPtr))
        {
            /*set Cse module is busy*/
            Cse_RunningInfo.CmdInProgress = TRUE;

            /* Write the values of M1-M3 to Pram Pages1,2,4*/
            CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, InPtr->M1, CSE_M1_SIZE_IN_BYTES);
            CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_2_OFFSET, InPtr->M2, CSE_M2_SIZE_IN_BYTES);
            CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_4_OFFSET, InPtr->M3, CSE_M3_SIZE_IN_BYTES);
            /* Write the command header. This will trigger the command execution */
            CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_LOAD_KEY, (uint8)CSE_FUNC_FORMAT_COPY, \
                                       (uint8)CSE_CALL_SEQ_FIRST, (uint8)KeyId);

            Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
            if (STATUS_SUCCESS == Stat)
            {
                Stat = CSE_Reg_ReadErrorBits();
                /* Read the obtained M4 and M5 */
                if (STATUS_SUCCESS == Stat)
                {
                    CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_5_OFFSET, InPtr->M4, CSE_M4_SIZE_IN_BYTES);
                    CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_7_OFFSET, InPtr->M5, CSE_M5_SIZE_IN_BYTES);
                }
            }
            /*free Cse module*/
            Cse_RunningInfo.CmdInProgress = FALSE;

        }
        else /*check CSE Module Status, Busy will return*/
        {
            Stat = STATUS_BUSY;
        }
        (void)System_FlsDeviceUnlock(CSE_DEV);
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Updates the RAM key memory slot with a 128-bit plaintext.
 * @note  Function ID:  DES_CSE_API_024
 * @param [in] PlainKey: pointer to the 128-bit plain text
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_LoadPlainKey(const uint8 *PlainKey)
{
    DEVICE_ASSERT(NULL_PTR != PlainKey);

    Hal_StatusType Stat;
    FlashDeviceType DevLock;
    if (PlainKey != NULL_PTR)
    {
        DevLock = System_FlsDeviceTryLock(CSE_DEV);
        if (DevLock == CSE_DEV)
        {
            /*Cse Module free will going*/
            if (Cse_RunningInfo.CmdInProgress == FALSE)
            {
                /*set Cse module is busy*/
                Cse_RunningInfo.CmdInProgress = TRUE;

                /* Write the bytes of the key to PRAM Page1*/
                CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, PlainKey, CSE_PAGE_SIZE_IN_BYTES);/*A-10*/

                /* Write the command header. This will trigger the command execution */
                CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_LOAD_PLAIN_KEY, (uint8)CSE_FUNC_FORMAT_COPY, \
                                           (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_RAM_KEY);/*C-12*/

                /*Wait until the execution of the command is complete*/
                Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
                if (STATUS_SUCCESS == Stat)
                {
                    Stat = CSE_Reg_ReadErrorBits();
                }
                /*free Cse module*/
                Cse_RunningInfo.CmdInProgress = FALSE;
            }
            else /*check CSE Module Status, Busy will return*/
            {
                Stat = STATUS_BUSY;
            }
            (void)System_FlsDeviceUnlock(CSE_DEV);
        }
        else
        {
            Stat = STATUS_ERROR;
        }
    }
    else
    {
        Stat = STATUS_ERROR;
    }

    return Stat;
}

/*!
 * @brief Initializes the PRNG.
 * @note  Function ID:  DES_CSE_API_025
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
static Hal_StatusType CSE_Hal_InitRNG(void)
{
    Hal_StatusType Stat;
    FlashDeviceType DevLock;
    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*Cse Module free will going*/
        if (Cse_RunningInfo.CmdInProgress == FALSE)
        {
            /*set Cse module is busy*/
            Cse_RunningInfo.CmdInProgress = TRUE;

            /* Write the command header. This will trigger the command execution */
            CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_INIT_RNG, (uint8)CSE_FUNC_FORMAT_COPY, \
                                       (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_SECRET_KEY);
            Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
            if (STATUS_SUCCESS == Stat)
            {
                Stat = CSE_Reg_ReadErrorBits();
            }

            /*free Cse module*/
            Cse_RunningInfo.CmdInProgress = FALSE;
        }
        else /*check CSE Module Status, Busy will return*/
        {
            Stat = STATUS_BUSY;
        }
        (void)System_FlsDeviceUnlock(CSE_DEV);
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Get a random challenge data.
 * @note  Function ID:  DES_CSE_API_026
 * @param [out] Challenge: pointer to the 128-bit buffer containing challenge data
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
static Hal_StatusType CSE_Hal_GetRndChallenge(uint8 *Challenge)
{
    DEVICE_ASSERT(NULL_PTR != Challenge);

    Hal_StatusType Stat;
    FlashDeviceType DevLock;
    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*Cse Module free will going*/
        if (Cse_RunningInfo.CmdInProgress == FALSE)
        {
            /*set Cse module is busy*/
            Cse_RunningInfo.CmdInProgress = TRUE;

            /* Write the command header. This will trigger the command execution */
            CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_DBG_CHAL, (uint8)CSE_FUNC_FORMAT_COPY, \
                                       (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_SECRET_KEY);
            Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
            if (STATUS_SUCCESS == Stat)
            {
                Stat = CSE_Reg_ReadErrorBits();
                /* Read the challenge generated by the CSE module */
                if (STATUS_SUCCESS == Stat)
                {
                    CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, Challenge, CSE_PAGE_SIZE_IN_BYTES);
                }
            }

            /*free Cse module*/
            Cse_RunningInfo.CmdInProgress = FALSE;
        }
        else /*check CSE Module Status, Busy will return*/
        {
            Stat = STATUS_BUSY;
        }
        (void)System_FlsDeviceUnlock(CSE_DEV);
    }
    else
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Erases all keys that stored in FLASH memory.
 * @note  Function ID:  DES_CSE_API_027
 * @param [in] Auth:  pointer to the 128-bit buffer containing the authorization value that get from CSE_Hal_GetRndChallenge
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
static Hal_StatusType CSE_Hal_SetResetKeyAuth(const uint8 *Auth)
{
    DEVICE_ASSERT(NULL_PTR != Auth);

    Hal_StatusType Stat;
    FlashDeviceType DevLock;
    DevLock = System_FlsDeviceTryLock(CSE_DEV);
    if (DevLock == CSE_DEV)
    {
        /*Cse Module free will going*/
        if (Cse_RunningInfo.CmdInProgress == FALSE)
        {
            /*set Cse module is busy*/
            Cse_RunningInfo.CmdInProgress = TRUE;

            /* Write the Auth */
            CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, Auth, CSE_PAGE_SIZE_IN_BYTES);
            /* Write the command header. This will trigger the command execution */
            CSE_Reg_WriteCommandHeader((uint8)CSE_CMD_DBG_AUTH, (uint8)CSE_FUNC_FORMAT_COPY, \
                                       (uint8)CSE_CALL_SEQ_FIRST, (uint8)CSE_SECRET_KEY);
            Stat = CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);
            if (STATUS_SUCCESS == Stat)
            {
                Stat = CSE_Reg_ReadErrorBits();
            }
            /*free Cse module*/
            Cse_RunningInfo.CmdInProgress = FALSE;
        }
        else /*check CSE Module Status, Busy will return*/
        {
            Stat = STATUS_BUSY;
        }
        (void)System_FlsDeviceUnlock(CSE_DEV);
    }
    else
    {
        Stat = STATUS_BUSY;
    }
    return Stat;
}

/*!
 * @brief Asynchronously performs the AES-128 encryption in ECB mode.
 * @note  Function ID:  DES_CSE_API_028
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
static Hal_StatusType CSE_Hal_EncryptECBAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat = STATUS_SUCCESS;

    /*Cse Module free will going*/
    if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
    {
        /*init Cse_RunningInfo and start execute request*/
        CSE_Hal_InitCommand(KeyId, CSE_CMD_ENC_ECB, Ptr->InputPtr, Ptr->OutputPtr, Ptr->InputLength);
        /*trigger cmd*/
        CSE_Hal_StartEncDecECBCmd();
        /*enable Cse Interrupt*/
        CSE_Reg_SetInterrupt(TRUE);
    }
    else /*check CSE Module Status, Busy will return*/
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Asynchronously performs the AES-128 decryption in ECB mode.
 * @note  Function ID:  DES_CSE_API_029
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
static Hal_StatusType CSE_Hal_DecryptECBAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat = STATUS_SUCCESS;

    /*Cse Module free will going*/
    if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
    {
        /*init Cse_RunningInfo and start execute request*/
        CSE_Hal_InitCommand(KeyId, CSE_CMD_DEC_ECB, Ptr->InputPtr, Ptr->OutputPtr, Ptr->InputLength);
        /*trigger cmd*/
        CSE_Hal_StartEncDecECBCmd();
        /*enable Cse Interrupt*/
        CSE_Reg_SetInterrupt(TRUE);
    }
    else /*check CSE Module Status, Busy will return*/
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Start the AES-128 encryption in CBC mode of
 * the input plain text buffer, in an asynchronous manner.
 * @note  Function ID:  DES_CSE_API_030
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @return Hal_StatusType
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
static Hal_StatusType CSE_Hal_EncryptCBCAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat = STATUS_SUCCESS;

    /*Cse Module free will going*/
    if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
    {
        /*init Cse_RunningInfo and start execute request*/
        CSE_Hal_InitCommand(KeyId, CSE_CMD_ENC_CBC, Ptr->InputPtr, Ptr->OutputPtr, Ptr->InputLength);
        Cse_RunningInfo.Iv = Ptr->Iv;
        /*trigger cmd*/
        CSE_Hal_StartEncDecCBCCmd();
        /*enable Cse Interrupt*/
        CSE_Reg_SetInterrupt(TRUE);
    }
    else /*check CSE Module Status, Busy will return*/
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Asynchronously performs the AES-128 decryption in CBC mode.
 * @note  Function ID:  DES_CSE_API_031
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
static Hal_StatusType CSE_Hal_DecryptCBCAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat = STATUS_SUCCESS;

    /*Cse Module free will going*/
    if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
    {
        /*init Cse_RunningInfo and start execute request*/
        CSE_Hal_InitCommand(KeyId, CSE_CMD_DEC_CBC, Ptr->InputPtr, Ptr->OutputPtr, Ptr->InputLength);
        Cse_RunningInfo.Iv = Ptr->Iv;
        /*trigger cmd*/
        CSE_Hal_StartEncDecCBCCmd();
        /*enable Cse Interrupt*/
        CSE_Reg_SetInterrupt(TRUE);
    }
    else /*check CSE Module Status, Busy will return*/
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Asynchronously generate the MAC of a given message using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_032
 * @param [in] Ptr: a structure contatining input and output. eg.message/mac.
 * @param [in] KeyId: the index of the key used in cse command.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
static Hal_StatusType CSE_Hal_GenerateMACAsync(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId)
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat = STATUS_SUCCESS;

    /*Cse Module free will going*/
    if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
    {
        /*init Cse_RunningInfo and start execute request*/
        CSE_Hal_InitCommand(KeyId, CSE_CMD_GENERATE_MAC, Ptr->InputPtr, Ptr->OutputPtr, \
            CSE_Hal_RoundTo((Ptr->InputLength<<CSE_BYTES_TO_FROM_BITS_SHIFT), 0x8U) >> CSE_BYTES_TO_FROM_BITS_SHIFT);
        Cse_RunningInfo.MsgLen = (Ptr->InputLength << CSE_BYTES_TO_FROM_BITS_SHIFT);
        /*trigger Cse cmd*/
        CSE_Hal_StartGenMACCmd();
        /*enable Cse Interrupt*/
        CSE_Reg_SetInterrupt(TRUE);
    }
    else /*check CSE Module Status, Busy will return*/
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Asynchronously verifies the MAC of a given message using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_033
 * @param [in] Ptr: a structure contatining input and output. eg.message/mac.
 * @param [in] KeyId: the index of the key used in cse command.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
static Hal_StatusType CSE_Hal_VerifyMACAsync(const Cse_InputOutputType *Ptr,
        Cse_KeyId KeyId)/*cstat !MISRAC2012-Rule-8.13 Output parameter*/
{
    DEVICE_ASSERT(NULL_PTR != Ptr);

    Hal_StatusType Stat = STATUS_SUCCESS;
    uint32 MsgLenTemp = 0u;

    /*Cse Module free will going*/
    if ((Cse_RunningInfo.CmdInProgress == FALSE) && (NULL_PTR != Ptr))
    {
        MsgLenTemp = Ptr->InputLength << CSE_BYTES_TO_FROM_BITS_SHIFT;

        /*init Cse_RunningInfo and start execute request*/
        CSE_Hal_InitCommand(KeyId, CSE_CMD_VERIFY_MAC, Ptr->InputPtr, NULL_PTR, \
                            CSE_Hal_RoundTo(MsgLenTemp, 0x8) >> CSE_BYTES_TO_FROM_BITS_SHIFT);
        Cse_RunningInfo.MsgLen = MsgLenTemp;
        Cse_RunningInfo.VerifStatus = Ptr->VerifyStatus;
        Cse_RunningInfo.MacWritten = FALSE;
        Cse_RunningInfo.Mac = Ptr->SecondaryInputPtr;
        Cse_RunningInfo.MacLen = Ptr->SecondaryInputLength << CSE_BYTES_TO_FROM_BITS_SHIFT;
        /*trigger Cse cmd*/
        CSE_Hal_StartVerifyMACCmd();
        /*enable Cse Interrupt*/
        CSE_Reg_SetInterrupt(TRUE);
    }
    else /*check CSE Module Status, Busy will return*/
    {
        Stat = STATUS_BUSY;
    }

    return Stat;
}

/*!
 * @brief Implementation of the CSE interrupt handler file.
 * @note  Function ID:  DES_CSE_API_034
 * @return none
 */
 /*PRQA S 1503 ++ # interrupt handle function.*/
ISR(CSE_IRQHandler)
{
    /*Read Cse Cmd completed interrupt flag*/
    uint8 Stat = CSE_Reg_GetCmdCompleteStatus();
    /*clear Cse Cmd completed interrupt flag*/
    CSE_Reg_ClearCmdCompleteStatus();

    /*Cse Cmd completed and Cse is free will continue next cmd*/
    if ((0U != Stat) && (Cse_RunningInfo.CmdInProgress == TRUE))
    {
        switch (Cse_RunningInfo.Cmd) /*cmd info*/
        {
        case CSE_CMD_ENC_ECB:
        case CSE_CMD_DEC_ECB:
            /*AES ECB cmd continue*/
            Cse_RunningInfo.CmdInProgress = CSE_Hal_ContinueEncDecECBCmd();
            break;
        case CSE_CMD_ENC_CBC:
        case CSE_CMD_DEC_CBC:
            /*AES CBC cmd continue*/
            Cse_RunningInfo.CmdInProgress = CSE_Hal_ContinueEncDecCBCCmd();
            break;
        case CSE_CMD_GENERATE_MAC:
            /*GenMac cmd continue*/
            Cse_RunningInfo.CmdInProgress = CSE_Hal_ContinueGenMACCmd();
            break;
        case CSE_CMD_VERIFY_MAC:
            /*VerifyMac cmd continue*/
            Cse_RunningInfo.CmdInProgress = CSE_Hal_ContinueVerifyMACCmd();
            break;
        default:
            /* Do nothing */
            break;
        }

        /* Finished operation, disable CSE interrupt */
        if (Cse_RunningInfo.CmdInProgress == FALSE)
        {
            (void)System_FlsDeviceUnlock(CSE_DEV);
            /*disable interrupt*/
            CSE_Reg_SetInterrupt(FALSE);

            if (NULL_PTR != Cse_RunningInfo.Cse_IsrCallback)
            {
                /*invoke callback function*/
                Cse_RunningInfo.Cse_IsrCallback(Cse_RunningInfo.CallbackArgs);
            }
        }
    }
}
 /*PRQA S 1503 -- # interrupt handle function.*/

/*!
 * @brief Performs the AES-128 encryption and decryption in ECB mode.
 * @note Function ID:  DES_CSE_API_035
 * @return none
 */
static void CSE_Hal_StartEncDecECBCmd(void)
{
    /*PRAM each page is 16 bytes, NumPagesLeft for need PRAM page number*/
    uint32 NumPagesLeft = (Cse_RunningInfo.InputBufferSize - Cse_RunningInfo.Index) >> \
                          CSE_BYTES_TO_FROM_PAGES_SHIFT;

    /*NumPages Max is 7 every operation, because PRAM hw limited*/
    uint16 NumPages = (uint16)((NumPagesLeft > CSE_DATA_PAGES_AVAILABLE) ? \
                               CSE_DATA_PAGES_AVAILABLE : NumPagesLeft);

    /* Bytes equal NumPages*16 */
    uint8 NumBytes = (uint8)(NumPages << CSE_BYTES_TO_FROM_PAGES_SHIFT);
    /*write the plain/cipher text ro PRAM(1-7)*/
    CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, &Cse_RunningInfo.InputBuff[Cse_RunningInfo.Index], NumBytes);

    /*write Numpages to Pram page0 <Word3 High 16bit> for command specific, data takes up pages number of Cse memory*/
    CSE_Reg_WriteCommandHalfWord(FEATURE_CSE_PAGE_LENGTH_OFFSET, NumPages);

    /*store enc/dec Bytes number for this ECB Cmd operation*/
    Cse_RunningInfo.PartSize = NumBytes;

    /*write Cmd to Pram commnd Header , Cse will start cmd operation*/
    CSE_Reg_WriteCommandHeader((uint8)Cse_RunningInfo.Cmd, (uint8)CSE_FUNC_FORMAT_COPY, \
                               (uint8)Cse_RunningInfo.Seq, (uint8)Cse_RunningInfo.KeyId);
}

/*!
 * @brief Start performs the AES-128 encryption and decryption in CBC mode.
 * @note  Function ID:  DES_CSE_API_036
 * @return none
 */
static void CSE_Hal_StartEncDecCBCCmd(void)
{
    /*PRAM each page is 16 bytes, NumPagesLeft for need PRAM page number*/
    uint32 NumPagesLeft = (Cse_RunningInfo.InputBufferSize - Cse_RunningInfo.Index) >> \
                          CSE_BYTES_TO_FROM_PAGES_SHIFT;

    /*NumPages Max is 7 every operation, because PRAM hw limited*/
    uint16 NumPages = (uint16)((NumPagesLeft > CSE_DATA_PAGES_AVAILABLE) ? \
                               CSE_DATA_PAGES_AVAILABLE : NumPagesLeft);

    /* Bytes equal NumPages*16 */
    uint8 NumBytes = (uint8)(NumPages << CSE_BYTES_TO_FROM_PAGES_SHIFT);

    /*Fist input text*/
    if (CSE_CALL_SEQ_FIRST == Cse_RunningInfo.Seq)
    {
        NumPages = (uint16)((NumPagesLeft > (CSE_DATA_PAGES_AVAILABLE - 1U)) ? \
                            (CSE_DATA_PAGES_AVAILABLE - 1U) : NumPagesLeft);
        NumBytes = (uint8)(NumPages << CSE_BYTES_TO_FROM_PAGES_SHIFT);

        /* Write the initialization vector to PRAM Page 1*/
        CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, Cse_RunningInfo.Iv, CSE_PAGE_SIZE_IN_BYTES);

        /*write the plain/cipher text ro PRAM Page(2-7)*/
        CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_2_OFFSET, \
                                  &Cse_RunningInfo.InputBuff[Cse_RunningInfo.Index], NumBytes);
    }
    else /*not first input*/
    {
        /* will from PRAM Page1 start continue input data */
        CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, \
                                  &Cse_RunningInfo.InputBuff[Cse_RunningInfo.Index], NumBytes);
    }

    /*write Numpages to Pram page0 <Word3 High 16bit> for command specific, data takes up pages number of Cse memory*/
    CSE_Reg_WriteCommandHalfWord(FEATURE_CSE_PAGE_LENGTH_OFFSET, \
                                 (uint16)(Cse_RunningInfo.InputBufferSize >> CSE_BYTES_TO_FROM_PAGES_SHIFT));

    /*store enc/dec Bytes number for this Cmd operation*/
    Cse_RunningInfo.PartSize = NumBytes;

    /* Write the command header. This will trigger the command execution */
    CSE_Reg_WriteCommandHeader((uint8)Cse_RunningInfo.Cmd, (uint8)CSE_FUNC_FORMAT_COPY, \
                               (uint8)Cse_RunningInfo.Seq, (uint8)Cse_RunningInfo.KeyId);
}

/*!
 * @brief Start generate the MAC using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_037
 * @return none
 */
static void CSE_Hal_StartGenMACCmd(void)
{
    /* will Gen Mac data Bytes number , each Max byes is 112*/
    uint8 NumBytes = (uint8)(((Cse_RunningInfo.InputBufferSize - Cse_RunningInfo.Index) >
                              CSE_DATA_BYTES_AVAILABLE) ? CSE_DATA_BYTES_AVAILABLE : \
                             (Cse_RunningInfo.InputBufferSize - Cse_RunningInfo.Index));

    /*write the input data to PRAM Page(1-7)*/
    CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET,
                              &Cse_RunningInfo.InputBuff[Cse_RunningInfo.Index], NumBytes);

    /*write generate data(bits) to PRAM page0 <Word3> for command specific*/
    CSE_Reg_WriteCommandWords(FEATURE_CSE_MESSAGE_LENGTH_OFFSET, &Cse_RunningInfo.MsgLen, 1U);

    /*store Bytes number for this Cmd operation*/
    Cse_RunningInfo.PartSize = NumBytes;

    /* Write the command header. This will trigger the command execution */
    CSE_Reg_WriteCommandHeader((uint8)Cse_RunningInfo.Cmd, (uint8)CSE_FUNC_FORMAT_COPY, \
                               (uint8)Cse_RunningInfo.Seq, (uint8)Cse_RunningInfo.KeyId);
}

/*!
 * @brief Start vrify the MAC using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_038
 * @return none
 */
static void CSE_Hal_StartVerifyMACCmd(void)
{
    /* will Verify Mac data Bytes number , each Max byes is 112*/
    uint8 NumBytes = (uint8)(((Cse_RunningInfo.InputBufferSize - Cse_RunningInfo.Index) > \
                              CSE_DATA_BYTES_AVAILABLE) ? CSE_DATA_BYTES_AVAILABLE : \
                             (Cse_RunningInfo.InputBufferSize - Cse_RunningInfo.Index));
    /* align at 0x10*/
    uint8 MacOffset = (uint8)CSE_Hal_RoundTo(NumBytes, 0x10U);

    /*write the input data to PRAM Page(1-7)*/
    CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, &Cse_RunningInfo.InputBuff[Cse_RunningInfo.Index], NumBytes);

    /*write generate data(bits) to PRAM page0 <Word3> for command specific*/
    CSE_Reg_WriteCommandWords(FEATURE_CSE_MESSAGE_LENGTH_OFFSET, &Cse_RunningInfo.MsgLen, 1U);

    /*write verify data(bits) to PRAM page0 <Word2> for command specific*/
    CSE_Reg_WriteCommandHalfWord(FEATURE_CSE_MAC_LENGTH_OFFSET, (uint16)Cse_RunningInfo.MacLen);

    /* If there is available space in CSE_PRAM, write the MAC to be verified */
    if ((MacOffset + CSE_PAGE_SIZE_IN_BYTES) <= CSE_DATA_BYTES_AVAILABLE)
    {
        /*MAC Data will write it after input data*/
        CSE_Reg_WriteCommandBytes(FEATURE_CSE_PAGE_1_OFFSET + MacOffset, Cse_RunningInfo.Mac, CSE_PAGE_SIZE_IN_BYTES);

        /*set Mac Written complete*/
        Cse_RunningInfo.MacWritten = TRUE;
    }

    /*store Bytes number for this Cmd operation*/
    Cse_RunningInfo.PartSize = NumBytes;

    /* Write the command header. This will trigger the command execution */
    CSE_Reg_WriteCommandHeader((uint8)Cse_RunningInfo.Cmd, (uint8)CSE_FUNC_FORMAT_COPY, \
                               (uint8)Cse_RunningInfo.Seq, (uint8)Cse_RunningInfo.KeyId);
}

/*!
 * @brief Continues perform encryption/decryption using ECB mode.
 * @note  Function ID:  DES_CSE_API_039
 * @return none
 */
static boolean CSE_Hal_ContinueEncDecECBCmd(void)
{
    boolean Res = TRUE;
    /* Read the status of the execution */
    Cse_RunningInfo.ErrCode = CSE_Reg_ReadErrorBits();
    if (STATUS_SUCCESS == Cse_RunningInfo.ErrCode)
    {
        /* Get partial result */
        CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, &Cse_RunningInfo.OutputBuff[Cse_RunningInfo.Index], \
                                 (uint8)Cse_RunningInfo.PartSize);
        /*update InputBuffer Offset*/
        Cse_RunningInfo.Index += (uint8)Cse_RunningInfo.PartSize;
        /* Decide if more commands are needed */
        if (Cse_RunningInfo.Index >= Cse_RunningInfo.InputBufferSize)
        {
            Res = FALSE;
        }
        else
        {
            /* Continue launching commands */
            CSE_Hal_StartEncDecECBCmd();
        }
    }
    else
    {
        /* Do not continue launching commands if an error occurred */
        Res = FALSE;
    }

    return Res;
}

/*!
 * @brief Continues perform encryption/decryption using CBC mode.
 * @note  Function ID:  DES_CSE_API_040
 * @return none
 */
static boolean CSE_Hal_ContinueEncDecCBCCmd(void)
{
    boolean Res = TRUE;
    /* Read the status of the execution */
    Cse_RunningInfo.ErrCode = CSE_Reg_ReadErrorBits();
    if (STATUS_SUCCESS == Cse_RunningInfo.ErrCode)
    {
        /* Get partial result */
        if (CSE_CALL_SEQ_FIRST == Cse_RunningInfo.Seq)
        {
            /*form PRAM Page2 start read data, because first Iv store Page1*/
            CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_2_OFFSET, &Cse_RunningInfo.OutputBuff[Cse_RunningInfo.Index], \
                                     (uint8)Cse_RunningInfo.PartSize);

            /*update Call Seq*/
            Cse_RunningInfo.Seq = CSE_CALL_SEQ_SUBSEQUENT;
        }
        else
        {
            /*form PRAM Page1 start read data when not First inputText*/
            CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_1_OFFSET, &Cse_RunningInfo.OutputBuff[Cse_RunningInfo.Index], \
                                     (uint8)Cse_RunningInfo.PartSize);
        }

        /*update InputBuffer Offset*/
        Cse_RunningInfo.Index += (uint8)Cse_RunningInfo.PartSize;

        /*if InputBuffer Index >= Fullsize  expression data enc/dec complete*/
        if (Cse_RunningInfo.Index >= Cse_RunningInfo.InputBufferSize)
        {
            Res = FALSE;
        }
        else /*data enc/dec not complete, will continue */
        {
            /* Continue launching commands */
            CSE_Hal_StartEncDecCBCCmd();
        }
    }
    else
    {
        /* Do not continue launching commands if an error occurred */
        Res = FALSE;
    }

    return Res;
}

/*!
 * @brief Continues generate the MAC using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_041
 * @return none
 */
static boolean CSE_Hal_ContinueGenMACCmd(void)
{
    boolean Res = TRUE;
    /* Read the status of the execution */
    Cse_RunningInfo.ErrCode = CSE_Reg_ReadErrorBits();
    if (STATUS_SUCCESS == Cse_RunningInfo.ErrCode)
    {
        if (CSE_CALL_SEQ_FIRST == Cse_RunningInfo.Seq)
        {
            /*update Call Seq for next operation when Call seq first*/
            Cse_RunningInfo.Seq = CSE_CALL_SEQ_SUBSEQUENT;
        }

        /*update input buffer index*/
        Cse_RunningInfo.Index += (uint8)Cse_RunningInfo.PartSize;

        /*if InputBuffer Index >= Fullsize  expression data generate mac complete*/
        if (Cse_RunningInfo.Index >= Cse_RunningInfo.InputBufferSize)
        {
            Res = FALSE;
            /*Read Mac Value to outputbuffer when operateion completed*/
            CSE_Reg_ReadCommandBytes(FEATURE_CSE_PAGE_2_OFFSET, Cse_RunningInfo.OutputBuff, CSE_PAGE_SIZE_IN_BYTES);
        }
        else
        {
            /* Continue launching commands when data not completed*/
            CSE_Hal_StartGenMACCmd();
        }
    }
    else
    {
        /* Do not continue launching commands if an error occurred */
        Res = FALSE;
    }

    return Res;
}

/*!
 * @brief Continues verify the MAC using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_042
 * @return none
 */
static boolean CSE_Hal_ContinueVerifyMACCmd(void)
{
    boolean Res;
    /* Read the status of the execution */
    Cse_RunningInfo.ErrCode = CSE_Reg_ReadErrorBits();
    if (STATUS_SUCCESS == Cse_RunningInfo.ErrCode)
    {
        if (CSE_CALL_SEQ_FIRST == Cse_RunningInfo.Seq)
        {
            Cse_RunningInfo.Seq = CSE_CALL_SEQ_SUBSEQUENT;
        }

        Cse_RunningInfo.Index += (uint8)Cse_RunningInfo.PartSize;

        /* Decide if more commands are needed */
        Cse_RunningInfo.CmdInProgress = (Cse_RunningInfo.MacWritten == FALSE)?TRUE:FALSE;

        if (Cse_RunningInfo.CmdInProgress == FALSE)
        {
            *(Cse_RunningInfo.VerifStatus) =
                (CSE_Reg_ReadCommandHalfWord(FEATURE_CSE_VERIFICATION_STATUS_OFFSET) == 0U)?1u:0u;
        }
        else
        {
            /* Continue launching commands */
            CSE_Hal_StartVerifyMACCmd();
        }
        Res = Cse_RunningInfo.CmdInProgress;
    }
    else
    {
        /* Do not continue launching commands if an error occurred */
        Res = FALSE;
    }

    return Res;
}

/*!
 * @brief Initializes the internal state of the driver.
 * @note  Function ID:  DES_CSE_API_043
 * @param [in] KeyId: operation use key id
 * @param [in] Cmd: CSE command
 * @param [in] InBuff: pointer to input buffer
 * @param [in] OutBuff: pointer to output buffer
 * @param [in] Length: inbuffer length
 * @return none
 */
static void CSE_Hal_InitCommand(Cse_KeyId KeyId, Cse_CmdType Cmd, const uint8 *InBuff,
                                uint8 *OutBuff, uint32 Length)/*cstat !MISRAC2012-Rule-8.13 Output buffer*/
{
    /*Cse module is busy*/
    Cse_RunningInfo.CmdInProgress = TRUE;
    Cse_RunningInfo.Cmd = Cmd;
    Cse_RunningInfo.InputBuff = InBuff;
    Cse_RunningInfo.OutputBuff = OutBuff;/*cstat !MISRAC2012-Rule-8.13  Output buffer*/
    Cse_RunningInfo.KeyId = KeyId;
    Cse_RunningInfo.InputBufferSize = Length;
    Cse_RunningInfo.Index = 0U;
    Cse_RunningInfo.ErrCode = STATUS_SUCCESS;
    /*Cse data first input*/
    Cse_RunningInfo.Seq = CSE_CALL_SEQ_FIRST;
}

/*!
 * @brief Connect the parameter authkey with constant and compress the result.
 * @note  Function ID:  DES_CSE_API_044
 * @param [in] AuthKey: authorizing key
 * @param [in] Constant: constant value
 * @param [out] K_out: output buffer
 * @return Hal_StatusType
 */
static Hal_StatusType CSE_Hal_KDF(const uint8 *AuthKey, const uint8 *Constant, uint8 *K_out)
{
    DEVICE_ASSERT(NULL_PTR != AuthKey);
    DEVICE_ASSERT(NULL_PTR != Constant);
    DEVICE_ASSERT(NULL_PTR != K_out);

    Hal_StatusType Res;
    uint8 i;
    uint8 Concat[32U] = {0};
    if ((NULL_PTR != AuthKey) && (NULL_PTR != Constant))
    {
        for (i = 0U; i < 16U; i++)
        {
            Concat[i] = AuthKey[i];
            Concat[i + 16U] = Constant[i];
        }
        Res = CSE_Hal_MPCompress(Concat, 2U, K_out, 2U);
    }
    else
    {
        Res = STATUS_ERROR;
    }

    return Res;
}

/*!
 * @brief Calculate M1 To M5.
 * @note  Function ID:  DES_CSE_API_044
 * @param [in] AuthKey: the key content to be updated.
 * @param [out] Kconstant: store M1~M5 value.
 * @return Hal_StatusType
 */
static Hal_StatusType CSE_Hal_SetKoutToRam(const uint8 *AuthKey, const uint8 *Kconstant)
{
    Hal_StatusType Res;
    uint8 Kout[16U];
    Res = CSE_Hal_KDF(AuthKey, Kconstant, Kout);
    if (STATUS_SUCCESS == Res)
    {
        Res = CSE_Hal_LoadPlainKey(Kout);
    }
    return Res;
}

/*!
 * @brief Calculate M1 To M5.
 * @note  Function ID:  DES_CSE_API_044
 * @param [in] InPtr: the key content to be updated.
 * @param [out] OutPtr: store M1~M5 value.
 * @return Hal_StatusType
 */
Hal_StatusType CSE_Hal_CalcM1ToM5(const Cse_InputKeyInfoType *InPtr, const Cse_SheKeyInfoType *OutPtr)
{
    DEVICE_ASSERT(NULL_PTR != InPtr);
    DEVICE_ASSERT(NULL_PTR != OutPtr);

    Hal_StatusType Res;
    uint8 UID[16U];
    uint8 UID_MAC[16U];
    uint8 M4_star[16U];
    uint8 M4_tmp[16U];
    uint8 Iv[16U];
    uint8 PLAIN_TEXT[32U];
    uint8 M1M2[48U];
    const uint8 Challenge[16U] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U};
    uint8 Sreg;
    uint8 i;
    Cse_InputOutputType Ptr;
    Cse_KeyAttr KeyAttr = KEY_ATTR_NONE;

    static const uint8 KEY_UPDATE_ENC_C[16U] = {0x01u, 0x01u, 0x53u, 0x48u, 0x45u, 0x00u, 0x80u, 0x00u, 0x00u, \
                                                0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xB0u};
    static const uint8 KEY_UPDATE_MAC_C[16U] = {0x01u, 0x02u, 0x53u, 0x48u, 0x45u, 0x00u, 0x80u, 0x00u, 0x00u, \
                                                0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xB0u};

    MemSet(UID, 0U, 16U);
    MemSet(Iv, 0U, 16U);
    MemSet(M4_tmp, 0U, 16U);

    if ((NULL_PTR != InPtr) && (NULL_PTR != OutPtr))
    {
        /*PRQA S 4523 ++ # the convert between int and enum.*/
        /*PRQA S 4393 ++ # the convert between int and enum.*/
        /*PRQA S 4332 ++ # the convert between int and enum.*/
        KeyAttr = (Cse_KeyAttr)MATCH_KEYATTR(InPtr->NewKeyAttr);
        /*PRQA S 4523 -- # the convert between int and enum.*/
        /*PRQA S 4393 -- # the convert between int and enum.*/
        /*PRQA S 4332 -- # the convert between int and enum.*/
        (void)CSE_Hal_GetID(Challenge, UID, &Sreg, UID_MAC);

        /* Calculate M1 */
        for (i = 0U; i < 15U; i++)
        {
            OutPtr->M1[i] = UID[i];
        }
        OutPtr->M1[15U] = (((uint8)(InPtr->NewKeyId) & 0x0FU) << 4U) | ((uint8)(InPtr->AuthKeyId) & 0x0FU);

        /* Calculate M2 */
        /* First, calculate K1 and load K1 into RAM */
        Res = CSE_Hal_SetKoutToRam((const uint8 *)InPtr->AuthKey, KEY_UPDATE_ENC_C);
        if (STATUS_SUCCESS == Res)
        {
            MemSet((PLAIN_TEXT), 0u, 16u);
            /* Second, concatenate the input data */
#if defined(AC7840X)
            /*0bit~27bit: 28bit CID  29bit~34bit:5bit FID*/
            PLAIN_TEXT[4U] = (uint8)((uint8)KeyAttr & 0x0FU) << 6U;
            /*PRQA S 1891 ++ #the upper layer call guarantees that a null pointer will never appear.*/
            PLAIN_TEXT[3U] = (uint8)(((InPtr->Count & 0x0000000FU) << 4U) | (((uint8)KeyAttr & 0x3CU) >> 2U));
            /*PRQA S 1891 -- #the upper layer call guarantees that a null pointer will never appear.*/
#elif defined(AC7842X)
            /*0bit~27bit: 28bit CID  29bit~34bit:5bit FID*/
            PLAIN_TEXT[4U] = (uint8)((uint8)KeyAttr & 0x07U) << 5U;
            /*PRQA S 1891 ++ # unsigned type(unsigned char) is allowed*/
            PLAIN_TEXT[3U] = (uint8)(((InPtr->Count & 0x0000000FU) << 4U) | (((uint8)KeyAttr & 0x78U) >> 3U));
            /*PRQA S 1891 -- # unsigned type(unsigned char) is allowed*/
#endif
            PLAIN_TEXT[2U] = (uint8)((InPtr->Count & 0x00000FF0U) >> 4U);
            PLAIN_TEXT[1U] = (uint8)((InPtr->Count & 0x000FF000U) >> 12U);
            PLAIN_TEXT[0U] = (uint8)((InPtr->Count & 0x0FF00000U) >> 20U);
            /* 128bit~255bit key id */
            for (i = 0U; i < 16U; i++)
            {
                PLAIN_TEXT[16U + i] = InPtr->NewKey[i];
            }
            Ptr.InputPtr = PLAIN_TEXT;
            Ptr.InputLength = 32U;
            Ptr.Iv = Iv;
            Ptr.OutputPtr = OutPtr->M2;
            Ptr.OutputLength = 32U;
            Res = CSE_Hal_EncryptCBC(&Ptr, CSE_RAM_KEY, 2U);
        }
        /* Calculate M3 */
        /* First, calculate K2 load it as RAM_KEY */
        if (STATUS_SUCCESS == Res)
        {
            Res = CSE_Hal_SetKoutToRam((const uint8 *)InPtr->AuthKey, KEY_UPDATE_MAC_C);
        }

        if (STATUS_SUCCESS == Res)
        {
            /* concatenate M1 and M2 */
            for (i = 0U; i < 16U; i++)
            {
                M1M2[i] = OutPtr->M1[i];
            }
            for (i = 16U; i < 48U; i++)
            {
                M1M2[i] = OutPtr->M2[i - 16U];
            }
            Ptr.InputPtr = M1M2;
            Ptr.OutputPtr = OutPtr->M3;
            Ptr.InputLength = 48U;
            Res = CSE_Hal_GenerateMAC(&Ptr, CSE_RAM_KEY, 2U);
        }
        /* First, calculate K3 load it as RAM_KEY */
        if (STATUS_SUCCESS == Res)
        {
            Res = CSE_Hal_SetKoutToRam((const uint8 *)InPtr->NewKey, KEY_UPDATE_ENC_C);
        }

        if (STATUS_SUCCESS == Res)
        {
            /* calculate K4 */
            M4_tmp[3U] = (uint8)(((InPtr->Count & 0x0000000FU) << 4U) | 0x08U);
            M4_tmp[2U] = (uint8)((InPtr->Count & 0x00000FF0U) >> 4U);
            M4_tmp[1U] = (uint8)((InPtr->Count & 0x000FF000U) >> 12U);
            M4_tmp[0U] = (uint8)((InPtr->Count & 0x0FF00000U) >> 20U);
            Ptr.InputPtr = M4_tmp;
            Ptr.InputLength = 16U;
            Ptr.Iv = Iv;
            Ptr.OutputPtr = M4_star;
            Res = CSE_Hal_EncryptCBC(&Ptr, CSE_RAM_KEY, 2U);
        }
        /* Calculate M5 */
        if (STATUS_SUCCESS == Res)
        {
            /* 0bit ~ 119bit UID */
            for (i = 0U; i < 15U; i++)
            {
                OutPtr->M4[i] = UID[i];
            }
            /* 120bit ~ 127bit KeyID(4bits)|AuthID(4bits) */
            OutPtr->M4[15U] = ((uint8)(InPtr->NewKeyId) << 4U) | ((uint8)(InPtr->AuthKeyId) & 0x0FU);
            /* 128bit ~ 255bit M4 */
            for (i = 0U; i < 16U; i++)
            {
                OutPtr->M4[16U + i] = M4_star[i];
            }
            Res = CSE_Hal_SetKoutToRam((const uint8 *)InPtr->NewKey, KEY_UPDATE_MAC_C);
        }

        if (STATUS_SUCCESS == Res)
        {
            Ptr.InputPtr = OutPtr->M4;
            Ptr.OutputPtr = OutPtr->M5;
            Ptr.InputLength = 32U;
            Res = CSE_Hal_GenerateMAC(&Ptr, CSE_RAM_KEY, 2U);
        }
    }
    else
    {
        Res = STATUS_ERROR;
    }

    return Res;
}

#ifndef CSE_SDK_NON_EXTENDED_API
/*!
 * @brief Returns the content of the status register.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetStatus(void)
{
    return CSE_Reg_GetStatus();/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
}

/*!
 * @brief Returns the content of the CSESTAT BOK bit value.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetBOKStatus(void)
{
    return ((CSE_Reg_GetStatus() & FLASH_CSESTAT_BOK_Msk) >> FLASH_CSESTAT_BOK_Pos);/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
}

/*!
 * @brief Returns the content of the CSESTAT BFN bit value.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetBFNStatus(void)
{
    return ((CSE_Reg_GetStatus() & FLASH_CSESTAT_BFN_Msk) >> FLASH_CSESTAT_BFN_Pos);/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
}

/*!
 * @brief Returns the content of the CSESTAT BIN bit value.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetBINStatus(void)
{
    return ((CSE_Reg_GetStatus() & FLASH_CSESTAT_Bin_Msk) >> FLASH_CSESTAT_Bin_Pos);/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
}

/*!
 * @brief Returns the content of the CSESTAT SB bit value.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetSBStatus(void)
{
    return ((CSE_Reg_GetStatus() & FLASH_CSESTAT_SB_Msk) >> FLASH_CSESTAT_SB_Pos);/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
}

/*!
 * @brief the content of the CSESTAT BSY bit value.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetBSYStatus(void)
{
    /*PRQA S 2985 ++ #the upper layer call guarantees that a null pointer will never appear.*/
    return ((CSE_Reg_GetStatus() & FLASH_CSESTAT_BSY_Msk) >> FLASH_CSESTAT_BSY_Pos);
    /*PRQA S 2985 -- #the upper layer call guarantees that a null pointer will never appear.*/
}
#endif
/*!
* @brief Calculate authorization value buffer for debug authorization
* @note  Function ID:  DES_CSE_API_045
* @param [in] MasterEcuKey: Master ECU key buffer
* @param [in] Challenge: Challenge buffer
* @param [out] AuthOut: authorization buffer
* @return Hal_StatusType
*/
static Hal_StatusType CSE_Hal_CalcResetKeyAuth(const uint8 *MasterEcuKey, const uint8 *Challenge, uint8 *AuthOut)
{
    DEVICE_ASSERT(NULL_PTR != MasterEcuKey);
    DEVICE_ASSERT(NULL_PTR != Challenge);
    DEVICE_ASSERT(NULL_PTR != AuthOut);

    Hal_StatusType Res;
    uint8 Sreg = 0u;
    uint8 i;
    uint8 K_out[16u];
    uint8 UID[16u];/*UID just have 15 bytes*/
    uint8 UID_MAC[16U];
    uint8 const UidChallenge[16u] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u};
    uint8 DATA[32u];
    static const uint8 DEBUG_KEY_C[16] = {0x01u, 0x03u, 0x53u, 0x48u, 0x45u, 0x00u, 0x80u, 0x00u, \
                                          0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xB0u};

    MemSet(K_out, 0U, 16u);
    MemSet(UID, 0U, 16u);
    MemSet(UID_MAC, 0U, 16u);

    Cse_InputOutputType Ptr;
    if ((NULL_PTR != MasterEcuKey) && (NULL_PTR != Challenge) && (NULL_PTR != AuthOut))
    {
        /*step1 Calculate the Authorization*/
        Res = CSE_Hal_KDF(MasterEcuKey, DEBUG_KEY_C, K_out);
        if (STATUS_SUCCESS == Res)
        {
            Res = CSE_Hal_LoadPlainKey(K_out);
        }
        if (STATUS_SUCCESS == Res)
        {
            Res = CSE_Hal_GetID(UidChallenge, UID, &Sreg, UID_MAC);
        }

        if (STATUS_SUCCESS == Res)
        {
            for (i = 0u; i < 16u; i++)
            {
                DATA[i] = Challenge[i];
            }
            for (; i < 32u; i++)
            {
                DATA[i] = UID[i - 16u];
            }
        }
        Ptr.InputPtr = DATA;
        Ptr.InputLength = 31u;
        Ptr.OutputPtr = AuthOut;
        Res = CSE_Hal_GenerateMAC(&Ptr, CSE_RAM_KEY, 2u);

    }
    else
    {
        Res = STATUS_ERROR;
    }

    return Res;
}

/*!
 * @brief Calculates value to the first number multiple of roundTo.
 * @note  Function ID:  DES_CSE_API_046
 * @param [in] Value: rouding value
 * @param [in] RoundTo: original value
 * @return Rounding results value
 */
static uint32 CSE_Hal_RoundTo(uint32 Value, uint32 RoundTo)
{
    return (Value + (RoundTo - 1U)) & (~(RoundTo - 1U));
}

/*!
 * @brief memset arr.
 * @note  Function ID:  DES_CSE_API_47
 * @param [in] buffer: rouding value buffer
 * @param [in] data: set value
 * @param [in] size: arr size
 * @return Rounding results value
 */
static void MemSet(uint8 *buffer, uint8 data, uint32 size)
{
    uint32 i = 0;
    for (i = 0; i < size; i++)
    {
        buffer[i] = data;
    }
}
#endif /*AC7840X or AC7842X*/
/* =============================================  EOF  ============================================== */
