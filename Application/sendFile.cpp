#include "sendFile.h"
#include "sendEmail.h"
#include <filesystem>

const int BUFFER_SIZE =  64 * 1024;  // Giảm buffer size để ổn định hơn
const int MAX_RETRIES = 3;
const int RETRY_DELAY_MS = 100;

// Gửi file qua socket
bool send_file_over_socket(SOCKET sock, const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cout << "[ERROR] Cannot open file: " << filename << std::endl;
    size_t fileSize = 0;
    send(sock, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0);
    return false;
  }

  file.seekg(0, std::ios::end);
  size_t fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  std::cout << "[INFO] Sending file: " << filename << " (" << fileSize
            << " bytes)" << std::endl;

  // Kiểm tra kích thước file
  const size_t maxSize = 50 * 1024 * 1024;  // 50MB limit
  if (fileSize > maxSize) {
    std::cout << "[ERROR] File too large: " << fileSize
              << " bytes (max: " << maxSize << ")" << std::endl;
    size_t zeroSize = 0;
    send(sock, reinterpret_cast<const char*>(&zeroSize), sizeof(zeroSize), 0);
    return false;
  }

  // Gửi file size trước
  if (send(sock, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize),
           0) == SOCKET_ERROR) {
    std::cout << "[ERROR] Failed to send file size. Error: "
              << WSAGetLastError() << std::endl;
    return false;
  }

  // Gửi data với buffer nhỏ hơn và retry
  std::vector<char> buffer(BUFFER_SIZE);
  size_t totalSent = 0;
  size_t progressMilestone = fileSize / 10;  // Progress mỗi 10%

  while (file && totalSent < fileSize) {
    file.read(buffer.data(), BUFFER_SIZE);
    std::streamsize bytesRead = file.gcount();

    if (bytesRead > 0) {
      bool sent = false;
      int retries = 0;

      while (!sent && retries < MAX_RETRIES) {
        int result = send(sock, buffer.data(), static_cast<int>(bytesRead), 0);

        if (result == SOCKET_ERROR) {
          int error = WSAGetLastError();
          std::cout << "[WARN] Send failed at byte " << totalSent
                    << ", error: " << error << ", retry: " << (retries + 1)
                    << std::endl;

          if (error == WSAECONNRESET || error == WSAECONNABORTED ||
              error == WSAENETRESET) {
            std::cout << "[ERROR] Connection lost, cannot continue"
                      << std::endl;
            return false;
          }

          retries++;
          std::this_thread::sleep_for(
              std::chrono::milliseconds(RETRY_DELAY_MS * retries));
        } else {
          sent = true;
          totalSent += result;

          // Progress report
          if (progressMilestone > 0 && totalSent % progressMilestone == 0) {
            // int percent = static_cast<int>((totalSent * 100) / fileSize);
            // std::cout << "[PROGRESS] Sent: " << percent << "% ("
            //           << (totalSent / 1024) << " KB)" << std::endl;
            std::cout << "[DEBUG] Chunk sent: " << result << " bytes, Total: " << totalSent 
          << "/" << fileSize << " (" << (totalSent * 100.0 / fileSize) << "%)" << std::endl;
          }
        }
      }

      if (!sent) {
        std::cout << "[ERROR] Failed to send after " << MAX_RETRIES
                  << " retries" << std::endl;
        return false;
      }
    }

    // Thêm delay nhỏ để tránh flood network
    if (fileSize > 1024 * 1024) {  // Chỉ delay với file > 1MB
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  std::cout << "[SUCCESS] File sent completely: " << totalSent << " bytes"
            << std::endl;
  if (totalSent != fileSize) {
      std::cout << "[ERROR] Transfer incomplete! Sent: " << totalSent 
                << ", Expected: " << fileSize << std::endl;
      return false;
  }
  return true;
}

// Nhận file từ socket
void receive_file_from_socket(SOCKET sock, const std::string& filename) {
  // Set receive timeout
  int timeout = 600000;  // 5 minutes in milliseconds
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

  // Tạo thư mục nếu chưa có
  std::filesystem::create_directories(
      std::filesystem::path(filename).parent_path());

  size_t fileSize;
  int received =
      recv(sock, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);
  if (received <= 0 || fileSize == 0) {
    std::cerr << "[Client] No file received or connection error. Received: "
              << received << ", FileSize: " << fileSize << "\n";
    return;
  }

  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "[Client] Cannot create file: " << filename << "\n";
    return;
  }

  std::vector<char> buffer(BUFFER_SIZE);
  size_t bytesReceived = 0;
  size_t progressMilestone = fileSize / 10;  // Progress mỗi 10%

  std::cout << "[Client] Receiving file: " << filename << " (" << fileSize
            << " bytes)\n";

  auto startTime = std::chrono::steady_clock::now();

  while (bytesReceived < fileSize) {
    size_t remaining = fileSize - bytesReceived;
    size_t chunkSize = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;

    int chunk = recv(sock, buffer.data(), static_cast<int>(chunkSize), 0);
    if (chunk <= 0) {
      int error = WSAGetLastError();
      std::cerr << "[Client] Connection lost while receiving file. Error: "
                << error << ", Received so far: " << bytesReceived << "\n";
      break;
    }

    file.write(buffer.data(), chunk);
    bytesReceived += chunk;

    // Progress report
    if (progressMilestone > 0 && bytesReceived % progressMilestone == 0) {
      int percent = static_cast<int>((bytesReceived * 100) / fileSize);
      auto now = std::chrono::steady_clock::now();
      auto elapsed =
          std::chrono::duration_cast<std::chrono::seconds>(now - startTime)
              .count();
      std::cout << "[PROGRESS] Received: " << percent << "% ("
                << (bytesReceived / 1024) << " KB) - " << elapsed << "s\n";
    }
  }

  file.close();

  if (bytesReceived == fileSize) {
    std::cout << "[Client] File received successfully: " << filename << " ("
              << bytesReceived << " bytes)\n";
  } else {
    std::cout << "[Client] File transfer incomplete: " << filename << " ("
              << bytesReceived << "/" << fileSize << " bytes)\n";
  }
}

// Gửi file qua email (wrapper cho send_email_with_attachment)
void send_file_via_email(const std::string& toEmail, const std::string& subject,
                         const std::string& body, const std::string& filepath) {
  send_email_with_attachment(toEmail, subject, body, filepath);
}