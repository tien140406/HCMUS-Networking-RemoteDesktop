#pragma once
#include "imgui.h"
#include <string>
#include <chrono>

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

struct ResultMessage {
    std::string message;
    std::string timestamp;
    ImVec4 color;
    
    ResultMessage(const std::string& msg, ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
};

// Color constants
namespace Colors {
    const ImVec4 SUCCESS = ImVec4(0.15f, 0.8f, 0.15f, 1.0f);
    const ImVec4 ERROR = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    const ImVec4 WARNING = ImVec4(0.85f, 0.85f, 0.2f, 1.0f);
    const ImVec4 INFO = ImVec4(0.7f, 0.7f, 1.0f, 1.0f);
}

void SetCustomImGuiStyle();