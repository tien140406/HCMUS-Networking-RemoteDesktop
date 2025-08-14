#pragma once
#include <atomic>

// Global running flag (declared in main.cpp)
extern std::atomic<bool> running;
extern class RemoteAdminUI g_ui;

// UI manager main function
void run_ui();