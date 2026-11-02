
#include <glib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>  
#include <ctype.h>

int FindHexSequence(unsigned char *data, size_t data_length, unsigned char *seq, size_t seq_length) 
{
    for (size_t i = 0; i <= data_length - seq_length; i++) {
        if (memcmp(&data[i], seq, seq_length) == 0) {
            printf("Found sequence at index: %zu\n", i);
            return i;
        }
    }
    return -1;
}

void HexToChars(unsigned char *hex, size_t length,char *output)
{
    size_t index = 0;

    for (size_t i = 0; i < length; i++) {
        // 检查字符是否为可打印字符
        if (isprint(hex[i])) {
            output[index++] = hex[i]; // 可打印字符直接存储
        } else {
            output[index++] = '.'; // 替换不可打印字符为 '.'
        }
    }
    output[index] = '\0'; // 以空字符结尾

    printf("Character representation: %s\n", output);
}


void StringToHexArray(const char *str, unsigned char *hex_array) 
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        // 将每个字符转为十六进制数并存储在数组中
        hex_array[i] = (unsigned char)str[i];
    }

}


int AsciiToHex(const char *asciiString, char *hexString) 
{
    if((!asciiString) || (!hexString))
    {
        return -1;
    }
    // 遍历每个字符，将其 ASCII 值转换为十六进制字符串
    for (size_t i = 0; i < strlen(asciiString); i++) {
        // 将每个字符转换为两位的十六进制表示
        sprintf(hexString + (i * 2), "%02X", (unsigned char)asciiString[i]);
    }
    // 在字符串末尾添加一个空字符
    hexString[strlen(asciiString) * 2] = '\0';
    return 0;
}

int HexToAscii(const char *hex_str, char *ascii_str,int input_len,int *out_len) 
{
    int i;
    if((!hex_str) || (!ascii_str))
    {
        return -1;
    }

    for (i = 0; i < input_len / 2; i++) {
        // 从十六进制字符串中提取每两个字符，并将它们转换为十进制整数
        sscanf(hex_str + 2 * i, "%2hhx", &ascii_str[i]);
    }
    *out_len = i - 1;
    ascii_str[i] = '\0'; // 添加字符串结束符
    return 0;
}
// 自定义函数：不区分大小写地查找子串
char *SiRunStrStr(const char *haystack, const char *needle) 
{
    
    
    if ((!needle) || (!haystack)) 
    {
        printf("parameter NULL\n");
        return NULL; // 如果子串为空，则返回母串的起始地址
    }

    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;

        // 比较当前字符，不区分大小写
        while (*h && *n && (tolower((unsigned char)*h) == tolower((unsigned char)*n))) {
            h++;
            n++;
        }

        // 如果到达子串的末尾，说明找到
        if (!*n) {
            return (char *)haystack; // 返回母串中找到的地址
        }

        haystack++;
    }

    return NULL; // 如果未找到，返回 NULL
}


// 函数定义
int FindHexPairIndices(unsigned char *array, int length, unsigned char target1, unsigned char target2, int *indices) 
{
    int count = 0;

    // 遍历数组
    for (int i = 0; i < length; i++) {
        // 检查当前元素和下一个元素
        if (array[i] == target1 && (i + 1 < length && array[i + 1] == target2)) {
            // 存储下标
            indices[count] = i;
            count++;
            i++; // 跳过下一个元素，避免重复计数
        }
    }

    return count; // 返回找到的组合次数
}