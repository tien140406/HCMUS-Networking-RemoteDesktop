#pragma once
#include "lib.h"

// Danh sách chương trình đang chạy (running programs - không có PID)
void list_programs_to_file(const std::string& outFile);

// Danh sách process đang chạy (với PID chi tiết)
void list_processes_to_file(const std::string& outFile);

// Danh sách chương trình đã cài đặt (installed programs - từ registry)
void list_installed_programs_to_file(const std::string& outFile);