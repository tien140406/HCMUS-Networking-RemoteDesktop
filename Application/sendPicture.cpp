#include "sendPicture.h"

void take_picture(const std::string& outFilePath) {
  namespace fs = std::filesystem;

  // Tạo thư mục nếu chưa có
  fs::create_directories(fs::path(outFilePath).parent_path());

  // Sử dụng OpenCV để chụp ảnh
  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    cerr << "[Server] Failed to open webcam!" << endl;
    return;
  }

  // Set camera properties for better quality
  cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

  cv::Mat frame;

  // Try multiple times to get a valid frame
  int attempts = 0;
  const int max_attempts = 5;

  do {
    cap >> frame;
    if (!frame.empty()) break;

    attempts++;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  } while (attempts < max_attempts);

  cap.release();

  if (frame.empty()) {
    cerr << "[Server] Failed to capture image after " << max_attempts
         << " attempts!" << endl;
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
