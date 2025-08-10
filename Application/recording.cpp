#include <atomic>
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>

static std::atomic<bool> recording(false);
static std::mutex recordMutex;
static std::thread recordingThread;

static cv::VideoCapture cap;
static cv::VideoWriter writer;

static const int frameWidth = 1280;
static const int frameHeight = 720;
static const int fps = 30;

static std::string video_path;
static std::mutex videoPathMutex;

void recording_loop() {
    cv::Mat frame, resizedFrame;
    auto lastFrameTime = std::chrono::steady_clock::now();
    auto frameDuration = std::chrono::milliseconds(1000 / fps);

    while (recording.load()) {
        auto currentTime = std::chrono::steady_clock::now();
        
        if (currentTime - lastFrameTime < frameDuration) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        cap >> frame;
        if (frame.empty()) {
            std::cerr << "[Server] Empty frame captured during recording!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        if (frame.cols != frameWidth || frame.rows != frameHeight) {
            cv::resize(frame, resizedFrame, cv::Size(frameWidth, frameHeight));
        } else {
            resizedFrame = frame;
        }

        {
            std::lock_guard<std::mutex> lock(recordMutex);
            if (recording.load() && writer.isOpened()) {
                writer.write(resizedFrame);
            }
        }

        lastFrameTime = currentTime;
    }
}

void start_recording(const std::string& outFilePath) {
    if (recording.load()) {
        std::cout << "[Server] Recording already in progress." << std::endl;
        return;
    }

    {
        std::lock_guard<std::mutex> lock(videoPathMutex);
        video_path = outFilePath;
    }

    cap.open(0);
    if (!cap.isOpened()) {
        std::cerr << "[Server] Failed to open webcam for recording!" << std::endl;
        return;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, frameWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, frameHeight);
    cap.set(cv::CAP_PROP_FPS, fps);

    namespace fs = std::filesystem;
    fs::create_directories(fs::path(outFilePath).parent_path());

    int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    writer.open(outFilePath, fourcc, fps, cv::Size(frameWidth, frameHeight), true);

    if (!writer.isOpened()) {
        std::cerr << "[Server] Failed to open VideoWriter: " << outFilePath << std::endl;
        cap.release();
        return;
    }

    recording.store(true);
    recordingThread = std::thread(recording_loop);

    std::cout << "[Server] Recording started: " << outFilePath << std::endl;
}

void stop_recording() {
    if (!recording.load()) {
        std::cout << "[Server] No recording in progress to stop." << std::endl;
        return;
    }

    recording.store(false);

    if (recordingThread.joinable()) {
        recordingThread.join();
    }

    {
        std::lock_guard<std::mutex> lock(recordMutex);
        if (writer.isOpened()) {
            writer.release();
        }
    }
    if (cap.isOpened()) {
        cap.release();
    }
}
