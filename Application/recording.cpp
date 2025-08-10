#include "recording.h"
#include <opencv2/opencv.hpp>
#include <thread>
#include <chrono>
#include <windows.h>
#include <iostream>
#include <string>
#include <filesystem>

using namespace std;

// Hàm để tích hợp với server - record trong thời gian xác định
void run_recording_and_save(const std::string& outFilePath,
                            int duration_seconds) {
  namespace fs = std::filesystem;

  // Tạo thư mục nếu chưa có
  fs::create_directories(fs::path(outFilePath).parent_path());

  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    cerr << "[Server] Failed to open webcam!" << endl;
    return;
  }

  // Get camera properties
  int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
  int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
  int fps = RecordingConfig::DEFAULT_FPS;

  // Set better camera properties
  cap.set(cv::CAP_PROP_FRAME_WIDTH, RecordingConfig::DEFAULT_WIDTH);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, RecordingConfig::DEFAULT_HEIGHT);
  cap.set(cv::CAP_PROP_FPS, fps);

  // Re-read properties after setting
  frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
  frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

  cv::VideoWriter writer(outFilePath,
                         cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps,
                         cv::Size(frame_width, frame_height));

  if (!writer.isOpened()) {
    cerr << "[Server] Failed to open VideoWriter for: " << outFilePath << endl;
    return;
  }

  cout << "[Server] Started recording video for " << duration_seconds
       << " seconds to: " << outFilePath << endl;

  cv::Mat frame;
  auto start_time = std::chrono::steady_clock::now();
  auto end_time = start_time + std::chrono::seconds(duration_seconds);
  int frame_count = 0;

  while (std::chrono::steady_clock::now() < end_time) {
    cap >> frame;

    if (!frame.empty()) {
      writer.write(frame);
      frame_count++;
    }

    // Maintain steady frame rate
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
  }

  // Cleanup
  cap.release();
  writer.release();

  cout << "[Server] Finished recording " << frame_count
       << " frames to: " << fs::absolute(outFilePath) << endl;
}
