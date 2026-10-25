#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>

/**
 * @brief Thread-safe bounded queue for passing raw video frames from the capture 
 * thread to the inference worker thread. Implements producer-consumer flow.
 */
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    size_t max_size_;

public:
    explicit ThreadSafeQueue(size_t max_size = 30) : max_size_(max_size) {}

    // CUJ / Hot Path: Producer pushes a captured frame. Blocks if queue is full (backpressure control).
    bool push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        // Sad Path / Edge Case: Drop oldest frame if queue exceeds capacity to prevent latency lag
        if (queue_.size() >= max_size_) {
            queue_.pop(); 
        }
        queue_.push(std::move(item));
        cond_.notify_one();
        return true;
    }

    // CUJ / Hot Path: Consumer pops a frame for inference. Blocks if queue is empty.
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !queue_.empty(); });
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<T> empty;
        std::swap(queue_, empty);
    }
};

/**
 * @brief Main Edge Vision Daemon handling video capture, preprocessing,
 * inference execution, and telemetry alerting.
 */
class VisionDaemon {
private:
    std::string source_uri_;
    std::atomic<bool> is_running_;
    ThreadSafeQueue<cv::Mat> frame_queue_;
    std::thread capture_thread_;
    std::thread worker_thread_;

    // Execution Path: Frame capture loop running on dedicated producer thread
    void captureLoop();

    // Hot Path: Inference and telemetry processing loop running on worker thread
    void workerLoop();

public:
    explicit VisionDaemon(std::string source_uri);
    ~VisionDaemon();

    // Execution Path: Initializes streams and spawns threads
    bool start();

    // Execution Path: Signals graceful shutdown and joins threads
    void stop();
};