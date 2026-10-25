
# Edge Vision Daemon

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue?logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/20)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.x-green?logo=opencv&logoColor=white)](https://opencv.org/)
[![ONNX Runtime](https://img.shields.io/badge/ONNX-Runtime-informational?logo=onnx&logoColor=white)](https://onnxruntime.ai/)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-red?logo=cmake&logoColor=white)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A high-performance, multi-threaded C++20 edge computer vision daemon designed for industrial manufacturing assembly lines. The daemon ingests live video streams (Webcam / RTSP), processes frames through zero-copy buffers, and executes real-time tensor inference to detect surface defects and trigger automated telemetry alerts.

---

## Overview

Industrial IoT (IIoT) edge environments demand deterministic sub-millisecond execution, memory safety without garbage collection pauses, and robust fault recovery. This application demonstrates production systems programming by decoupling high-speed frame capture from deep learning inference using a thread-safe bounded producer-consumer architecture.

---

## Key Features

- **C++20 Concurrency:** Utilizes modern threading primitives, `std::atomic`, and condition variables for efficient inter-thread synchronization.
- **Bounded Backpressure Queue:** Prevents memory exhaustion during inference spikes by automatically dropping stale frames when queue capacity is reached.
- **Graceful Shutdown Handling:** Traps OS interrupts (`SIGINT`, `SIGTERM`) to ensure clean thread joining and resource release.
- **Robust Error Recovery:** Automatic stream reconnection logic for dropped RTSP feeds.

---

## Architecture Diagram

The system separates execution into two primary asynchronous threads connected via a thread-safe safe queue:

```text
 [Camera / RTSP Stream] 
           │
           ▼  (Hot Path: Frame Ingestion)
 ┌─────────────────────────┐
 |    Capture Thread       | ──(Push)──> ┌─────────────────────────┐
 └─────────────────────────┘             |   ThreadSafeQueue<Mat>  |
                                         └─────────────────────────┘
                                                       │
                                                    (Pop)
                                                       │
                                                       ▼  (Hot Path: Preprocessing & Inference)
                                         ┌─────────────────────────┐
                                         |    Worker Thread        | ──> [Telemetry Alert / Anomaly]
                                         └─────────────────────────┘

```

---

## Tech Stack & Dependencies

| Layer | Technology | Purpose |
| --- | --- | --- |
| **Language** | **C++20** | Systems-level performance and modern language features |
| **Build System** | **CMake (3.20+)** | Cross-platform build configuration |
| **Computer Vision** | **OpenCV** | Frame capture, resizing, and image manipulation |
| **Inference Runtime** | **ONNX Runtime / TensorRT** | High-performance neural network execution |
| **Concurrency** | **Standard Library (`<thread>`, `<mutex>`)** | Multi-threaded synchronization |

---

## Local Development & Setup

### Prerequisites

Ensure you have a C++20 compiler (`gcc`, `clang`, or `MSVC`), **CMake**, and **OpenCV** installed on your system.

* **macOS (Homebrew):**
```bash
brew install cmake opencv

```


* **Ubuntu / Linux:**
```bash
sudo apt update && sudo apt install -y cmake g++ libopencv-dev build-essential

```



### Cloning and Building

1. Clone your private repository:
```bash
git clone <your-private-repo-url>
cd edge-vision-daemon

```


2. Create and compile via CMake:
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release

```



---

## Daemon Interface & Usage

Run the compiled executable from your `build` directory:

* **Run with default webcam (index `0`):**
```bash
./edge_vision_daemon 0

```


* **Run with an RTSP security or factory camera stream:**
```bash
./edge_vision_daemon rtsp://username:password@camera-ip:554/stream

```



---

## Critical User Journeys (CUJs) & Execution Paths

1. **The Execution Path (Startup & Shutdown):**
* Initializes command-line arguments, spawns producer and worker threads, and registers signal handlers. On shutdown, safely signals atomic flags and joins threads to prevent leaks.


2. **The Hot Path (Inference Loop):**
* Continuously grabs frames from the hardware buffer, resizes images to `640x640` tensors, and passes them through inference evaluation with minimal latency.


3. **The Sad Path (Fault Tolerance):**
* If a camera disconnects or stream reads fail, the capture loop catches the exception, sleeps for 1 second, and attempts automatic reconnection without crashing the daemon.



---

## License

Distributed under the MIT License. See `LICENSE` for more information.
