#include "lib.h"
#include "sendEmail.h"

namespace cv {
    class VideoCapture;
    class VideoWriter;
    class Mat;
}

void send_picture();
void start_recording();
void stop_and_send_recording();
void cleanup_recording();
bool isValidTempPath(const std::string& path);

namespace RecordingConfig {
    constexpr int DEFAULT_FPS = 20;
    constexpr int DEFAULT_WIDTH = 1280;
    constexpr int DEFAULT_HEIGHT = 720;
    constexpr int MAX_CAPTURE_ATTEMPTS = 5;
    constexpr int MAX_CONSECUTIVE_EMPTY_FRAMES = 10;
}