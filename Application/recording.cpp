// Cải thiện recording.cpp
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>

static std::atomic<bool> recording(false);
static std::mutex recordMutex;
static std::thread recordingThread;
static std::condition_variable recordingComplete;
static std::atomic<bool> fileReady(false);

static cv::VideoCapture cap;
static cv::VideoWriter writer;

static const int frameWidth = 1280;
static const int frameHeight = 720;
static const int fps = 20;  // Giảm fps để ổn định hơn

static std::string video_path;
static std::mutex videoPathMutex;

void recording_loop() {
  cv::Mat frame, resizedFrame;
  auto lastFrameTime = std::chrono::steady_clock::now();
  auto frameDuration = std::chrono::milliseconds(1000 / fps);
  int frameCount = 0;

  while (recording.load()) {
    auto currentTime = std::chrono::steady_clock::now();

    if (currentTime - lastFrameTime < frameDuration) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    cap >> frame;
    if (frame.empty()) {
      std::cerr << "[Server] Empty frame captured during recording!"
                << std::endl;
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
        frameCount++;

        // Flush mỗi 30 frames để đảm bảo data được ghi
        if (frameCount % 30 == 0) {
          writer << cv::Mat();  // Flush buffer
        }
      }
    }

    lastFrameTime = currentTime;
  }

  std::cout << "[Server] Recording loop ended. Total frames: " << frameCount
            << std::endl;
}

void start_recording(const std::string& outFilePath) {
  if (recording.load()) {
    std::cout << "[Server] Recording already in progress." << std::endl;
    return;
  }

  fileReady.store(false);

  {
    std::lock_guard<std::mutex> lock(videoPathMutex);
    video_path = outFilePath;
  }

  cap.open(0);
  if (!cap.isOpened()) {
    std::cerr << "[Server] Failed to open webcam for recording!" << std::endl;
    return;
  }

  // Set camera properties
  cap.set(cv::CAP_PROP_FRAME_WIDTH, frameWidth);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, frameHeight);
  cap.set(cv::CAP_PROP_FPS, fps);

  namespace fs = std::filesystem;
  fs::create_directories(fs::path(outFilePath).parent_path());

  // Thử nhiều codec khác nhau để tăng khả năng tương thích
  std::vector<int> codecs = {
      cv::VideoWriter::fourcc('X', 'V', 'I', 'D'),  // XVID - tương thích tốt
      cv::VideoWriter::fourcc('M', 'P', '4', 'V'),  // MPEG-4
      cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),  // MJPEG
      cv::VideoWriter::fourcc('H', '2', '6', '4')   // H.264
  };

  bool writerOpened = false;
  for (int codec : codecs) {
    writer.open(outFilePath, codec, fps, cv::Size(frameWidth, frameHeight),
                true);
    if (writer.isOpened()) {
      std::cout << "[Server] Using codec: " << std::hex << codec << std::dec
                << std::endl;
      writerOpened = true;
      break;
    }
  }

  if (!writerOpened) {
    std::cerr << "[Server] Failed to open VideoWriter with any codec: "
              << outFilePath << std::endl;
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

  std::cout << "[Server] Stopping recording..." << std::endl;
  recording.store(false);

  if (recordingThread.joinable()) {
    recordingThread.join();
  }

  {
    std::lock_guard<std::mutex> lock(recordMutex);
    if (writer.isOpened()) {
      // Đảm bảo flush tất cả data trước khi đóng
      std::cout << "[Server] Finalizing video file..." << std::endl;
      writer.release();
    }
  }

  if (cap.isOpened()) {
    cap.release();
  }

  // Đợi một chút để file system hoàn tất việc ghi
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::string savedPath;
  {
    std::lock_guard<std::mutex> lock(videoPathMutex);
    savedPath = video_path;
  }

  // Kiểm tra file có tồn tại và có kích thước hợp lý không
  if (std::filesystem::exists(savedPath)) {
    auto fileSize = std::filesystem::file_size(savedPath);
    std::cout << "[Server] Recording stopped and video saved: " << savedPath
              << " (" << fileSize << " bytes)" << std::endl;
    fileReady.store(true);
    recordingComplete.notify_all();
  } else {
    std::cerr << "[Server] Error: Video file was not created properly!"
              << std::endl;
  }
}

// Thêm hàm để đợi file sẵn sàng
bool wait_for_recording_complete(int timeoutSeconds = 10) {
  std::unique_lock<std::mutex> lock(recordMutex);
  return recordingComplete.wait_for(lock, std::chrono::seconds(timeoutSeconds),
                                    [] { return fileReady.load(); });
}