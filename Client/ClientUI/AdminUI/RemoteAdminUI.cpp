#include "RemoteAdminUI.h"
#include "imgui.h"
#include <cstring>

RemoteAdminUI::RemoteAdminUI() {
    lastCheckTime = std::chrono::steady_clock::now();
}

RemoteAdminUI::~RemoteAdminUI() {
    // Cleanup if needed
}

void RemoteAdminUI::AddResult(const std::string& message, ImVec4 color) {
    results.emplace_back(message, color);
    // Keep only last 100 messages
    if (results.size() > 100) {
        results.erase(results.begin());
    }
}

void RemoteAdminUI::HandleConnect() {
    if (isConnected) {
        // Disconnect
        isConnected = false;
        connectionStatus = "Disconnected";
        activeCommand = CommandState::IDLE;
        AddResult("Disconnected from server", Colors::WARNING);
        // TODO: Close your socket connection here
    } else {
        // Connect
        connectionStatus = "Connecting...";
        // TODO: Add your actual socket connection logic here
        
        // Simulate connection for demo
        isConnected = true;
        connectionStatus = "Connected";
        AddResult("Connected to " + std::string(serverIP) + ":" + std::string(serverPort), Colors::SUCCESS);
    }
}

void RemoteAdminUI::ExecuteCommand(CommandState command) {
    if (activeCommand != CommandState::IDLE) {
        AddResult("Another command is already running!", Colors::ERROR);
        return;
    }
    
    activeCommand = command;
    
    // TODO: Replace these with your actual backend calls
    switch (command) {
        case CommandState::LIST_PROCESSES:
            AddResult("Executing: List Processes", Colors::INFO);
            SimulateCommand("Process List:\nnotepad.exe (PID: 1234)\nchrome.exe (PID: 5678)");
            break;
            
        case CommandState::START_PROCESS:
            AddResult("Executing: Start Process - " + std::string(processName), Colors::INFO);
            SimulateCommand("Started process: " + std::string(processName));
            break;
            
        case CommandState::KILL_PROCESS:
            AddResult("Executing: Kill Process - " + std::string(processName), Colors::INFO);
            SimulateCommand("Killed process: " + std::string(processName));
            break;
            
        case CommandState::LIST_APPS:
            AddResult("Executing: List Applications", Colors::INFO);
            SimulateCommand("Installed Applications:\nGoogle Chrome\nNotepad++\nVLC Media Player");
            break;
            
        case CommandState::START_APP:
            AddResult("Executing: Start App - " + std::string(appName), Colors::INFO);
            SimulateCommand("Started application: " + std::string(appName));
            break;
            
        case CommandState::STOP_APP:
            AddResult("Executing: Stop App - " + std::string(appName), Colors::INFO);
            SimulateCommand("Stopped application: " + std::string(appName));
            break;
            
        case CommandState::SHUTDOWN:
            AddResult("Executing: Shutdown", Colors::INFO);
            SimulateCommand("Server shutdown initiated", Colors::WARNING);
            break;
            
        case CommandState::RESTART:
            AddResult("Executing: Restart", Colors::INFO);
            SimulateCommand("Server restart initiated", Colors::WARNING);
            break;
            
        case CommandState::KEYLOGGER:
            AddResult("Executing: Keylogger for " + std::to_string(keyloggerDuration) + " seconds", Colors::INFO);
            SimulateCommand("Keylogger started");
            break;
            
        case CommandState::WEBCAM:
            AddResult("Executing: Webcam Capture", Colors::INFO);
            SimulateCommand("Webcam capture completed\nImage saved to: webcam_capture.jpg");
            break;
            
        case CommandState::GET_FILE:
            AddResult("Executing: Get File - " + std::string(filePath), Colors::INFO);
            SimulateCommand("File downloaded: " + std::string(filePath));
            break;
            
        case CommandState::LIST_DIRECTORY:
            AddResult("Executing: List Directory - " + std::string(directoryPath), Colors::INFO);
            SimulateCommand("Directory listing:\nfile1.txt\nfile2.jpg\nsubfolder/");
            break;
    }
}

void RemoteAdminUI::SimulateCommand(const std::string& result, ImVec4 color) {
    // In real implementation, this would be called when you receive response from server
    AddResult(result, color);
    activeCommand = CommandState::IDLE;
}

void RemoteAdminUI::CheckEmailAutomatically() {
    if (currentMode == UIMode::EMAIL && isConnected) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastCheckTime).count() >= 15) {
            lastCheckTime = now;
            
            // Update timestamp
            auto time_now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(time_now);
            auto tm = *std::localtime(&time_t);
            char buffer[32];
            strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
            lastEmailCheck = buffer;
            
            emailStatus = "Checking...";
            
            // TODO: Add your actual email checking logic here
            static bool checking = false;
            if (!checking) {
                checking = true;
                emailStatus = "No new commands";
                checking = false;
            }
        }
    }
}