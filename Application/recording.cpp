#include "recording.h"

using namespace std;

static atomic<bool> recording(false);
static mutex recordMutex;
static thread recordingThread;
std::atomic<bool> recording_in_progress(false);

static cv::VideoCapture cap;
static cv::VideoWriter writer;

static const int frameWidth = 1280;
static const int frameHeight = 720;
static const int fps = 30;

void recording_loop() {
    cv::Mat frame, resizedFrame;
    auto lastFrameTime = chrono::steady_clock::now();
    auto frameDuration = chrono::milliseconds(1000 / fps);

    while (recording.load()) {
        auto currentTime = chrono::steady_clock::now();
        
        // Check if enough time has passed for the next frame
        if (currentTime - lastFrameTime < frameDuration) {
            this_thread::sleep_for(chrono::milliseconds(1));
            continue;
        }

        cap >> frame;
        if (frame.empty()) {
            cerr << "[Server] Empty frame captured during recording!" << endl;
            this_thread::sleep_for(chrono::milliseconds(10));
            continue;
        }

        // Resize frame to match output dimensions if needed
        if (frame.cols != frameWidth || frame.rows != frameHeight) {
            cv::resize(frame, resizedFrame, cv::Size(frameWidth, frameHeight));
        } else {
            resizedFrame = frame;
        }

        {
            lock_guard<mutex> lock(recordMutex);
            if (recording.load() && writer.isOpened()) { // Double-check before writing
                writer.write(resizedFrame);
            }
        }

        lastFrameTime = currentTime;
    }
}

void start_recording(const std::string& outFilePath) {
    if (recording.load()) {
        cout << "[Server] Recording already in progress." << endl;
        return;
    }

    cap.open(0);
    if (!cap.isOpened()) {
        cerr << "[Server] Failed to open webcam for recording!" << endl;
        return;
    }

    // Get actual camera capabilities
    double actualWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double actualHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double actualFPS = cap.get(cv::CAP_PROP_FPS);
    
    cout << "[Server] Camera native resolution: " << actualWidth << "x" << actualHeight 
         << " @ " << actualFPS << " fps" << endl;

    // Try to set desired resolution
    cap.set(cv::CAP_PROP_FRAME_WIDTH, frameWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, frameHeight);
    cap.set(cv::CAP_PROP_FPS, fps);

    // Verify what was actually set
    actualWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    actualHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    cout << "[Server] Camera set to: " << actualWidth << "x" << actualHeight << endl;

    // Create output directory if needed
    namespace fs = std::filesystem;
    fs::create_directories(fs::path(outFilePath).parent_path());

    // Try different codecs for better compatibility
    int fourcc = cv::VideoWriter::fourcc('M', 'P', '4', 'V'); // Try MP4V first
    writer.open(outFilePath, fourcc, fps, cv::Size(frameWidth, frameHeight), true);
    
    if (!writer.isOpened()) {
        // Fallback to MJPG
        fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
        writer.open(outFilePath, fourcc, fps, cv::Size(frameWidth, frameHeight), true);
    }
    
    if (!writer.isOpened()) {
        cerr << "[Server] Failed to open VideoWriter: " << outFilePath << endl;
        cap.release();
        return;
    }

    recording.store(true);
    recording_in_progress.store(true);
    recordingThread = thread(recording_loop);

    cout << "[Server] Recording started with codec: " << (char*)&fourcc << endl;
}

void stop_recording() {
    if (!recording.load()) {
        cout << "[Server] No recording in progress to stop." << endl;
        return;
    }

    cout << "[Server] Stopping recording..." << endl;
    recording.store(false);

    if (recordingThread.joinable()) {
        recordingThread.join();
    }

    {
        lock_guard<mutex> lock(recordMutex);
        if (writer.isOpened()) {
            writer.release();
        }
    }
    
    if (cap.isOpened()) {
        cap.release();
    }
    
    recording_in_progress.store(false);
    cout << "[Server] Recording stopped and video saved." << endl;
}