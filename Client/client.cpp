#include "../Application/executeCommand.h"
#include "../Application/checkCommand.h"
#include "clientUI/example.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <map>

#pragma comment(lib, "ws2_32.lib")

const std::string saveDir = "C:/MMT/";

static std::atomic<bool> running(true);

void signal_handler(int) { running.store(false); }

// Helper function to send all data reliably
bool send_all(SOCKET sock, const char* data, size_t size) {
    size_t sent = 0;
    while (sent < size) {
        int result = send(sock, data + sent, static_cast<int>(size - sent), 0);
        if (result == SOCKET_ERROR || result == 0) {
            std::cerr << "[Client] Send failed with error: " << WSAGetLastError() << "\n";
            return false;
        }
        sent += result;
    }
    return true;
}

// Helper function to receive all data reliably
bool recv_all(SOCKET sock, char* buffer, size_t size) {
    size_t received = 0;
    while (received < size) {
        int result = recv(sock, buffer + received, static_cast<int>(size - received), 0);
        if (result == SOCKET_ERROR || result == 0) {
            std::cerr << "[Client] Receive failed or connection closed. Error: " << WSAGetLastError() << "\n";
            return false;
        }
        received += result;
    }
    return true;
}

// Add socket timeout to prevent hanging
void set_socket_timeout(SOCKET sock, int timeout_ms) {
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
}

SOCKET connect_to_server(const std::string &host, int port) {
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    std::cerr << "[Client] Socket creation failed: " << WSAGetLastError()
              << "\n";
    return INVALID_SOCKET;
  }

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);

  if (connect(sock, (sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    std::cerr << "[Client] Connection failed: " << WSAGetLastError() << "\n";
    closesocket(sock);
    return INVALID_SOCKET;
  }

  std::cout << "[Client] Connected to server " << host << ":" << port << "\n";
  return sock;
}

// Thử kết nối với retry
SOCKET connect_with_retry(const std::string &host, int port, int attempts = 3,
                          int delay_ms = 500) {
  for (int i = 0; i < attempts; ++i) {
    SOCKET sock = connect_to_server(host, port);
    if (sock != INVALID_SOCKET) return sock;
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
  }
  return INVALID_SOCKET;
}

// Nhận file và lưu vào saveDir + filename - FIXED VERSION
void receive_file(SOCKET sock, const std::string &filename) {
  std::filesystem::create_directories(saveDir);

  // Receive file size first
  size_t fileSize = 0;
  if (!recv_all(sock, reinterpret_cast<char *>(&fileSize), sizeof(fileSize))) {
    std::cerr << "[Client] Failed to receive file size\n";
    return;
  }

  std::cout << "[Client] Expecting file size: " << fileSize << " bytes\n";

  if (fileSize == 0) {
    std::cerr << "[Client] Server reported error or empty file (size=0)\n";
    return;
  }

  std::string fullpath = saveDir + filename;
  std::ofstream file(fullpath, std::ios::binary);
  if (!file) {
    std::cerr << "[Client] Cannot open output file: " << fullpath << "\n";
    
    // Drain the socket to prevent issues
    char tmp[4096];
    size_t drained = 0;
    while (drained < fileSize) {
      size_t toDrain = std::min(sizeof(tmp), fileSize - drained);
      int chunk = recv(sock, tmp, static_cast<int>(toDrain), 0);
      if (chunk <= 0) break;
      drained += chunk;
    }
    return;
  }

  // Receive file content
  char buffer[4096];
  size_t bytesReceived = 0;
  
  while (bytesReceived < fileSize) {
    size_t remaining = fileSize - bytesReceived;
    size_t toReceive = std::min(sizeof(buffer), remaining);
    
    if (!recv_all(sock, buffer, toReceive)) {
      std::cerr << "[Client] Failed to receive file chunk at offset " << bytesReceived << "\n";
      file.close();
      return;
    }
    
    file.write(buffer, toReceive);
    if (!file) {
      std::cerr << "[Client] Failed to write to file\n";
      file.close();
      return;
    }
    
    bytesReceived += toReceive;
    std::cout << "[Client] Progress: " << bytesReceived << "/" << fileSize << " bytes\n";
  }
  
  file.close();
  
  if (bytesReceived == fileSize) {
    std::cout << "[Client] Successfully received file: " << fullpath << " (" << bytesReceived << " bytes)\n";
  } else {
    std::cerr << "[Client] File transfer incomplete. Received " << bytesReceived << "/" << fileSize << " bytes\n";
  }
}

// map command -> (email subject, local filename)
std::map<std::string, std::pair<std::string, std::string>> fileCommands = {
    {"get_screenshot", {"Screenshot from server", "screenshot.png"}},
    {"get_picture", {"Picture from server", "picture.png"}},
    {"list_program", {"Program list from server", "programs.txt"}},
    {"keylogger", {"Keylogger log from server", "keylog.txt"}}};

void process_command(const std::string &command,
                     const std::string &senderEmail) {
  SOCKET sock = connect_with_retry("192.168.221.38", 8888);
  if (sock == INVALID_SOCKET) {
    std::cerr << "[Client] Could not connect to server after retries\n";
    return;
  }

  // Set 30 second timeout
  set_socket_timeout(sock, 30000);

  std::string payload = senderEmail + "\n" + command + "\n";
  
  // Use send_all for payload too
  if (!send_all(sock, payload.c_str(), payload.size())) {
    std::cerr << "[Client] Failed to send command payload\n";
    closesocket(sock);
    return;
  }

  if (fileCommands.count(command)) {
    auto [subject, localFile] = fileCommands[command];
    receive_file(sock, localFile);
    std::string fullpath = saveDir + localFile;
    
    // Check if file exists and has content before sending email
    if (std::filesystem::exists(fullpath) && std::filesystem::file_size(fullpath) > 0) {
      send_email_with_attachment(senderEmail, subject, "Requested file from server", fullpath);
    } else {
      std::cerr << "[Client] File not received properly, skipping email\n";
    }
  } else {
    execute_command(command);
  }

  closesocket(sock);
}

int main() {
  run_imgui_example(); 
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    std::cerr << "[Client] WSAStartup failed\n";
    return 1;
  }

  curl_global_init(CURL_GLOBAL_DEFAULT);
  std::cout << "[Client] Agent started.\n";

  while (running.load()) {
    auto commands =
        fetch_email_commands();  // phải trả về vector<pair<cmd,sender>>
    for (auto &[cmd, senderEmail] : commands) {
      process_command(cmd, senderEmail);
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  curl_global_cleanup();
  WSACleanup();
  return 0;
}