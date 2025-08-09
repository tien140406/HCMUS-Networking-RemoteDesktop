#pragma once
#include "lib.h"
#include "sendEmail.h"

// Danh sách chương trình đã cài đặt
void list_programs();
void list_programs_to_file(const std::string &outFile);

// Danh sách process đang chạy
void list_processes();
void list_processes_to_file(const std::string &outFile);