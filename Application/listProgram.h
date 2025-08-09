#pragma once
#include "lib.h"
#include "sendEmail.h"

<<<<<<< Updated upstream
void list_programs();
=======
// Liệt kê các tiến trình đang chạy (processes)
void list_processes();
void list_processes_to_file(const std::string &outFile);

// Liệt kê các chương trình đã cài đặt (installed programs)
void list_programs();
void list_programs_to_file(const std::string &outFile);
>>>>>>> Stashed changes
