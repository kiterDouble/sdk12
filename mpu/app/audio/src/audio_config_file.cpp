#include <iostream>
#include <unistd.h>
#include <cstdio> // for rename
#include <condition_variable>
#include "audio_config_file.h"
#include "sr_log.h"
#include "err_code.h"

bool JsonFileHandler::file_exists(const std::string& path) {
    return access(path.c_str(), F_OK) != -1;
}

void JsonFileHandler::load_from_file() {
    // 注意：调用者应持有mtx锁

    if (!file_exists(filepath)) {
        LOG_INFO("File not found: {}. Initializing empty JSON.", filepath);
        data = Json::objectValue;
        dirty.store(true, std::memory_order_release);
        return;
    }

    std::ifstream file(filepath);
    if (!file.is_open()) {
        LOG_INFO("open file error.", filepath);
        LOG_ERROR("Failed to open file for reading: {}", filepath);
        data = Json::objectValue;
        dirty.store(true, std::memory_order_release);
        return;
    }

    Json::CharReaderBuilder builder;
    std::string errs;
    if (!Json::parseFromStream(builder, file, &data, &errs)) {
        LOG_INFO("parseFromStream error.", filepath);
        LOG_ERROR("Failed to parse JSON from file: {}, error: {}", filepath, errs);
        data = Json::objectValue;
        dirty.store(true, std::memory_order_release);
        return;
    }

    LOG_INFO("JSON loaded from {}", filepath);
}

void JsonFileHandler::save_to_file() {
    // 注意：调用者应持有mtx锁
    
    if (!dirty.load(std::memory_order_acquire)) return;

    std::string temp_path = filepath + ".tmp";
    std::ofstream temp_file(temp_path);
    if (!temp_file.is_open()) {
        LOG_ERROR("Failed to create temp file: {}", temp_path);
        return;
    }

    try {
        Json::StreamWriterBuilder writer;
        writer["indentation"] = "    ";
        std::unique_ptr<Json::StreamWriter> json_writer(writer.newStreamWriter());
        json_writer->write(data, &temp_file);
        temp_file << std::endl;
        temp_file.close();

        if (rename(temp_path.c_str(), filepath.c_str()) != 0) {
            LOG_ERROR("Failed to rename {} to {}: {}", temp_path, filepath, std::strerror(errno));
            return;
        }

        chmod(filepath.c_str(), 0600);
        dirty.store(false, std::memory_order_release);
        LOG_INFO("JSON saved to {}", filepath);
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during file save: {}", e.what());
        // 注意：这里不会抛出异常，但调用者应该知道保存失败
    }
}

void JsonFileHandler::start_save_worker() {
    worker_running.store(true, std::memory_order_relaxed);
    save_worker_thread = std::thread(&JsonFileHandler::save_worker_func, this);
}

void JsonFileHandler::save_worker_func() {
    // 等待初始化完成（等待构造函数完全初始化）
    while (worker_running.load(std::memory_order_acquire)) {
        // 使用try_lock检查构造函数是否完成初始化
        std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
        if (lock.owns_lock()) {
            break; // 成功获取锁，说明构造函数已完成
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    while (worker_running.load(std::memory_order_acquire)) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // 等待条件：有脏数据或需要关闭
        save_cv.wait_for(lock, write_delay, [this]() {
            return dirty.load(std::memory_order_acquire) || 
                   !worker_running.load(std::memory_order_acquire);
        });
        
        if (!worker_running.load(std::memory_order_acquire)) break;
        
        // 如果数据是脏的，保存文件
        if (dirty.load(std::memory_order_acquire)) {
            try {
                save_to_file();
                // 保存成功后，reset save_pending
                save_pending.store(false, std::memory_order_release);
            } catch (const std::exception& e) {
                LOG_ERROR("Exception in save_worker_func: {}", e.what());
            }
        }
    }
}

JsonFileHandler::JsonFileHandler(const std::string& file)
    : filepath(file) {
    // 初始化数据
    data = Json::objectValue;
    
    // 加载文件
    std::lock_guard<std::mutex> lock(mtx);
    load_from_file();
    
    // 启动保存工作线程
    start_save_worker();
}

JsonFileHandler::~JsonFileHandler() {
    // 停止工作线程，使用release确保修改对其他线程可见
    worker_running.store(false, std::memory_order_release);
    save_cv.notify_all();
    
    // 等待工作线程结束
    if (save_worker_thread.joinable()) {
        save_worker_thread.join();
    }
    
    // 执行最后一次同步保存
    std::lock_guard<std::mutex> lock(mtx);
    if (dirty.load(std::memory_order_acquire)) {
        save_to_file();
    }
}

Json::Value JsonFileHandler::get_data() const {
    std::lock_guard<std::mutex> lock(mtx);
    return data;  // 返回拷贝，保证线程安全
}

void JsonFileHandler::set_data(const Json::Value& new_data) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (data != new_data) {
            data = new_data;
            dirty.store(true, std::memory_order_release);
            save_pending.store(true, std::memory_order_release);
        }
    }
    save_cv.notify_one();
}

void JsonFileHandler::json_merge_patch(Json::Value& target, const Json::Value& patch) {
    if (!patch.isObject()) {
        target = patch;
        return;
    }

    for (const auto& key : patch.getMemberNames()) {
        const Json::Value& value = patch[key];
        if (value.isNull()) {
            if (target.isObject()) {
                target.removeMember(key);
            }
        } else if (target.isObject() && target[key].isObject() && !value.isNull()) {
            json_merge_patch(target[key], value);
        } else {
            target[key] = value;
        }
    }
}

void JsonFileHandler::update_value(const Json::Value& patch) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        Json::Value old = data;
        json_merge_patch(data, patch);
        if (data != old) {
            dirty.store(true, std::memory_order_release);
            save_pending.store(true, std::memory_order_release);
        }
    }
    save_cv.notify_one();
}

void JsonFileHandler::flush() {
    std::lock_guard<std::mutex> lock(mtx);
    try {
        save_to_file();
        save_pending.store(false, std::memory_order_release);
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in flush: {}", e.what());
    }
}

void JsonFileHandler::reload() {
    std::lock_guard<std::mutex> lock(mtx);
    load_from_file();
    // 重新加载后，reset save_pending
    save_pending.store(false, std::memory_order_release);
}