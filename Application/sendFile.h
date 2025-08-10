#pragma once
#include "lib.h"

// Gửi file qua socket
bool send_file_over_socket(SOCKET sock, const std::string& filename);

// Nhận file từ socket
void receive_file_from_socket(SOCKET sock, const std::string& filename);

// Gửi file với attachment qua email (để client sử dụng)
void send_file_via_email(const std::string& toEmail, const std::string& subject,
                         const std::string& body, const std::string& filepath);