#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "include/sndfile.h"
#include "include/mpg123.h"

// -------------------------- WAV格式解析相关 --------------------------
// WAV文件头结构体（简化版，仅保留关键字段）
typedef struct {
    // RIFF Chunk
    char riff_id[4];         // "RIFF"
    unsigned int riff_size;  // 文件总大小 - 8
    char riff_type[4];       // "WAVE"
    // Format Chunk
    char fmt_id[4];          // "fmt "
    unsigned int fmt_size;   // fmt块大小（通常16）
    unsigned short audio_fmt;// 音频格式（1=PCM）
    unsigned short channels; // 声道数
    unsigned int sample_rate;// 采样率
    unsigned int byte_rate;  // 每秒字节数（采样率×声道数×位深/8）
    unsigned short block_align;// 块对齐（声道数×位深/8）
    unsigned short bit_depth;// 位深（8/16/24/32）
    // Data Chunk
    char data_id[4];         // "data"
    unsigned int data_size;  // 音频数据总字节数
} WavHeader;

/**
 * @brief 解析WAV文件时长
 * @param file_path WAV文件路径
 * @return 时长（秒，浮点型），失败返回-1.0
 */
double get_wav_duration(const char *file_path) {
    SF_INFO sfinfo;
    // 初始化SF_INFO结构体
    memset(&sfinfo, 0, sizeof(SF_INFO));
    
    // 打开音频文件，自动解析所有WAV类型（PCM/压缩/扩展均支持）
    SNDFILE *sndfile = sf_open(file_path, SFM_READ, &sfinfo);
    if (!sndfile) {
        fprintf(stderr, "Failed to open WAV file: %s\n", sf_strerror(NULL));
        return -1.0;
    }

    // 核心：时长 = 总采样帧数 / 采样率（libsndfile自动处理所有编码的采样数计算）
    double duration = (double)sfinfo.frames / sfinfo.samplerate;

    // 关闭文件
    sf_close(sndfile);
    return duration;
}

// -------------------------- PCM格式计算相关 --------------------------
/**
 * @brief 计算PCM文件时长（需指定采样率/声道数/位深）
 * @param file_path PCM文件路径
 * @param sample_rate 采样率（如44100、16000）
 * @param channels 声道数（1=单声道，2=立体声）
 * @param bit_depth 位深（8/16/24/32）
 * @return 时长（秒，浮点型），失败返回-1.0
 */
double get_pcm_duration(const char *file_path, int sample_rate, int channels, int bit_depth) {
    // 验证参数合法性
    if (sample_rate <= 0 || channels <= 0 || bit_depth <= 0 || bit_depth % 8 != 0) {
        fprintf(stderr, "Invalid PCM parameters\n");
        return -1.0;
    }

    // 获取PCM文件总大小
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        perror("Failed to get PCM file size");
        return -1.0;
    }
    off_t file_size = file_stat.st_size;
    if (file_size == 0) {
        fprintf(stderr, "PCM file is empty\n");
        return -1.0;
    }

    // 计算每秒字节数
    double bytes_per_second = (double)sample_rate * channels * (bit_depth / 8);
    // 计算时长
    double duration = (double)file_size / bytes_per_second;
    return duration;
}

// -------------------------- MP3格式解析相关 --------------------------
/**
 * @brief 解析MP3文件时长（基于libmpg123）
 * @param file_path MP3文件路径
 * @return 时长（秒，浮点型），失败返回-1.0
 */
double get_mp3_duration(const char *file_path) {
    // 初始化libmpg123
    mpg123_init();
    int err;
    mpg123_handle *mh = mpg123_new(NULL, &err);
    if (!mh) {
        // fprintf(stderr, "Failed to create mpg123 handle: %s\n", mpg123_strerror(NULL, err));
        return -1.0;
    }

    // 打开MP3文件
    if (mpg123_open(mh, file_path) != MPG123_OK) {
        fprintf(stderr, "Failed to open MP3 file: %s\n", mpg123_strerror(mh));
        mpg123_delete(mh);
        mpg123_exit();
        return -1.0;
    }

    // 获取MP3文件总时长（毫秒）
    long rate;
    int channels, encoding;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
        fprintf(stderr, "Failed to get MP3 format\n");
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return -1.0;
    }

    off_t total_samples = mpg123_length(mh); // 总采样数
    if (total_samples == -1) {
        fprintf(stderr, "Failed to get MP3 total samples\n");
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return -1.0;
    }

    // 时长 = 总采样数 / 采样率
    double duration = (double)total_samples / rate;

    // 清理资源
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    return duration;
}

// // -------------------------- 主函数（测试） --------------------------
// int main(int argc, char *argv[]) {
//     if (argc < 2) {
//         fprintf(stderr, "Usage:\n");
//         fprintf(stderr, "  %s <audio_file> [pcm_sample_rate pcm_channels pcm_bit_depth]\n", argv[0]);
//         fprintf(stderr, "  Example:\n");
//         fprintf(stderr, "    %s test.wav\n", argv[0]);
//         fprintf(stderr, "    %s test.mp3\n", argv[0]);
//         fprintf(stderr, "    %s test.pcm 16000 1 16\n", argv[0]);
//         return 1;
//     }

//     const char *file_path = argv[1];
//     char *ext = strrchr(file_path, '.');
//     if (!ext) {
//         fprintf(stderr, "Cannot get file extension\n");
//         return 1;
//     }

//     double duration = -1.0;
//     // 转换后缀为小写，兼容大小写（如.MP3/.mp3）
//     if (strcasecmp(ext, ".wav") == 0) {
//         duration = get_wav_duration(file_path);
//     } else if (strcasecmp(ext, ".mp3") == 0) {
//         duration = get_mp3_duration(file_path);
//     } else if (strcasecmp(ext, ".pcm") == 0) {
//         if (argc < 5) {
//             fprintf(stderr, "PCM need parameters: sample_rate channels bit_depth\n");
//             return 1;
//         }
//         int sample_rate = atoi(argv[2]);
//         int channels = atoi(argv[3]);
//         int bit_depth = atoi(argv[4]);
//         duration = get_pcm_duration(file_path, sample_rate, channels, bit_depth);
//     } else {
//         fprintf(stderr, "Unsupported format: %s\n", ext);
//         return 1;
//     }

//     if (duration >= 0) {
//         // 转换为 分:秒 格式（可选）
//         int minutes = (int)duration / 60;
//         double seconds = duration - minutes * 60;
//         printf("Audio duration: %.2f seconds (%.0d:%.2f)\n", duration, minutes, seconds);
//     } else {
//         fprintf(stderr, "Failed to get audio duration\n");
//         return 1;
//     }

//     return 0;
// }