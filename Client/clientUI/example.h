#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

enum class UIMode {
    EMAIL,
    MANUAL
};

enum class CommandState {
    IDLE,
    LIST_PROCESSES,
    START_PROCESS,
    KILL_PROCESS,
    LIST_APPS,
    START_APP,
    STOP_APP,
    SHUTDOWN,
    RESTART,
    KEYLOGGER,
    WEBCAM,
    GET_FILE,
    LIST_DIRECTORY
};

void SetCustomImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Light background
    colors[ImGuiCol_WindowBg]             = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
    colors[ImGuiCol_ChildBg]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PopupBg]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

    // Borders and frames
    colors[ImGuiCol_Border]               = ImVec4(0.70f, 0.70f, 0.70f, 0.30f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button]               = ImVec4(0.80f, 0.85f, 0.90f, 1.00f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.60f, 0.75f, 0.85f, 1.00f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.40f, 0.60f, 0.80f, 1.00f);

    // Headers (tree nodes, tables, etc.)
    colors[ImGuiCol_Header]               = ImVec4(0.80f, 0.85f, 0.90f, 0.65f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.60f, 0.75f, 0.85f, 0.80f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.40f, 0.60f, 0.80f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab]                  = ImVec4(0.85f, 0.90f, 0.95f, 1.00f);
    colors[ImGuiCol_TabHovered]           = ImVec4(0.65f, 0.80f, 0.95f, 1.00f);
    colors[ImGuiCol_TabActive]            = ImVec4(0.50f, 0.70f, 0.90f, 1.00f);

    // Titles
    colors[ImGuiCol_TitleBg]              = ImVec4(0.90f, 0.95f, 1.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.80f, 0.90f, 1.00f, 1.00f);

    // Text
    colors[ImGuiCol_Text]                 = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Sliders and checks
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.60f, 0.70f, 0.85f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.40f, 0.55f, 0.80f, 1.00f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.20f, 0.40f, 0.70f, 1.00f);

    // Style settings
    style.WindowRounding     = 8.0f;
    style.FrameRounding      = 6.0f;
    style.GrabRounding       = 4.0f;
    style.ScrollbarRounding  = 6.0f;
    style.TabRounding        = 4.0f;

    style.WindowPadding      = ImVec2(12, 12);
    style.FramePadding       = ImVec2(8, 6);
    style.ItemSpacing        = ImVec2(10, 8);
    style.IndentSpacing      = 20.0f;
}

struct ResultMessage {
    std::string message;
    std::string timestamp;
    ImVec4 color;
    
    ResultMessage(const std::string& msg, ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) {
        message = msg;
        color = col;
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
        timestamp = buffer;
    }
};

class RemoteAdminUI {
private:
    // Connection state
    char serverIP[64] = "192.168.1.100";
    char serverPort[16] = "8080";
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
    
    // Colors
    ImVec4 colorSuccess = ImVec4(0.15f, 0.8f, 0.15f, 1.0f);
    ImVec4 colorError = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 colorWarning = ImVec4(0.85f, 0.85f, 0.2f, 1.0f);
    ImVec4 colorInfo = ImVec4(0.7f, 0.7f, 1.0f, 1.0f);

public:
    void AddResult(const std::string& message, ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) {
        results.emplace_back(message, color);
        // Keep only last 100 messages
        if (results.size() > 100) {
            results.erase(results.begin());
        }
    }
    
    void HandleConnect() {
        if (isConnected) {
            // Disconnect
            isConnected = false;
            connectionStatus = "Disconnected";
            activeCommand = CommandState::IDLE;
            AddResult("Disconnected from server", colorWarning);
            // TODO: Close your socket connection here
        } else {
            // Connect
            connectionStatus = "Connecting...";
            // TODO: Add your actual socket connection logic here
            
            // Simulate connection for demo
            isConnected = true;
            connectionStatus = "Connected";
            AddResult("Connected to " + std::string(serverIP) + ":" + std::string(serverPort), colorSuccess);
        }
    }
    
    void ExecuteCommand(CommandState command) {
        if (activeCommand != CommandState::IDLE) {
            AddResult("Another command is already running!", colorError);
            return;
        }
        
        activeCommand = command;
        
        // TODO: Replace these with your actual backend calls
        switch (command) {
            case CommandState::LIST_PROCESSES:
                AddResult("Executing: List Processes", colorInfo);
                // Your backend call here
                SimulateCommand("Process List:\nnotepad.exe (PID: 1234)\nchrome.exe (PID: 5678)");
                break;
                
            case CommandState::START_PROCESS:
                AddResult("Executing: Start Process - " + std::string(processName), colorInfo);
                // Your backend call here
                SimulateCommand("Started process: " + std::string(processName));
                break;
                
            case CommandState::KILL_PROCESS:
                AddResult("Executing: Kill Process - " + std::string(processName), colorInfo);
                // Your backend call here
                SimulateCommand("Killed process: " + std::string(processName));
                break;
                
            case CommandState::LIST_APPS:
                AddResult("Executing: List Applications", colorInfo);
                // Your backend call here
                SimulateCommand("Installed Applications:\nGoogle Chrome\nNotepad++\nVLC Media Player");
                break;
                
            case CommandState::START_APP:
                AddResult("Executing: Start App - " + std::string(appName), colorInfo);
                // Your backend call here
                SimulateCommand("Started application: " + std::string(appName));
                break;
                
            case CommandState::STOP_APP:
                AddResult("Executing: Stop App - " + std::string(appName), colorInfo);
                // Your backend call here
                SimulateCommand("Stopped application: " + std::string(appName));
                break;
                
            case CommandState::SHUTDOWN:
                AddResult("Executing: Shutdown", colorInfo);
                // Your backend call here
                SimulateCommand("Server shutdown initiated", colorWarning);
                break;
                
            case CommandState::RESTART:
                AddResult("Executing: Restart", colorInfo);
                // Your backend call here
                SimulateCommand("Server restart initiated", colorWarning);
                break;
                
            case CommandState::KEYLOGGER:
                AddResult("Executing: Keylogger for " + std::to_string(keyloggerDuration) + " seconds", colorInfo);
                // Your backend call here
                // Note: For keylogger, you'll need to handle the duration in your backend
                SimulateCommand("Keylogger started");
                break;
                
            case CommandState::WEBCAM:
                AddResult("Executing: Webcam Capture", colorInfo);
                // Your backend call here
                SimulateCommand("Webcam capture completed\nImage saved to: webcam_capture.jpg");
                break;
                
            case CommandState::GET_FILE:
                AddResult("Executing: Get File - " + std::string(filePath), colorInfo);
                // Your backend call here
                SimulateCommand("File downloaded: " + std::string(filePath));
                break;
                
            case CommandState::LIST_DIRECTORY:
                AddResult("Executing: List Directory - " + std::string(directoryPath), colorInfo);
                // Your backend call here
                SimulateCommand("Directory listing:\nfile1.txt\nfile2.jpg\nsubfolder/");
                break;
        }
    }
    
    // Simulate command completion (replace this with your actual response handling)
    void SimulateCommand(const std::string& result, ImVec4 color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f)) {
        // In real implementation, this would be called when you receive response from server
        AddResult(result, color);
        activeCommand = CommandState::IDLE;
    }
    
    void CheckEmailAutomatically() {
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
                // For now, simulate check completion after some time
                static bool checking = false;
                if (!checking) {
                    checking = true;
                    // In real implementation, this would be async
                    // You could use a separate thread or callback
                    emailStatus = "No new commands";
                    checking = false;
                }
            }
        }
    }
    
    void RenderConnectionPanel() {
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
            ImGui::PushStyleColor(ImGuiCol_Text, isConnected ? colorSuccess : colorError);
            ImGui::Text("%s", connectionStatus.c_str());
            ImGui::PopStyleColor();
        }
    }
    
    void RenderModeSwitch() {
        if (ImGui::CollapsingHeader("Mode Selection", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BeginDisabled(activeCommand != CommandState::IDLE);
            
            if (ImGui::RadioButton("Email Mode", currentMode == UIMode::EMAIL)) {
                currentMode = UIMode::EMAIL;
                lastCheckTime = std::chrono::steady_clock::now() - std::chrono::seconds(15); // Trigger immediate check
            }
            
            ImGui::SameLine();
            
            if (ImGui::RadioButton("Manual Mode", currentMode == UIMode::MANUAL)) {
                currentMode = UIMode::MANUAL;
            }
            
            ImGui::EndDisabled();
            
            if (activeCommand != CommandState::IDLE) {
                ImGui::PushStyleColor(ImGuiCol_Text, colorWarning);
                ImGui::Text("Command is running... Please wait.");
                ImGui::PopStyleColor();
            }
        }
    }
    
    void RenderEmailMode() {
        if (ImGui::CollapsingHeader("Email Command Monitor", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Last Check: %s", lastEmailCheck.c_str());
            ImGui::Text("Status: %s", emailStatus.c_str());
            ImGui::Separator();
            ImGui::TextWrapped("Automatically checking for email commands every 15 seconds...");
        }
    }
    
    void RenderManualMode() {
        bool isDisabled = !isConnected || activeCommand != CommandState::IDLE;
        
        if (ImGui::CollapsingHeader("Manual Commands", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BeginDisabled(isDisabled);
            
            // Process Management
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
            
            // Application Management
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
            
            // System Commands
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
            
            // Monitoring & Files
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
            
            ImGui::EndDisabled();
        }
    }
    
    void RenderResults() {
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
    
    void Render() {
        // Update email checking
        CheckEmailAutomatically();
        
        ImGui::Begin("Remote Administration Client");
        
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
};

// Global instance
RemoteAdminUI g_ui;

// Main application loop
int run_imgui_example() {
    // Initialize GLFW
    if (!glfwInit()) return -1;
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(2400, 1600, "Remote Administration Client", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Scale up FIRST
    float scaleFactor = 1.5f;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f * scaleFactor);
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    // Apply custom theme AFTER ImGui is initialized
    // Don't call ImGui::StyleColorsDark() - it overwrites your custom theme!
    SetCustomImGuiStyle();  // This sets both colors and style
    
    // Scale the style AFTER setting the theme
    ImGui::GetStyle().ScaleAllSizes(scaleFactor);
    io.FontGlobalScale = scaleFactor;
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Render our UI
        g_ui.Render();
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwTerminate();
    return 0;
}