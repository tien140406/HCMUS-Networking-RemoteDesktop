#pragma once
#include "lib.h"

const size_t ROBUST_CHUNK_SIZE = 8192;    // 8KB chunks
const size_t ROBUST_BUFFER_SIZE = 65536;  // 64KB socket buffer
const int ROBUST_TIMEOUT_MS = 30000;      // 30 giây
const int ROBUST_MAX_RETRIES = 5;

// ===== HELPER FUNCTIONS =====

bool configure_socket_for_large_files(SOCKET sock);
bool send_file_size_robust(SOCKET sock, size_t fileSize);

bool receive_file_size_robust(SOCKET sock, size_t& fileSize);
bool send_chunk_complete(SOCKET sock, const char* data, size_t size);

bool receive_chunk_complete(SOCKET sock, char* data, size_t expectedSize,
                            size_t& actualReceived);

bool open_file_with_retry(std::ifstream& file, const std::string& filename,
                          int maxRetries);
void send_zero_size_signal(SOCKET sock);

// =====================================================
// MAIN FUNCTIONS - Thay thế functions cũ
// =====================================================

bool send_file_over_socket_robust(SOCKET sock, const std::string& filename);

bool receive_file_from_socket_robust(SOCKET sock, const std::string& filename);

// Gửi file qua socket
bool send_file_over_socket(SOCKET sock, const std::string& filename);

// Nhận file từ socket
void receive_file_from_socket(SOCKET sock, const std::string& filename);

// Gửi file với attachment qua email (để client sử dụng)
void send_file_via_email(const std::string& toEmail, const std::string& subject,
                         const std::string& body, const std::string& filepath);