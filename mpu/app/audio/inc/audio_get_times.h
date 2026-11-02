#ifndef  _AUDIO_GET_TIMES_H_
#define  _AUDIO_GET_TIMES_H_


/**
 * @brief 解析WAV文件时长
 * @param file_path WAV文件路径
 * @return 时长（秒，浮点型），失败返回-1.0
 */
double get_wav_duration(const char *file_path);

// -------------------------- PCM格式计算相关 --------------------------
/**
 * @brief 计算PCM文件时长（需指定采样率/声道数/位深）
 * @param file_path PCM文件路径
 * @param sample_rate 采样率（如44100、16000）
 * @param channels 声道数（1=单声道，2=立体声）
 * @param bit_depth 位深（8/16/24/32）
 * @return 时长（秒，浮点型），失败返回-1.0
 */
double get_pcm_duration(const char *file_path, int sample_rate, int channels, int bit_depth);

// -------------------------- MP3格式解析相关 --------------------------
/**
 * @brief 解析MP3文件时长（基于libmpg123）
 * @param file_path MP3文件路径
 * @return 时长（秒，浮点型），失败返回-1.0
 */
double get_mp3_duration(const char *file_path);

#endif