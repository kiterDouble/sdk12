#include <string.h>
#include "algorithm1.h"

/**
 * @brief 将key_raw转换为key_auth,作为SecOC软件模块的认证密钥
 *
 * @param[in]   	in   			key_raw数据
 * @param[in]   	vin   			VIN号
 * @param[out]   	out   			计算后输出的key_auth数据
 */
void SecOC_Algorithm1(const uint8 *in, const uint8 *vin, uint8 *out)
{
	if(in == NULL || vin == NULL || out == NULL) {
		return;
	}
	int	i = 0;
	int j = 0;
	int	i_list[10] = {6,9,7,4,8,2,0,5,1,3};
	int	*switch_i = &i_list[0];
	
	uint8	used_iv[16] = {0};
	uint8	used_k[16] = {0};
	uint8	used_sc[16] = {0};
    uint8	t = 0;
	
	while (1) {
		switch (*switch_i) {
			case 2:
				switch_i++;
			    for (i = 0; i < 16; i++) {
				    used_k[i] = used_iv[i] ^ used_sc[i];
				}
				break;
			case 3:
			    switch_i++;
			    for (i = 0; i < 16 - 1; i++) {
					if (used_sc[i] < used_sc[i + 1]) {
						t = used_sc[i];
						used_sc[i] = used_sc[i + 1];
						used_sc[i + 1] = t;
					}
				}
			    return;
			case 1:
		        switch_i++;
				for (i = 0; i < 16; i++) {
					used_k[i] ^= in[i];
				}
				for(i = 0; i < j; i++) {
					if (out[i] % 2 == 1) {
						used_sc[i] ^= used_k[i];
					}
				}
				break;
			case 7:
				switch_i++;
			    for (i = 0; i < 16 - 1; i++) {
					if (used_sc[i] > used_sc[i + 1]) {
						t = used_sc[i];
						used_sc[i] = used_sc[i + 1];
						used_sc[i + 1] = t;
					}
				}
				break;	
			case 5:
	            switch_i++;
				for(i = 0; i < j; i++) {
					if (used_iv[i] % 2 == 1) {
						out[i] = in[i] ^ used_k[i];
					} else {
						out[i] = in[i] ^ used_iv[i]; 
					}
				}
				break;
			case 9:
			    switch_i++;
			    for (i = 0; i < 16; i++) {
				    used_k[i] = in[i] ^ vin[16 - i];
				}
				break;
			case 4:
				switch_i++;
				for (i = 0; i < 16; i++) {
                    used_iv[i] = used_k[i] % (in[i] + i + 1);
				}
				break;
			case 0:
			    switch_i++;
				for (i = 0; i < 16 - 1; i++) {
				    if (used_k[i] & 0x1) {
						t = used_iv[i];
						used_iv[i] = used_iv[i + 1];
						used_iv[i + 1] = t;
					}
				}
				break;
			case 6:
				switch_i++;
				for (i = 0; i < 16; i++) {
					used_sc[i] ^= vin[i + 1] + i;
				}
				j = i;
				break;
			case 8:
			    switch_i++;
				for (i = 0; i < 16; i++) {
					used_sc[i] ^= in[i] + i;
				}
				break;
		}
	}
}
