#include "VisionDaemon.hpp"
#include <iostream>
#include <csignal>

// Global pointer for catching system interrupts (SIGINT/SIGTERM) for graceful shutdown
static std::atomic<bool> g_shutdown_requested(false);

void signalHandler(int signum) {
    std::cout << "\n[Execution Path] Interrupt signal (" << signum << ") received. Shutting down daemon gracefully...\n";
    g_shutdown_requested = true;
}

VisionDaemon::VisionDaemon(std::string source_uri)
    : source_uri_(std::move(source_uri)), is_running_(false), frame_queue_(30) {}

VisionDaemon::~VisionDaemon() {
    stop();
}

bool VisionDaemon::start() {
    std::cout << "[Execution Path] Initializing VisionDaemon with source: " << source_uri_ << "\n";
    is_running_ = true;

    // Launch Producer Thread (Camera/RTSP Stream Capture)
    capture_thread_ = std::thread(&VisionDaemon::captureLoop, this);

    // Launch Consumer Thread (Inference & Anomaly Detection Pipeline)
    worker_thread_ = std::thread(&VisionDaemon::workerLoop, this);

    return true;
}

void VisionDaemon::captureLoop() {
    cv::VideoCapture cap;
    
    // Execution Path: Attempt opening video source (index 0 for webcam or RTSP string URL)
    if (source_uri_ == "0") {
        cap.open(0);
    } else {
        cap.open(source_uri_);
    }

    // Sad Path: Handle camera stream failure or disconnection
    if (!cap.isOpened()) {
        std::cerr << "[Sad Path Error] Failed to open video stream source: " << source_uri_ << "\n";
        is_running_ = false;
        return;
    }

    std::cout << "[Execution Path] Video stream successfully opened. Beginning frame ingestion...\n";
    cv::Mat frame;

    while (is_running_ && !g_shutdown_requested) {
        // Hot Path: Read raw frame from hardware buffer
        if (!cap.read(frame)) {
            std::cerr << "[Sad Path Warning] Frame read failed or stream ended. Attempting reconnection...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        if (frame.empty()) continue;

        // Push frame into thread-safe bounded queue for worker consumption
        frame_queue_.push(std::move(frame));
    }

    cap.release();
    std::cout << "[Execution Path] Capture thread terminated cleanly.\n";
}

void VisionDaemon::workerLoop() {
    std::cout << "[Execution Path] Inference worker thread started.\n";
    cv::Mat current_frame;

    while (is_running_ && !g_shutdown_requested) {
        // Hot Path: Retrieve frame from queue
        if (!frame_queue_.pop(current_frame)) {
            break;
        }

        // Hot Path / Preprocessing: Resize and normalize frame for ONNX runtime tensor ingestion
        cv::Mat resized;
        cv::resize(current_frame, resized, cv::Size(640, 640));

        // Simulated AI Tensor Execution (Placeholder for ONNX Runtime C++ API inference call)
        // In production, tensor buffer data is mapped directly into ONNX Runtime memory arena.
        bool anomaly_detected = (cv::mean(resized)[0] > 200.0); // Dummy threshold logic for demonstration

        // CUJ / Telemetry Alert Path
        if (anomaly_detected) {
            std::cout << "[CUJ Alert] Surface anomaly detected on assembly line frame! Dispatching telemetry event...\n";
        }
    }

    std::cout << "[Execution Path] Worker thread terminated cleanly.\n";
}

void VisionDaemon::stop() {
    if (is_running_) {
        is_running_ = false;
        frame_queue_.clear();

        // Join threads safely to prevent resource leaks
        if (capture_thread_.joinable()) capture_thread_.join();
        if (worker_thread_.joinable()) worker_thread_.join();
        
        std::cout << "[Execution Path] VisionDaemon fully stopped.\n";
    }
}

int main(int argc, char* argv[]) {
    // Register signal handlers for graceful exit (Ctrl+C)
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Determine input source (default to webcam index "0" if argument not specified)
    std::string source = (argc > 1) ? argv[1] : "0";

    VisionDaemon daemon(source);
    if (!daemon.start()) {
        std::cerr << "[Fatal Error] Could not start Vision Daemon.\n";
        return 1;
    }

    std::cout << "[Execution Path] Edge Vision Daemon running. Press Ctrl+C to terminate.\n";

    // Main thread heartbeat block
    while (!g_shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    daemon.stop();
    std::cout << "[Execution Path] Application shutdown complete.\n";
    return 0;
}