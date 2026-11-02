#ifndef _AUDIO_CONFIG_FILE_H_
#define _AUDIO_CONFIG_FILE_H_

#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <sys/stat.h>
#include <fstream>
#include <functional>
#include <condition_variable>
#include <json/json.h>

#define LOG_INFO    LOG_Debug
#define LOG_ERROR   LOG_Error

class JsonFileHandler {
public:
    explicit JsonFileHandler(const std::string& file);
    ~JsonFileHandler();

    // 禁止拷贝和赋值
    JsonFileHandler(const JsonFileHandler&) = delete;
    JsonFileHandler& operator=(const JsonFileHandler&) = delete;

    // 同步读取
    Json::Value get_data() const;

    // 完全替换
    void set_data(const Json::Value& new_data);

    // 增量更新（RFC 7386 mergePatch）
    void update_value(const Json::Value& patch);

    // 强制立即写入
    void flush();

    // 重新加载
    void reload();

private:
    // 主互斥锁 - 保护所有数据访问
    mutable std::mutex mtx;
    std::string filepath;
    Json::Value data;
    std::atomic<bool> dirty{false};
    
    // 保存相关
    std::thread save_worker_thread;
    std::condition_variable save_cv;
    std::atomic<bool> save_pending{false};
    std::atomic<bool> worker_running{false};
    std::chrono::milliseconds write_delay{100};

    // 工具函数
    bool file_exists(const std::string& path);
    void load_from_file();
    void save_to_file();
    
    // 工作线程函数
    void start_save_worker();
    void save_worker_func();
    
    // RFC 7386 mergePatch 实现
    void json_merge_patch(Json::Value& target, const Json::Value& patch);
};
#endif