#pragma once
#include "lib.h"

const size_t ROBUST_CHUNK_SIZE = 8192;    // 8KB chunks
const size_t ROBUST_BUFFER_SIZE = 65536;  // 64KB socket buffer
const int ROBUST_TIMEOUT_MS = 30000;      // 30 giây
const int ROBUST_MAX_RETRIES = 5;

bool send_file_over_socket(SOCKET sock, const std::string& filename);

void receive_file_from_socket(SOCKET sock, const std::string& filename);