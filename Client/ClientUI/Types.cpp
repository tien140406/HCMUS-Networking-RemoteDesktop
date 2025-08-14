#include "Types.h"
#include "imgui.h"
#include <ctime>

ResultMessage::ResultMessage(const std::string& msg, ImVec4 col) {
    message = msg;
    color = col;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    timestamp = buffer;
}

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

    // Sliders
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.50f, 0.65f, 0.85f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.35f, 0.55f, 0.80f, 1.00f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.20f, 0.40f, 0.70f, 1.00f);

    // Scrollbars (prettier)
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.94f, 0.94f, 0.94f, 1.00f); // Background track
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.75f, 0.80f, 0.85f, 1.00f); // Grip idle
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.60f, 0.75f, 0.85f, 1.00f); // Grip hover
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.40f, 0.50f, 0.60f, 0.80f); // Grip dragging

    // Style settings
    style.WindowRounding     = 8.0f;
    style.FrameRounding      = 6.0f;
    style.GrabRounding       = 4.0f;
    style.ScrollbarRounding  = 8.0f; // Round scrollbar corners
    style.TabRounding        = 4.0f;

    style.WindowPadding      = ImVec2(12, 12);
    style.FramePadding       = ImVec2(8, 6);
    style.ItemSpacing        = ImVec2(10, 8);
    style.IndentSpacing      = 20.0f;
}
