#pragma once
#include "lib.h"
#include "sendEmail.h"

namespace cv {
class VideoCapture;
class VideoWriter;
class Mat;
}  // namespace cv

// Recording functions - không có send_picture để tránh conflict
void start_recording();
void stop_and_send_recording();
void cleanup_recording();
bool isValidTempPath(const std::string& path);

// Thêm các hàm để tích hợp với server
void capture_picture_to_file(const std::string& outFilePath);
void run_recording_and_save(const std::string& outFilePath,
                            int duration_seconds);

namespace RecordingConfig {
constexpr int DEFAULT_FPS = 20;
constexpr int DEFAULT_WIDTH = 1280;
constexpr int DEFAULT_HEIGHT = 720;
constexpr int MAX_CAPTURE_ATTEMPTS = 5;
constexpr int MAX_CONSECUTIVE_EMPTY_FRAMES = 10;
}  // namespace RecordingConfig