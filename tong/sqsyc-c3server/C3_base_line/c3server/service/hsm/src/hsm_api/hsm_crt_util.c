/**
 * @file hsm_crt_util.c
 * @brief HSM file management utility implementation
 * @description File operation interfaces for SE chip, including create, delete, select, read, write operations
 * @note Based on CIU98_B V2 SE[3162] User Manual V1.2
 */

#include <string.h>
#include <stdio.h>

#include "hsm_crt_util.h"
#include "sr_log.h"

// HED SDK 头文件
#include "se.h"
#include "fs.h"
#include "comm.h"
#include "key.h"
#include "port_config.h"
#include "pin.h"
#include "crypto.h"
#include "apdu.h"
#include "tpdu.h"
#include "util.h"

#define HSM_FILE_MAX_NAME_LEN   32
#define HSM_FILE_MAX_DATA_LEN   240    // Maximum data length for single read/write


/**
 * @brief Build CREATEFILE command data
 * @note INS=0xE0, CLA=0x80, P1=0x00, P2=file_type
 *
 * Binary file format (matching createBF1 structure from personal() function):
 * FID(2) + FileAttr(1) + Size(3 bytes) + ReadAttr(1) + WriteAttr(1) + RT-KID(1) + WT-KID(1) + NameLen(1) + Name
 *
 * DDF/ADF format:
 * FID(2) + CreateRight(1) + SecFileWriteAttr(1) + WT-KID(1) + NameLen(1) + Name
 */
static int build_create_file_cmd(hsm_crt_type_t file_type, uint16_t fid, uint16_t size,
                                  uint8_t read_attr, uint8_t write_attr,
                                  uint8_t read_key_id, uint8_t write_key_id,
                                  const char *name, uint8_t name_len,
                                  uint8_t *cmd_buf, uint32_t *cmd_len)
{
    int ret = -1;
    uint32_t offset = 0;
    uint8_t lc = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        // APDU header: CLA=0x80, INS=0xE0, P1=0x00, P2=file_type
        cmd_buf[offset++] = 0x80;  // CLA
        cmd_buf[offset++] = 0xE0;  // INS (CREATEFILE)
        cmd_buf[offset++] = 0x00;  // P1
        cmd_buf[offset++] = file_type;  // P2

        if (file_type == HSM_FILE_TYPE_BIN) {
            // Binary file format (matching createBF1 structure):
            // FID(2) + FileAttr(1) + Size(3 bytes) + ReadAttr(1) + WriteAttr(1) + RT-KID(1) + WT-KID(1) + NameLen(1) + Name
            // Lc = 2 + 1 + 3 + 1 + 1 + 1 + 1 + 1 + name_len = 11 + name_len
            lc = 11 + name_len;
            cmd_buf[offset++] = lc;  // Lc
            
            // FID (2 bytes)
            cmd_buf[offset++] = (fid >> 8) & 0xFF;  // FID high
            cmd_buf[offset++] = fid & 0xFF;         // FID low
            
            // File attribute (1 byte) - using 0x10 as default
            cmd_buf[offset++] = 0x10;
            
            // Size (3 bytes) - matching createBF1 format
            // If size is 0, use default value
            if (size == 0) {
                size = HSM_FILE_DEFAULT_SIZE;
            }
            cmd_buf[offset++] = (size >> 16) & 0xFF;  // Size byte 2
            cmd_buf[offset++] = (size >> 8) & 0xFF;   // Size byte 1
            cmd_buf[offset++] = size & 0xFF;          // Size byte 0
            
            // Read attribute (1 byte)
            cmd_buf[offset++] = read_attr;
            
            // Write attribute (1 byte)
            cmd_buf[offset++] = write_attr;
            
            // RT-KID (1 byte)
            cmd_buf[offset++] = read_key_id;
            
            // WT-KID (1 byte)
            cmd_buf[offset++] = write_key_id;
            
            // Name length (1 byte)
            cmd_buf[offset++] = name_len;
            
            // Name
            if (name && name_len > 0) {
                memcpy(&cmd_buf[offset], name, name_len);
                offset += name_len;
            }
        } else {
            // DDF/ADF format:
            // FID(2) + CreateRight(1) + SecFileWriteAttr(1) + WT-KID(1) + NameLen(1) + Name
            // Lc = 2 + 1 + 1 + 1 + 1 + name_len = 6 + name_len
            lc = 6 + name_len;
            cmd_buf[offset++] = lc;  // Lc
            
            // FID (2 bytes)
            cmd_buf[offset++] = (fid >> 8) & 0xFF;  // FID high
            cmd_buf[offset++] = fid & 0xFF;         // FID low
            
            // CreateRight (1 byte) - default: no restriction
            cmd_buf[offset++] = 0x00;
            
            // SecurityFileWriteAttr (1 byte)
            cmd_buf[offset++] = 0x00;
            
            // WT-KID (1 byte)
            cmd_buf[offset++] = 0x00;
            
            // Name length (1 byte)
            cmd_buf[offset++] = name_len;
            
            // Name
            if (name && name_len > 0) {
                memcpy(&cmd_buf[offset], name, name_len);
                offset += name_len;
            }
        }

        *cmd_len = offset;
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

/**
 * @brief Build DELETEFILE command data
 * @note INS=0xE4, CLA=0x80
 *
 * P1 values (from PDF document):
 * - P1='00': Delete DDF/ADF by name (Lc=1-64, Data=DF name)
 * - P1='01': Delete EF by name (Lc=1-32, Data=EF name)
 * - P1='02': Delete file by FID (Lc=2, Data=FID)
 */
static int build_delete_file_cmd(hsm_crt_type_t file_type, uint16_t fid,
                                  uint8_t *cmd_buf, uint32_t *cmd_len)
{
    int ret = -1;
    uint32_t offset = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        (void)file_type;

        // APDU header: CLA=0x80, INS=0xE4, P1=0x02 (delete by FID), P2=0x00
        cmd_buf[offset++] = 0x80;  // CLA
        cmd_buf[offset++] = 0xE4;  // INS (DELETEFILE)
        cmd_buf[offset++] = 0x02;  // P1 (delete by FID)
        cmd_buf[offset++] = 0x00;  // P2
        cmd_buf[offset++] = 0x02;  // Lc (FID is 2 bytes)
        cmd_buf[offset++] = (fid >> 8) & 0xFF;  // FID high
        cmd_buf[offset++] = fid & 0xFF;         // FID low

        *cmd_len = offset;
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsm_crt_create(uint16_t fid, uint16_t size,
                    uint8_t read_attr, uint8_t write_attr,
                    uint8_t read_key_id, uint8_t write_key_id,
                    const char *name, uint8_t name_len)
{
    int ret = -1;
    uint8_t cmd_buf[64] = {0};
    uint32_t cmd_len = 0;
    uint8_t out_buf[16] = {0};
    uint32_t out_len = 0;
    uint16_t status_word = 0;
    se_error_t se_ret = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        LOG_Debug("[%s] fid=0x%04X, size=%d, name=%s",
                  __func__, fid, size, name ? name : "NULL");

        // Parameter validation
        if (name_len > HSM_FILE_MAX_NAME_LEN) {
            LOG_Error("[%s] Name length exceeds maximum (%d)", __func__, HSM_FILE_MAX_NAME_LEN);
            break;
        }

        // Build CREATEFILE command (default file type is BIN)
        build_create_file_cmd(HSM_FILE_TYPE_BIN, fid, size, read_attr, write_attr,
                              read_key_id, write_key_id, name, name_len,
                              cmd_buf, &cmd_len);

        // Send command using api_transceive
        se_ret = api_transceive(cmd_buf, cmd_len, out_buf, &out_len);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_transceive failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        // Check status word (last 2 bytes of response)
        if (out_len >= 2) {
            status_word = (out_buf[out_len - 2] << 8) | out_buf[out_len - 1];
            if (status_word != 0x9000) {
                LOG_Error("[%s] Status word error: 0x%04X", __func__, status_word);
                ret = status_word;
                break;
            }
        }

        LOG_Info("[%s] File created successfully: fid=0x%04X", __func__, fid);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int hsm_crt_delete(uint16_t fid)
{
    int ret = -1;
    uint8_t cmd_buf[16] = {0};
    uint32_t cmd_len = 0;
    uint8_t out_buf[16] = {0};
    uint32_t out_len = 0;
    uint16_t status_word = 0;
    se_error_t se_ret = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        LOG_Debug("[%s] fid=0x%04X", __func__, fid);

        // Build DELETEFILE command (default file type is BIN)
        build_delete_file_cmd(HSM_FILE_TYPE_BIN, fid, cmd_buf, &cmd_len);

        // Send command using api_transceive
        se_ret = api_transceive(cmd_buf, cmd_len, out_buf, &out_len);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_transceive failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        // Check status word
        if (out_len >= 2) {
            status_word = (out_buf[out_len - 2] << 8) | out_buf[out_len - 1];
            if (status_word != 0x9000) {
                LOG_Error("[%s] Status word error: 0x%04X", __func__, status_word);
                ret = status_word;
                break;
            }
        }

        LOG_Info("[%s] File deleted successfully: fid=0x%04X", __func__, fid);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}


int hsm_crt_export(uint16_t fid, uint32_t len, uint8_t *out_buf, uint32_t *out_len)
{
    int ret = -1;
    uint8_t len_buf[2] = {0};
    uint32_t chunk_out_len = 0;
    uint32_t data_len = 0;
    se_error_t se_ret = 0;
    uint32_t total_read = 0;
    int loop_ret = 0;
    uint32_t chunk_len = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        LOG_Debug("[%s] fid=0x%04X, max_len=%u", __func__, fid, len);

        if (out_buf == NULL || out_len == NULL || len == 0) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        // Step 1: Select file
        se_ret = api_select_file(fid);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_select_file failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        // Step 2: Read 2-byte length header
        se_ret = api_read_file(0, 2, false, len_buf, &chunk_out_len);
        if (se_ret != SE_SUCCESS || chunk_out_len < 2) {
            LOG_Error("[%s] Failed to read length header: ret=0x%08X, chunk_out=%u",
                      __func__, se_ret, chunk_out_len);
            ret = se_ret;
            break;
        }
        data_len = (len_buf[0] << 8) | len_buf[1];
        LOG_Debug("[%s] Length header: data_len=%u", __func__, data_len);

        if (data_len > len) {
            LOG_Error("[%s] Buffer too small: need %u, have %u", __func__, data_len, len);
            break;
        }

        if (data_len > HSM_FILE_MAX_CONTENT_LEN) {
            LOG_Error("[%s] Data length exceeds max: %u > %u", __func__, data_len, HSM_FILE_MAX_CONTENT_LEN);
            break;
        }

        // Step 3: Read actual data in chunks (offset=2, skip length header)
        total_read = 0;
        loop_ret = 0;
        while (total_read < data_len) {
            chunk_len = data_len - total_read;
            if (chunk_len > HSM_FILE_MAX_DATA_LEN) {
                chunk_len = HSM_FILE_MAX_DATA_LEN;
            }
            chunk_out_len = 0;
            se_ret = api_read_file(2 + total_read, chunk_len, false, out_buf + total_read, &chunk_out_len);
            if (se_ret != SE_SUCCESS) {
                LOG_Error("[%s] api_read_file failed at offset=%u: 0x%08X",
                          __func__, 2 + total_read, se_ret);
                loop_ret = se_ret;
                break;
            }
            total_read += chunk_out_len;
            if (chunk_out_len == 0) {
                break;  // No more data
            }
        }
        if (loop_ret != 0) {
            ret = loop_ret;
            break;
        }

        *out_len = total_read;

        LOG_Info("[%s] Data exported successfully: fid=0x%04X, data_len=%u, read=%u",
                 __func__, fid, data_len, *out_len);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int hsm_crt_list_query(uint8_t *out_buf, uint32_t *out_len)
{
    int ret = -1;
    se_error_t se_ret = SE_SUCCESS;
    uint8_t raw_buf[256] = {0};
    uint32_t raw_len = 0;
    uint32_t offset = 0;
    uint32_t fid_count = 0;
    uint16_t *fid_array = (uint16_t *)out_buf;
    uint8_t fid_len = 0;
    uint16_t fid = 0;
    uint8_t name_len = 0;
    uint8_t size_len = 0;
    uint8_t read_attr_len = 0;
    uint8_t write_attr_len = 0;
    uint8_t read_keyid_len = 0;
    uint8_t write_keyid_len = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (out_buf == NULL || out_len == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        se_ret = apdu_get_file_info(false, raw_buf, &raw_len);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] apdu_get_file_info failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        // Parse TLV format and extract only FID
        while (offset < raw_len) {
            fid_len = raw_buf[offset++];
            if (fid_len == 0) break; // padding

            if (offset + fid_len > raw_len) break;
            fid = 0;
            if (fid_len == 2) {
                fid = (raw_buf[offset] << 8) | raw_buf[offset + 1];
            } else if (fid_len == 1) {
                fid = raw_buf[offset];
            }
            offset += fid_len;

            // Store FID in output array
            if (fid_count < 128) { // max 128 FIDs to avoid buffer overflow
                fid_array[fid_count++] = fid;
            }

            // Skip remaining fields: Name_len + Name + Size_len + Size + ReadAttr + WriteAttr + ReadKeyID + WriteKeyID
            if (offset >= raw_len) break;
            name_len = raw_buf[offset++];
            offset += name_len;

            if (offset >= raw_len) break;
            size_len = raw_buf[offset++];
            offset += size_len;

            if (offset >= raw_len) break;
            read_attr_len = raw_buf[offset++];
            offset += read_attr_len;

            if (offset >= raw_len) break;
            write_attr_len = raw_buf[offset++];
            offset += write_attr_len;

            if (offset >= raw_len) break;
            read_keyid_len = raw_buf[offset++];
            offset += read_keyid_len;

            if (offset >= raw_len) break;
            write_keyid_len = raw_buf[offset++];
            offset += write_keyid_len;
        }

        *out_len = fid_count * sizeof(uint16_t);
        LOG_Info("[%s] Found %u files, total bytes=%u", __func__, fid_count, *out_len);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int hsm_crt_import(uint16_t fid,
                    const uint8_t *data, uint32_t data_len)
{
    int ret = -1;
    char name[16] = {0};
    uint8_t name_len = 0;
    uint16_t size = HSM_FILE_DEFAULT_SIZE;
    uint8_t read_attr = HSM_FILE_DEFAULT_READ_ATTR;
    uint8_t write_attr = HSM_FILE_DEFAULT_WRITE_ATTR;
    uint8_t read_key_id = HSM_FILE_DEFAULT_READ_KEY_ID;
    uint8_t write_key_id = HSM_FILE_DEFAULT_WRITE_KEY_ID;
    uint32_t total_write_len = 0;
    se_error_t se_ret = 0;
    uint8_t len_header[2] = {0};
    uint32_t write_offset = 0;
    int loop_ret = 0;
    uint32_t data_offset = 0;
    uint32_t chunk_len = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (data_len > HSM_FILE_MAX_CONTENT_LEN) {
            LOG_Error("[%s] Data too long: %u > max %u", __func__, data_len, HSM_FILE_MAX_CONTENT_LEN);
            break;
        }

        snprintf(name, sizeof(name), "crt_%04X.bin", fid);
        name_len = strlen(name);

        total_write_len = 2 + data_len;

        // Auto-adjust file size if total write length exceeds default
        // if (total_write_len > size) {
        //     size = (uint16_t)total_write_len;
        // }

        LOG_Debug("[%s] fid=0x%04X, size=%u, name=%s, data_len=%u, total_write=%u",
                  __func__, fid, size, name, data_len, total_write_len);

        // Step 2: Create file (default file type is BIN)
        ret = hsm_crt_create(fid, size,
                              read_attr, write_attr,
                              read_key_id, write_key_id,
                              name, name_len);
        if (ret != 0) {
            LOG_Error("[%s] Failed to create file: %d", __func__, ret);
            break;
        }

        // Step 3: Select file
        se_ret = api_select_file(fid);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_select_file failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        // Step 4: Write 2-byte length header + data in chunks
        // Length header: big-endian, data_len
        len_header[0] = (data_len >> 8) & 0xFF;
        len_header[1] = data_len & 0xFF;
        write_offset = 0;

        // Write length header first
        se_ret = api_write_file(0, len_header, 2, false);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_write_file failed at length header: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
        write_offset += 2;

        // Write actual data
        loop_ret = 0;
        if (data && data_len > 0) {
            data_offset = 0;
            while (data_offset < data_len) {
                chunk_len = data_len - data_offset;
                if (chunk_len > HSM_FILE_MAX_DATA_LEN) {
                    chunk_len = HSM_FILE_MAX_DATA_LEN;
                }

                se_ret = api_write_file(write_offset + data_offset, data + data_offset, chunk_len, false);
                if (se_ret != SE_SUCCESS) {
                    LOG_Error("[%s] api_write_file failed at offset=%u: 0x%08X",
                              __func__, write_offset + data_offset, se_ret);
                    loop_ret = se_ret;
                    break;
                }
                data_offset += chunk_len;
            }
        }
        if (loop_ret != 0) {
            ret = loop_ret;
            break;
        }

        LOG_Info("[%s] File imported successfully: fid=0x%04X, data_len=%u", __func__, fid, data_len);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}
