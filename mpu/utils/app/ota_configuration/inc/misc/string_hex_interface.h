#ifndef _STRING_HEX_INTERFACE_H_
#define _STRING_HEX_INTERFACE_H_

int FindHexSequence(unsigned char *data, size_t data_length, unsigned char *seq, size_t seq_length);
void HexToChars(unsigned char *hex, size_t length,char *output);
void StringToHexArray(const char *str, unsigned char *hex_array);
int AsciiToHex(const char *asciiString, char *hexString);
int HexToAscii(const char *hex_str, char *ascii_str,int input_len,int *out_len);
char *SiRunStrStr(const char *haystack, const char *needle);
int FindHexPairIndices(unsigned char *array, int length, unsigned char target1, unsigned char target2, int *indices);
#endif