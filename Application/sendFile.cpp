#include "sendFile.h"
#include "sendEmail.h"
#include <filesystem>

const int BUFFER_SIZE = 8192;

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

  // Gửi file size trước
  if (send(sock, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize),
           0) == SOCKET_ERROR) {
    std::cout << "[ERROR] Failed to send file size" << std::endl;
    return false;
  }

  // Gửi data với buffer
  std::vector<char> buffer(BUFFER_SIZE);
  size_t totalSent = 0;

  while (file && totalSent < fileSize) {
    file.read(buffer.data(), BUFFER_SIZE);
    std::streamsize bytesRead = file.gcount();

    if (bytesRead > 0) {
      int result = send(sock, buffer.data(), static_cast<int>(bytesRead), 0);
      if (result == SOCKET_ERROR) {
        std::cout << "[ERROR] Send failed at byte: " << totalSent << std::endl;
        return false;
      }
      totalSent += result;

      // Progress cho file lớn
      if (fileSize > 1024 * 1024 && totalSent % (1024 * 1024) == 0) {
        std::cout << "[PROGRESS] Sent: " << (totalSent / (1024 * 1024)) << " MB"
                  << std::endl;
      }
    }
  }

  std::cout << "[SUCCESS] File sent: " << totalSent << " bytes" << std::endl;
  return true;
}

// Nhận file từ socket
void receive_file_from_socket(SOCKET sock, const std::string& filename) {
  // Tạo thư mục nếu chưa có
  std::filesystem::create_directories(
      std::filesystem::path(filename).parent_path());

  size_t fileSize;
  int received =
      recv(sock, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);
  if (received <= 0 || fileSize == 0) {
    std::cerr << "[Client] No file received or connection error.\n";
    return;
  }

  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "[Client] Cannot create file: " << filename << "\n";
    return;
  }

  std::vector<char> buffer(BUFFER_SIZE);
  size_t bytesReceived = 0;

  std::cout << "[Client] Receiving file: " << filename << " (" << fileSize
            << " bytes)\n";

  while (bytesReceived < fileSize) {
    size_t remaining = fileSize - bytesReceived;
    size_t chunkSize = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;

    int chunk = recv(sock, buffer.data(), static_cast<int>(chunkSize), 0);
    if (chunk <= 0) {
      std::cerr << "[Client] Connection lost while receiving file\n";
      break;
    }

    file.write(buffer.data(), chunk);
    bytesReceived += chunk;

    // Progress cho file lớn
    if (fileSize > 1024 * 1024 && bytesReceived % (1024 * 1024) == 0) {
      std::cout << "[PROGRESS] Received: " << (bytesReceived / (1024 * 1024))
                << " MB\n";
    }
  }

  file.close();
  std::cout << "[Client] File received successfully: " << filename << " ("
            << bytesReceived << " bytes)\n";
}

// Gửi file qua email (wrapper cho send_email_with_attachment)
void send_file_via_email(const std::string& toEmail, const std::string& subject,
                         const std::string& body, const std::string& filepath) {
  send_email_with_attachment(toEmail, subject, body, filepath);
}