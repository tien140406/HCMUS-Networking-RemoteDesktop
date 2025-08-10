#include "sendFile.h"
#include "sendEmail.h"
#include <filesystem>

/*
const int BUFFER_SIZE = 4096;  // Giảm buffer size để ổn định hơn
const int MAX_RETRIES = 3;
const int RETRY_DELAY_MS = 100;
*/

bool configure_socket_for_large_files(SOCKET sock) {
  std::cout << "[CONFIG] Configuring socket for large file transfer..."
            << std::endl;

  // 1. Set buffer sizes
  int sendBuf = static_cast<int>(ROBUST_BUFFER_SIZE);
  int recvBuf = static_cast<int>(ROBUST_BUFFER_SIZE);

  setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&sendBuf, sizeof(sendBuf));
  setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&recvBuf, sizeof(recvBuf));

  // 2. Set timeouts
  int timeout = ROBUST_TIMEOUT_MS;
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

  // 3. Disable Nagle algorithm
  int noDelay = 1;
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay, sizeof(noDelay));

  // 4. Enable keep-alive
  int keepAlive = 1;
  setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepAlive,
             sizeof(keepAlive));

  std::cout << "[CONFIG] Socket configured successfully" << std::endl;
  return true;
}

bool send_file_size_robust(SOCKET sock, size_t fileSize) {
  // Gửi size dưới dạng 64-bit integer
  uint64_t networkSize = static_cast<uint64_t>(fileSize);

  const char* data = reinterpret_cast<const char*>(&networkSize);
  int totalSent = 0;
  int sizeToSend = sizeof(networkSize);

  while (totalSent < sizeToSend) {
    int sent = send(sock, data + totalSent, sizeToSend - totalSent, 0);
    if (sent == SOCKET_ERROR) {
      std::cout << "[ERROR] Failed to send file size: " << WSAGetLastError()
                << std::endl;
      return false;
    }
    totalSent += sent;
  }

  std::cout << "[INFO] File size sent: " << fileSize << " bytes" << std::endl;
  return true;
}

bool receive_file_size_robust(SOCKET sock, size_t& fileSize) {
  uint64_t networkSize = 0;
  char* data = reinterpret_cast<char*>(&networkSize);
  int totalReceived = 0;
  int sizeToReceive = sizeof(networkSize);

  while (totalReceived < sizeToReceive) {
    int received =
        recv(sock, data + totalReceived, sizeToReceive - totalReceived, 0);
    if (received <= 0) {
      std::cout << "[ERROR] Failed to receive file size: " << WSAGetLastError()
                << std::endl;
      return false;
    }
    totalReceived += received;
  }

  fileSize = static_cast<size_t>(networkSize);
  std::cout << "[INFO] File size received: " << fileSize << " bytes"
            << std::endl;
  return true;
}

bool send_chunk_complete(SOCKET sock, const char* data, size_t size) {
  size_t totalSent = 0;

  while (totalSent < size) {
    size_t remainingBytes = size - totalSent;
    int bytesToSend = static_cast<int>(
        std::min(remainingBytes, static_cast<size_t>(INT_MAX)));

    int result = send(sock, data + totalSent, bytesToSend, 0);

    if (result == SOCKET_ERROR) {
      int error = WSAGetLastError();

      if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
        // Socket buffer full - wait and retry
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      } else if (error == WSAECONNRESET || error == WSAECONNABORTED) {
        std::cout << "[ERROR] Connection lost: " << error << std::endl;
        return false;
      } else {
        std::cout << "[ERROR] Send error: " << error << std::endl;
        return false;
      }
    } else if (result == 0) {
      std::cout << "[ERROR] Connection closed by peer" << std::endl;
      return false;
    }

    totalSent += static_cast<size_t>(result);
  }

  return true;
}

bool receive_chunk_complete(SOCKET sock, char* data, size_t expectedSize,
                            size_t& actualReceived) {
  actualReceived = 0;

  while (actualReceived < expectedSize) {
    size_t remainingBytes = expectedSize - actualReceived;
    int bytesToReceive = static_cast<int>(
        std::min(remainingBytes, static_cast<size_t>(INT_MAX)));

    int result = recv(sock, data + actualReceived, bytesToReceive, 0);

    if (result == SOCKET_ERROR) {
      int error = WSAGetLastError();

      if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      } else {
        std::cout << "[ERROR] Receive error: " << error << std::endl;
        return false;
      }
    } else if (result == 0) {
      // Connection closed - might be normal if we got all data
      std::cout << "[INFO] Connection closed, received: " << actualReceived
                << "/" << expectedSize << std::endl;
      return actualReceived > 0;
    }

    actualReceived += static_cast<size_t>(result);
  }

  return true;
}

bool open_file_with_retry(std::ifstream& file, const std::string& filename,
                          int maxRetries) {
  for (int i = 0; i < maxRetries; i++) {
    file.clear();  // Clear any error flags
    file.open(filename, std::ios::binary);
    if (file.is_open()) {
      std::cout << "[INFO] File opened successfully: " << filename << std::endl;
      return true;
    }

    std::cout << "[RETRY] File open attempt " << (i + 1) << "/" << maxRetries
              << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  std::cout << "[ERROR] Failed to open file after retries: " << filename
            << std::endl;
  return false;
}

void send_zero_size_signal(SOCKET sock) {
  size_t zeroSize = 0;
  send(sock, reinterpret_cast<const char*>(&zeroSize), sizeof(zeroSize), 0);
}

// =====================================================
// MAIN FUNCTIONS - Thay thế functions cũ
// =====================================================

bool send_file_over_socket_robust(SOCKET sock, const std::string& filename) {
  std::cout << "[ROBUST] Starting robust file transfer: " << filename
            << std::endl;

  // Step 1: Configure socket
  configure_socket_for_large_files(sock);

  // Step 2: Open file with retry
  std::ifstream file;
  if (!open_file_with_retry(file, filename, 10)) {
    send_zero_size_signal(sock);
    return false;
  }

  // Step 3: Get file size
  file.seekg(0, std::ios::end);
  size_t fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  if (fileSize == 0) {
    std::cout << "[ERROR] File is empty: " << filename << std::endl;
    send_zero_size_signal(sock);
    file.close();
    return false;
  }

  std::cout << "[INFO] File size: " << fileSize << " bytes ("
            << (fileSize / 1024.0 / 1024.0) << " MB)" << std::endl;

  // Step 4: Send file size
  if (!send_file_size_robust(sock, fileSize)) {
    file.close();
    return false;
  }

  // Step 5: Send file data in chunks
  std::vector<char> buffer(ROBUST_CHUNK_SIZE);
  size_t totalSent = 0;
  size_t progressMilestone =
      std::max(fileSize / 20, static_cast<size_t>(1));  // Minimum 1 byte

  auto startTime = std::chrono::steady_clock::now();

  std::cout << "[INFO] Starting chunked transfer..." << std::endl;

  while (totalSent < fileSize) {
    // Calculate chunk size
    size_t remainingBytes = fileSize - totalSent;
    size_t currentChunkSize = std::min(ROBUST_CHUNK_SIZE, remainingBytes);

    // Read chunk from file
    file.read(buffer.data(), currentChunkSize);
    std::streamsize bytesRead = file.gcount();

    if (bytesRead <= 0) {
      std::cout << "[ERROR] Failed to read from file at position " << totalSent
                << std::endl;
      file.close();
      return false;
    }

    // Send chunk
    if (!send_chunk_complete(sock, buffer.data(),
                             static_cast<size_t>(bytesRead))) {
      std::cout << "[ERROR] Failed to send chunk at position " << totalSent
                << std::endl;
      file.close();
      return false;
    }

    totalSent += static_cast<size_t>(bytesRead);

    // Progress report
    if (totalSent % progressMilestone == 0 || totalSent == fileSize) {
      auto now = std::chrono::steady_clock::now();
      auto elapsed =
          std::chrono::duration_cast<std::chrono::seconds>(now - startTime)
              .count();

      int percent = static_cast<int>((totalSent * 100) / fileSize);
      double mbSent = totalSent / (1024.0 * 1024.0);
      double speed = (elapsed > 0) ? (mbSent / elapsed) : 0.0;

      std::cout << "[PROGRESS] " << percent << "% (" << mbSent
                << " MB) - Speed: " << speed << " MB/s" << std::endl;
    }

    // Flow control for large files
    if (fileSize > 10 * 1024 * 1024 && currentChunkSize == ROBUST_CHUNK_SIZE) {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
  }

  file.close();

  std::cout << "[SUCCESS] File sent completely: " << totalSent << " bytes"
            << std::endl;
  return true;
}

bool receive_file_from_socket_robust(SOCKET sock, const std::string& filename) {
  std::cout << "[ROBUST] Starting robust file receive: " << filename
            << std::endl;

  // Step 1: Configure socket
  configure_socket_for_large_files(sock);

  // Step 2: Create directory if needed
  std::filesystem::create_directories(
      std::filesystem::path(filename).parent_path());

  // Step 3: Receive file size
  size_t fileSize = 0;
  if (!receive_file_size_robust(sock, fileSize)) {
    return false;
  }

  if (fileSize == 0) {
    std::cout << "[INFO] No file to receive (size = 0)" << std::endl;
    return true;
  }

  std::cout << "[INFO] Expecting: " << fileSize << " bytes ("
            << (fileSize / 1024.0 / 1024.0) << " MB)" << std::endl;

  // Step 4: Open output file
  std::ofstream outFile(filename, std::ios::binary);
  if (!outFile.is_open()) {
    std::cout << "[ERROR] Cannot create file: " << filename << std::endl;
    return false;
  }

  // Step 5: Receive file data in chunks
  std::vector<char> buffer(ROBUST_CHUNK_SIZE);
  size_t totalReceived = 0;
  size_t progressMilestone = std::max(fileSize / 20, static_cast<size_t>(1));

  auto startTime = std::chrono::steady_clock::now();

  std::cout << "[INFO] Starting chunked receive..." << std::endl;

  while (totalReceived < fileSize) {
    // Calculate expected chunk size
    size_t remainingBytes = fileSize - totalReceived;
    size_t expectedChunkSize = std::min(ROBUST_CHUNK_SIZE, remainingBytes);

    // Receive chunk
    size_t actualReceived = 0;
    if (!receive_chunk_complete(sock, buffer.data(), expectedChunkSize,
                                actualReceived)) {
      std::cout << "[ERROR] Failed to receive chunk at position "
                << totalReceived << std::endl;
      outFile.close();
      return false;
    }

    if (actualReceived == 0) {
      std::cout << "[ERROR] No data received in chunk" << std::endl;
      break;
    }

    // Write to file
    outFile.write(buffer.data(), actualReceived);
    if (!outFile.good()) {
      std::cout << "[ERROR] Failed to write to file at position "
                << totalReceived << std::endl;
      outFile.close();
      return false;
    }

    totalReceived += actualReceived;

    // Progress report
    if (totalReceived % progressMilestone == 0 || totalReceived == fileSize) {
      auto now = std::chrono::steady_clock::now();
      auto elapsed =
          std::chrono::duration_cast<std::chrono::seconds>(now - startTime)
              .count();

      int percent = static_cast<int>((totalReceived * 100) / fileSize);
      double mbReceived = totalReceived / (1024.0 * 1024.0);
      double speed = (elapsed > 0) ? (mbReceived / elapsed) : 0.0;

      std::cout << "[PROGRESS] " << percent << "% (" << mbReceived
                << " MB) - Speed: " << speed << " MB/s" << std::endl;
    }
  }

  outFile.flush();
  outFile.close();

  // Verify file size
  if (std::filesystem::exists(filename)) {
    auto actualSize = std::filesystem::file_size(filename);
    if (actualSize == fileSize) {
      std::cout << "[SUCCESS] File received completely: " << filename << " ("
                << actualSize << " bytes)" << std::endl;
      return true;
    } else {
      std::cout << "[ERROR] Size mismatch: expected " << fileSize << ", got "
                << actualSize << std::endl;
      return false;
    }
  }

  std::cout << "[ERROR] File was not created: " << filename << std::endl;
  return false;
}


// Gửi file qua socket
bool send_file_over_socket(SOCKET sock, const std::string& filename) {
  return send_file_over_socket_robust(sock, filename);
}

// Nhận file từ socket
void receive_file_from_socket(SOCKET sock, const std::string& filename) {
  receive_file_from_socket_robust(sock, filename);
}

// Gửi file qua email (wrapper cho send_email_with_attachment)
void send_file_via_email(const std::string& toEmail, const std::string& subject,
                         const std::string& body, const std::string& filepath) {
  send_email_with_attachment(toEmail, subject, body, filepath);
}