#pragma once
#include "../Types.h"
#include "imgui.h"
#include <vector>
#include <chrono>
#include <GL/gl.h>
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>

struct ImageData {
    GLuint texture = 0;
    int width = 0;
    int height = 0;
};

class RemoteAdminUI {
private:
    // Connection state
    std::atomic<bool> stopEmailThread{false};
    std::thread emailThread;
    SOCKET mySock;
    char serverIP[64] = "127.0.0.1";
    char serverPort[16] = "8888";
    bool isConnected = false;
    std::string connectionStatus = "Disconnected";
    
    // UI state
    UIMode currentMode = UIMode::EMAIL;
    CommandState activeCommand = CommandState::IDLE;
    
    // Email state
    std::string lastEmailCheck = "Never";
    std::string emailStatus = "Idle";
    std::chrono::steady_clock::time_point lastCheckTime;
    
    // Manual command inputs
    char processName[256] = "";
    char appName[256] = "";
    int keyloggerDuration = 30;
    char filePath[512] = "";
    char directoryPath[512] = "";
    
    // Results
    std::vector<ResultMessage> results;

    // Private helper methods
    void HandleConnect();
    void ExecuteCommand(CommandState command);
    void SimulateCommand(const std::string& result, ImVec4 color = Colors::SUCCESS);
    
    // UI Rendering methods
    // File display variables
    GLuint currentResultTexture = 0;
    int currentResultWidth = 0;
    int currentResultHeight = 0;
    std::string currentResultFileName = "";
    std::string currentResultText = "";

    cv::VideoCapture currentVideo;
    bool isVideoPlaying = false;
    bool isVideoPaused = false;
    double videoFPS = 0.0;
    int videoFrameCount = 0;
    int currentVideoFrame = 0;
    std::chrono::steady_clock::time_point lastFrameTime;
    GLuint videoTexture = 0;

    void RenderFileDisplay();
    void RenderConnectionPanel();
    void RenderModeSwitch();
    void RenderEmailMode();
    void RenderManualMode();
    void RenderResults();
    void RenderProcessManagement();
    void RenderApplicationManagement();
    void RenderSystemCommands();
    void RenderMonitoringAndFiles();
    void RenderHelpPanel();
    ImVec2 CalculateImageSize(int imageWidth, int imageHeight);
    ImageData LoadImageTexture(const std::string& filepath);
    void LoadMedia(const std::string& filepath);

    void UpdateVideoFrame();
    void RenderVideoControls();
    GLuint CreateTextureFromMat(const cv::Mat& mat);

    void StartEmailCheckThread();
    void StopEmailCheckThread();
    void SetMode(UIMode mode);

public:
    RemoteAdminUI();
    ~RemoteAdminUI();
    
    void SetResultImage(GLuint texture, int width, int height, const std::string& filename);
    void SetResultText(const std::string& text);
    void AddResult(const std::string& message, ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    void ClearResult();
    void Render();

    void SeekVideo(int frame);
    void PlayPauseVideo();
    void StopVideo();
    void PlayVideo(const std::string& filepath);

    UIMode GetCurrentMode() const { return currentMode; }
};