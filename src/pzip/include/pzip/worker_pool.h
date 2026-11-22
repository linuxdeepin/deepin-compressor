// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common.h"
#include <queue>
#include <thread>
#include <future>

namespace pzip {

/**
 * @brief 通用线程池模板类
 * 
 * 类似于 Go 版 pzip 的 FileWorkerPool，支持任务入队和并行处理
 * 
 * @tparam T 任务类型
 */
template<typename T>
class WorkerPool {
public:
    using Executor = std::function<Error(T*)>;

    /**
     * @brief 构造函数
     * @param executor 任务执行函数
     * @param concurrency 并发数（0 表示使用 CPU 核心数）
     * @param capacity 队列容量
     */
    WorkerPool(Executor executor, size_t concurrency = 0, size_t capacity = 1);
    
    ~WorkerPool();

    // 禁止拷贝
    WorkerPool(const WorkerPool&) = delete;
    WorkerPool& operator=(const WorkerPool&) = delete;

    /**
     * @brief 启动工作线程
     */
    void start();

    /**
     * @brief 入队任务
     * @param task 任务指针
     */
    void enqueue(T* task);

    /**
     * @brief 关闭线程池，等待所有任务完成
     * @return 第一个遇到的错误（如果有）
     */
    Error close();

    /**
     * @brief 取消所有待处理任务
     */
    void cancel();

    /**
     * @brief 获取待处理任务数量
     */
    size_t pendingTasks() const;

    /**
     * @brief 是否正在运行
     */
    bool isRunning() const { return running_; }

private:
    void workerThread();

    Executor executor_;
    size_t concurrency_;
    size_t capacity_;

    std::vector<std::thread> workers_;
    std::queue<T*> tasks_;
    
    mutable std::mutex mutex_;
    std::condition_variable taskAvailable_;
    std::condition_variable spaceAvailable_;
    
    std::atomic<bool> running_{false};
    std::atomic<bool> shutdown_{false};
    std::atomic<bool> cancelled_{false};
    
    Error firstError_;
    std::mutex errorMutex_;
};

// ============================================================================
// 模板实现
// ============================================================================

template<typename T>
WorkerPool<T>::WorkerPool(Executor executor, size_t concurrency, size_t capacity)
    : executor_(std::move(executor))
    , concurrency_(concurrency == 0 ? std::thread::hardware_concurrency() : concurrency)
    , capacity_(capacity)
{
}

template<typename T>
WorkerPool<T>::~WorkerPool() {
    if (running_) {
        cancel();
        close();
    }
}

template<typename T>
void WorkerPool<T>::start() {
    if (running_) return;
    
    running_ = true;
    shutdown_ = false;
    cancelled_ = false;
    firstError_ = Error();
    
    workers_.reserve(concurrency_);
    for (size_t i = 0; i < concurrency_; ++i) {
        workers_.emplace_back(&WorkerPool::workerThread, this);
    }
}

template<typename T>
void WorkerPool<T>::enqueue(T* task) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // 等待队列有空间
    spaceAvailable_.wait(lock, [this] {
        return tasks_.size() < capacity_ || shutdown_ || cancelled_;
    });
    
    if (shutdown_ || cancelled_) return;
    
    tasks_.push(task);
    taskAvailable_.notify_one();
}

template<typename T>
Error WorkerPool<T>::close() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
    }
    taskAvailable_.notify_all();
    spaceAvailable_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
    running_ = false;
    
    std::lock_guard<std::mutex> lock(errorMutex_);
    return firstError_;
}

template<typename T>
void WorkerPool<T>::cancel() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cancelled_ = true;
        
        // 清空队列
        while (!tasks_.empty()) {
            tasks_.pop();
        }
    }
    taskAvailable_.notify_all();
    spaceAvailable_.notify_all();
}

template<typename T>
size_t WorkerPool<T>::pendingTasks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tasks_.size();
}

template<typename T>
void WorkerPool<T>::workerThread() {
    while (true) {
        T* task = nullptr;
        
        {
            std::unique_lock<std::mutex> lock(mutex_);
            
            taskAvailable_.wait(lock, [this] {
                return !tasks_.empty() || shutdown_ || cancelled_;
            });
            
            if (cancelled_ || (shutdown_ && tasks_.empty())) {
                return;
            }
            
            if (!tasks_.empty()) {
                task = tasks_.front();
                tasks_.pop();
                spaceAvailable_.notify_one();
            }
        }
        
        if (task && !cancelled_) {
            Error err = executor_(task);
            
            if (err) {
                std::lock_guard<std::mutex> lock(errorMutex_);
                if (!firstError_) {
                    firstError_ = err;
                }
                cancelled_ = true;
                taskAvailable_.notify_all();
            }
        }
    }
}

} // namespace pzip

