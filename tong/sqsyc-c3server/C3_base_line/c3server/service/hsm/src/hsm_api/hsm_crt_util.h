/**
 * @file hsm_crt_util.h
 * @brief HSM file management utility interface
 * @description File operation interfaces for SE chip, including create, delete, select, read, write operations
 * @note Based on CIU98_B V2 SE[3162] User Manual V1.2
 */

#ifndef _HSM_FILE_UTIL_H
#define _HSM_FILE_UTIL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HSM_FILE_DEFAULT_SIZE       512
#define HSM_FILE_DEFAULT_READ_ATTR  0x02
#define HSM_FILE_DEFAULT_WRITE_ATTR 0x20
#define HSM_FILE_DEFAULT_READ_KEY_ID  0x00
#define HSM_FILE_DEFAULT_WRITE_KEY_ID 0x01
#define HSM_FILE_MAX_CONTENT_LEN    4032  ///< Max user data length (2 bytes header + data)

/**
 * @brief File type definition
 */
typedef enum {
    HSM_FILE_TYPE_DDF = 0x01,   ///< DDF (Dedicated File)
    HSM_FILE_TYPE_ADF = 0x02,   ///< ADF (Application Dedicated File)
    HSM_FILE_TYPE_BIN = 0x03,   ///< Binary file (EF)
} hsm_crt_type_t;

/**
 * @brief File attribute flags
 */
typedef enum {
    HSM_FILE_ATTR_NONE    = 0x00,   ///< No attribute
    HSM_FILE_ATTR_APIN    = 0x80,   ///< Require admin PIN for access
    HSM_FILE_ATTR_UPIN    = 0x40,   ///< Require user PIN for access
    HSM_FILE_ATTR_DISU    = 0x08,   ///< Disable update operation (binary file only)
} hsm_crt_attr_t;

/**
 * @brief Delete a binary file from SE
 * @param fid: File ID (2 bytes)
 * @return 0 on success, negative error code on failure
 * @note Default file type is HSM_FILE_TYPE_BIN
 * @note P1=0x02: Delete file by FID
 * @note INS=0xE4, CLA=0x80
 */
int hsm_crt_delete(uint16_t fid);

/**
 * @brief Export data from a file (READBINARY)
 * @param fid: File ID (2 bytes)
 * @param len: Max output buffer size (must be >= actual data length)
 * @param out_buf: Output data buffer
 * @param out_len: Output data length (actual user data bytes, excluding 2-byte header)
 * @return 0 on success, negative error code on failure
 * @note File format: [data_len(2 bytes, big-endian)][data...]
 * @note This function reads the 2-byte length header first, then reads actual data
 * @note INS=0xB0, CLA=0x00 (plaintext), offset=0
 * @note This function internally calls api_select_file before reading
 */
int hsm_crt_export(uint16_t fid, uint32_t len, uint8_t *out_buf, uint32_t *out_len);

/**
 * @brief Query EF file list and return FID array
 * @param out_buf: Output buffer for FID array (uint16_t array)
 * @param out_len: Output data length in bytes (FID count * 2)
 * @return 0 on success, negative error code on failure
 * @note This function queries the current directory for ALL EF (binary file) list
 * @note Only FID values are returned, other file attributes are discarded
 * @note INS=0x4A, P1=0x01 (EF), P2=0x00
 */
int hsm_crt_list_query(uint8_t *out_buf, uint32_t *out_len);

/**
 * @brief Import data to a binary file (create if not exists, then write)
 * @param fid: File ID (2 bytes)
 * @param data: Input data buffer
 * @param data_len: Input data length (max HSM_FILE_MAX_CONTENT_LEN)
 * @return 0 on success, negative error code on failure
 * @note Default file type is HSM_FILE_TYPE_BIN
 * @note File format: [data_len(2 bytes, big-endian)][data...]
 * @note File name is automatically generated as crt_{FID}.bin (e.g., crt_0003.bin)
 * @note File size, read/write attr and key ID all use macro defaults
 * @note This function combines create and write operations
 * @note If file already exists, it will be deleted and recreated
 * @note offset=0, encrypt=false
 */
int hsm_crt_import(uint16_t fid, const uint8_t *data, uint32_t data_len);




#ifdef __cplusplus
}
#endif

#endif /* _HSM_FILE_UTIL_H */
