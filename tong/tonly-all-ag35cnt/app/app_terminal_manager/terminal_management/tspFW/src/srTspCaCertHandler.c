#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include "srTspFWLog.h"
#include "srTspFWInterface.h"

#define MAX_PATH_LEN 256
#define SR_CA_CRT_FILENAME "ca.crt"
#define SR_CLIENT_CRT_FILENAME "client.crt"
#define SR_CLIENT_KEY_FILENAME "client.key"
static char g_srCaDir[MAX_PATH_LEN] = {0};

static const char *srCaCrt =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDCzCCAfOgAwIBAgIUE65UJCeUx0A2zIobOBSfu0D1ViwwDQYJKoZIhvcNAQEL\n"
    "BQAwFTETMBEGA1UEAwwKRU1RLVJvb3RDQTAeFw0yNjA0MjEwODMwNTJaFw0zNjA0\n"
    "MTgwODMwNTJaMBUxEzARBgNVBAMMCkVNUS1Sb290Q0EwggEiMA0GCSqGSIb3DQEB\n"
    "AQUAA4IBDwAwggEKAoIBAQCsrZbT0lOPRXUQj0ebS3fJETP2VHxCyMs+AHCQugLG\n"
    "3qHaZc62XzzvYM9iG+l2kxajS/4Yyvc8feNwBdfE5ZuTZzyPFfk/IE7IJmZgCq0M\n"
    "2dFtqsdIZz1I+/sXzs4UFqyglIsehfbgpdNWu3dMUyhUtFpHZTz9G6GmKlJqGEzo\n"
    "PGs6p33ihzNXpKKPrw+qIKlDs9e4CiBJ7THIOv+fe9vW/q4xZu1KfxcGw6+mcpnM\n"
    "uQ3qShk2oAcB3uRXy3muvrwPINgkE47JqLaySzPX+Czo2Mq1AZE0tU2AAK0tZGYy\n"
    "WSRSGoqTUvc3Nmz0W9UPAqnTsObDGGT44CKrstVaV1UHAgMBAAGjUzBRMB0GA1Ud\n"
    "DgQWBBQ6BYWsDOaPyyRndvIZJ2ffRJD+WDAfBgNVHSMEGDAWgBQ6BYWsDOaPyyRn\n"
    "dvIZJ2ffRJD+WDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQA1\n"
    "MTp+JglVzWzzsTSRr6jUUXF8tS8IaR5mJ5V6T8zcOTdtiJt2qeuOz9ix0ykuC3aS\n"
    "IucO4UuLsC6WTQBRSubmD1Qc+qNPu6bb91A0UIp4lETypn1wjzliAE3Z4h8FmQuH\n"
    "kHkbom4MegNNVewSJZwrE2XP57z++svuAIxRu2Yp44bXGTod6jiQlFNxHZ8g0TQR\n"
    "jYPXJC8r3744Vo7SkvA9jA5gQfJt82x1pUZhB83vxcQNpJ09FaHdz0syB96vZJkF\n"
    "3Dl+BXmMYhxCaPFvAPJHvXiO+eL0g1nymJNCRVaKPRnfs0Vxc1qRmZt8BhXuJIIY\n"
    "2ZfsD+/zh/t04OPLlZ+w\n"
    "-----END CERTIFICATE-----\n";
static const char *srClientCrt =
    "-----BEGIN CERTIFICATE-----\n"
    "MIICszCCAZsCFCDzJLyWQLiulRl/YBM4qbMyoPicMA0GCSqGSIb3DQEBCwUAMBUx\n"
    "EzARBgNVBAMMCkVNUS1Sb290Q0EwHhcNMjYwNDIxMDg0NzQ3WhcNMzYwNDE4MDg0\n"
    "NzQ3WjAXMRUwEwYDVQQDDAxNUVRUWC1DbGllbnQwggEiMA0GCSqGSIb3DQEBAQUA\n"
    "A4IBDwAwggEKAoIBAQC7kEgRtjcK5P3DHqjhVft7sO/lEiVLgXQDAt8Mpsst+mjg\n"
    "sh1Ew3Hj8qKnUr/6FpgYYsVOFI/b5sLJmlItbYgSnLU3PRnbNK/oULfpGJM3rZT9\n"
    "qNqJiAo1+IAB/uNFzBGpXFgB7Sz3xNrHODxV0xhKqvQjEGhfRB/mReghWGqYh0Au\n"
    "wDtUnIB2CTltjYLkiVQ6FHTvN2yJgS0BeQbcq/ClHUnaKzvSeHk3uZBVn9tJ9Ht2\n"
    "gGVtdJ2nJhtfGlINxf4/1fhqRAD/yP3SeOsJ47gvmdHftCG47YtonUYHE3qjxNLY\n"
    "JN/l0UT1T/Y0GUxMy4N4Ek4yktlySr9HxzutASHNAgMBAAEwDQYJKoZIhvcNAQEL\n"
    "BQADggEBABr5qXtCc5zEQvUFXOU2tMdYCnxx6WtE3IzFreUdfPsyN9j38GrQaJ2/\n"
    "5X/uRFAIZh71wFz4PDs5TrbCh0mN0cJuU2Qq++XFBP7libuDP0/6Py4xOnnZFuBS\n"
    "asLAjSe4PGuWRcv0EML4EQ/gtJEvzMN5LcxoP1XI78N4IywqoFPElrNDHoM6Zggc\n"
    "WZQNYok4zJ929ZOCROtDZRm67WLm8qs2E7uYhumqIqUSiF58hOKzF4WRRI0l3y7N\n"
    "hjRXX4fFA8U37ZRc5SdUM3Rx1vx/TKRwiP65GJK1C2umapxJtj4OMhBXR9h0LHBZ\n"
    "8rOxqC8ZpsIa9EebhFAhG1nAgh+uavY=\n"
    "-----END CERTIFICATE-----\n";
static const char *srClientkey =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIIEowIBAAKCAQEAu5BIEbY3CuT9wx6o4VX7e7Dv5RIlS4F0AwLfDKbLLfpo4LId\n"
    "RMNx4/Kip1K/+haYGGLFThSP2+bCyZpSLW2IEpy1Nz0Z2zSv6FC36RiTN62U/aja\n"
    "iYgKNfiAAf7jRcwRqVxYAe0s98Taxzg8VdMYSqr0IxBoX0Qf5kXoIVhqmIdALsA7\n"
    "VJyAdgk5bY2C5IlUOhR07zdsiYEtAXkG3KvwpR1J2is70nh5N7mQVZ/bSfR7doBl\n"
    "bXSdpyYbXxpSDcX+P9X4akQA/8j90njrCeO4L5nR37QhuO2LaJ1GBxN6o8TS2CTf\n"
    "5dFE9U/2NBlMTMuDeBJOMpLZckq/R8c7rQEhzQIDAQABAoIBAEGkRRStyKWvboAT\n"
    "AteZ1+F6EvsAVr0OGSPUpWswFHuK35mDvGqMXBeCmxokA7vqnGAscRDacV5sbE9S\n"
    "CuCLLMwWu/URFTN6RPXxCewyBZO+1B3SMmlfivNIfMQ2CCQtPyEeiE5kSxCLfGWs\n"
    "cLE4p4+6NKbw3xY4KVHVtV+2KdgnItUJpugWnjjlHrALA5oO7L2pIfWK4HKy/3OM\n"
    "VDFX9R7v9HYyokM0buRFD/BEE+O6Ir8gutluJiHoD8pGthGKkd0Pm2ZvtTdUA4Ew\n"
    "I1805ellE1CR9TEswoQVgl5mdllPbDeleu7qkBnWhaiHAC8Ut3qTR44/Hw60JRMd\n"
    "x+gGqwECgYEA+QUVtGmexITP7p7WrgIgQI2m7ZFZ56u6cqNB7I+p1TD4jX4ITCR9\n"
    "rGriXZhLpZdgo2VmxIFW1gd2RbJIMHN0y8LV3QtP2lFB7unaIn3CmqlPUSL6fqRM\n"
    "ayiUYo7oMLJr8+uQrsOUYhFyCSnhiluMbSWyfBcWlNrt2dH7LpTwwKUCgYEAwNIz\n"
    "Aqg6eLX8ZldP1liD95WuoHj0weE4X2VRABMwpT5VRc0C9u+2jrwCdTPdIyCEl5V7\n"
    "M99s/JLCek2m/9j+NP4Zo6b6RX+L2S+n7ffP+yqOC1g2ff1NaxE+1pFXFANR2CX5\n"
    "gm38P/j0hb1f7leFMovCG30Byhtj0aVEvXhWLAkCgYEA76J4B5yYf677Vjsehmdf\n"
    "WB+4oud8KBg3HTiEKTz7B0DP/4FoRLXdBn35Bm6XZWvuQG9FXRbGVMLrixiMpauF\n"
    "mswM+GF5X7NQirQaCZUSH7mMrZzGh/Zd38U2Pj4drTcV8a4gFj353gHBxpW6CGYf\n"
    "Fa3wO8AQKAEiYLnUSThlEk0CgYALHNIlyA4+oLZgK44FW2qXnT/ILdBlUBaQoOUv\n"
    "LOvzygjeEfemxZ/uz/UQTiHjgV9C4TGNhsXgNJESvWDvg9D6ut49aUUpNjWYGcAC\n"
    "3fJVlllDXB0f27ZXglHtyciL9bkEzQN5Od98zFtDTeu2b8LUa+uO7ABrZnK1RWIV\n"
    "zVaiCQKBgDidjY0BeXIAMm3jAw/y3aHegfUc5ihXgKelqgZV6CXE+W9Z1s+inYR3\n"
    "Mqrfl/tMrp6UkTVWYtKt6nhUkC93BNzHS00clSBuyKacbrTM17ST9Gm1EUVQoGI3\n"
    "CqITjyYXKkyGJ4V/gZg4186Rqt5Q4bra2/YRAAy+hyBxSQ1WSCLI\n"
    "-----END RSA PRIVATE KEY-----\n";

int32_t SRTspGetCaPath(char *buffer, size_t len)
{
    if (!buffer || len == 0)
    {
        SR_TSP_LOG_E("Invalid buffer for CA full path");
        return RET_ERR;
    }

    if (g_srCaDir[0] == '\0')
    {
        SR_TSP_LOG_W("srCa directory path is not initialized");
        return RET_ERR;
    }

    int neededLen = snprintf(NULL, 0, "%s/%s", g_srCaDir, SR_CA_CRT_FILENAME) + 1;
    if ((size_t)neededLen > len)
    {
        SR_TSP_LOG_E("Buffer too small for CA full path, need %d bytes", neededLen);
        return RET_ERR;
    }

    snprintf(buffer, len, "%s/%s", g_srCaDir, SR_CA_CRT_FILENAME);
    return RET_OK;
}

int32_t SRTspGetClientRetPath(char *buffer, size_t len)
{
    if (!buffer || len == 0)
    {
        SR_TSP_LOG_E("Invalid buffer for CA full path");
        return RET_ERR;
    }

    if (g_srCaDir[0] == '\0')
    {
        SR_TSP_LOG_W("srCa directory path is not initialized");
        return RET_ERR;
    }

    int neededLen = snprintf(NULL, 0, "%s/%s", g_srCaDir, SR_CLIENT_CRT_FILENAME) + 1;
    if ((size_t)neededLen > len)
    {
        SR_TSP_LOG_E("Buffer too small for CA full path, need %d bytes", neededLen);
        return RET_ERR;
    }

    snprintf(buffer, len, "%s/%s", g_srCaDir, SR_CLIENT_CRT_FILENAME);
    return RET_OK;
}

int32_t SRTspGetClientKeyPath(char *buffer, size_t len)
{
    if (!buffer || len == 0)
    {
        SR_TSP_LOG_E("Invalid buffer for CA full path");
        return RET_ERR;
    }

    if (g_srCaDir[0] == '\0')
    {
        SR_TSP_LOG_W("srCa directory path is not initialized");
        return RET_ERR;
    }

    int neededLen = snprintf(NULL, 0, "%s/%s", g_srCaDir, SR_CLIENT_KEY_FILENAME) + 1;
    if ((size_t)neededLen > len)
    {
        SR_TSP_LOG_E("Buffer too small for CA full path, need %d bytes", neededLen);
        return RET_ERR;
    }

    snprintf(buffer, len, "%s/%s", g_srCaDir, SR_CLIENT_KEY_FILENAME);
    return RET_OK;
}

int32_t SRTspWriteCaCrt(const char *dirPath)
{
    if (!dirPath || strlen(dirPath) == 0)
    {
        SR_TSP_LOG_E("Invalid directory path");
        return RET_ERR;
    }

    struct stat st;
    if (stat(dirPath, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        SR_TSP_LOG_E("Invalid or non-directory path: %s", strerror(errno));
        return RET_ERR;
    }

    char filePath[512] = {0};
    snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, SR_CA_CRT_FILENAME);

    if (access(filePath, F_OK) == 0)
    {
        SR_TSP_LOG_I("Certificate already exists: %s, skip writing", filePath);
        strncpy(g_srCaDir, dirPath, MAX_PATH_LEN - 1);
        return RET_OK;
    }

    FILE *fp = fopen(filePath, "w");
    if (!fp)
    {
        SR_TSP_LOG_E("Failed to open file: %s", strerror(errno));
        return RET_ERR;
    }

    size_t len = strlen(srCaCrt);
    size_t written = fwrite(srCaCrt, 1, len, fp);
    fclose(fp);

    if (written != len)
    {
        SR_TSP_LOG_E("Failed to write full content to: %s", filePath);
        return RET_ERR;
    }
    strncpy(g_srCaDir, dirPath, MAX_PATH_LEN - 1);
    SR_TSP_LOG_I("Certificate written successfully to: %s", filePath);
    return RET_OK;
}

int32_t SRTspWriteClientCrt(const char *dirPath)
{
    if (!dirPath || strlen(dirPath) == 0)
    {
        SR_TSP_LOG_E("Invalid directory path");
        return RET_ERR;
    }

    struct stat st;
    if (stat(dirPath, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        SR_TSP_LOG_E("Invalid or non-directory path: %s", strerror(errno));
        return RET_ERR;
    }

    char filePath[512] = {0};
    snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, SR_CLIENT_CRT_FILENAME);

    if (access(filePath, F_OK) == 0)
    {
        SR_TSP_LOG_I("Certificate already exists: %s, skip writing", filePath);
        strncpy(g_srCaDir, dirPath, MAX_PATH_LEN - 1);
        return RET_OK;
    }

    FILE *fp = fopen(filePath, "w");
    if (!fp)
    {
        SR_TSP_LOG_E("Failed to open file: %s", strerror(errno));
        return RET_ERR;
    }

    size_t len = strlen(srClientCrt);
    size_t written = fwrite(srClientCrt, 1, len, fp);
    fclose(fp);

    if (written != len)
    {
        SR_TSP_LOG_E("Failed to write full content to: %s", filePath);
        return RET_ERR;
    }
    strncpy(g_srCaDir, dirPath, MAX_PATH_LEN - 1);
    SR_TSP_LOG_I("Certificate written successfully to: %s", filePath);
    return RET_OK;
}

int32_t SRTspWriteClientKey(const char *dirPath)
{
    if (!dirPath || strlen(dirPath) == 0)
    {
        SR_TSP_LOG_E("Invalid directory path");
        return RET_ERR;
    }

    struct stat st;
    if (stat(dirPath, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        SR_TSP_LOG_E("Invalid or non-directory path: %s", strerror(errno));
        return RET_ERR;
    }

    char filePath[512] = {0};
    snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, SR_CLIENT_KEY_FILENAME);

    if (access(filePath, F_OK) == 0)
    {
        SR_TSP_LOG_I("Certificate already exists: %s, skip writing", filePath);
        strncpy(g_srCaDir, dirPath, MAX_PATH_LEN - 1);
        return RET_OK;
    }

    FILE *fp = fopen(filePath, "w");
    if (!fp)
    {
        SR_TSP_LOG_E("Failed to open file: %s", strerror(errno));
        return RET_ERR;
    }

    size_t len = strlen(srClientkey);
    size_t written = fwrite(srClientkey, 1, len, fp);
    fclose(fp);

    if (written != len)
    {
        SR_TSP_LOG_E("Failed to write full content to: %s", filePath);
        return RET_ERR;
    }
    strncpy(g_srCaDir, dirPath, MAX_PATH_LEN - 1);
    SR_TSP_LOG_I("Certificate written successfully to: %s", filePath);
    return RET_OK;
}
