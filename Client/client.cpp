<<<<<<< Updated upstream
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
  WSADATA wsa;
  WSAStartup(MAKEWORD(2, 2), &wsa);

  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
=======
#include "../Application/lib.h"
#include "../Application/checkCommand.h"
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

<<<<<<< Updated upstream
=======
// Helper function to send all data reliably
bool send_all(SOCKET sock, const char *data, size_t size) {
  size_t sent = 0;
  while (sent < size) {
    int result = send(sock, data + sent, static_cast<int>(size - sent), 0);
    if (result == SOCKET_ERROR || result == 0) {
      std::cerr << "[Client] Send failed with error: " << WSAGetLastError()
                << "\n";
      return false;
    }
    sent += result;
  }
  return true;
}

// Helper function to receive all data reliably
bool recv_all(SOCKET sock, char *buffer, size_t size) {
  size_t received = 0;
  while (received < size) {
    int result =
        recv(sock, buffer + received, static_cast<int>(size - received), 0);
    if (result == SOCKET_ERROR || result == 0) {
      std::cerr << "[Client] Receive failed or connection closed. Error: "
                << WSAGetLastError() << "\n";
      return false;
    }
    received += result;
  }
  return true;
}

// Add socket timeout to prevent hanging
void set_socket_timeout(SOCKET sock, int timeout_ms) {
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_ms,
             sizeof(timeout_ms));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout_ms,
             sizeof(timeout_ms));
}

>>>>>>> Stashed changes
// Kết nối tới server (không gọi WSAStartup ở đây)
SOCKET connect_to_server(const std::string &host, int port) {
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    std::cerr << "[Client] Socket creation failed: " << WSAGetLastError()
              << "\n";
    return INVALID_SOCKET;
  }
>>>>>>> Stashed changes

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8888);
  inet_pton(AF_INET, "127.0.0.1",
            &serverAddr.sin_addr);

<<<<<<< Updated upstream
  connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
  cout << "Connected to server\n";
=======
  if (connect(sock, (sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    std::cerr << "[Client] Connection failed: " << WSAGetLastError() << "\n";
    closesocket(sock);
    return INVALID_SOCKET;
  }
>>>>>>> Stashed changes

  while (true) {
    string command;
    cout << "Enter command (or 'exit'): ";
    getline(cin, command);

<<<<<<< Updated upstream
    if (command == "exit") break;

    send(clientSocket, command.c_str(), command.length(), 0);

    char buffer[1024] = {};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
      cout << "[Server's respond] " << buffer << endl;
=======
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

<<<<<<< Updated upstream
// Nhận file và lưu vào saveDir + filename
void receive_file(SOCKET sock, const std::string &filename) {
  std::filesystem::create_directories(saveDir);

  size_t fileSize = 0;
  int ret =
      recv(sock, reinterpret_cast<char *>(&fileSize), sizeof(fileSize), 0);
  if (ret <= 0) {
    std::cerr << "[Client] Failed to read file size or connection closed.\n";
    return;
  }

  if (fileSize == 0) {
    std::cerr << "[Client] No file received (size=0).\n";
    return;
  }

  std::string fullpath = saveDir + filename;
  std::ofstream file(fullpath, std::ios::binary);
  if (!file) {
    std::cerr << "[Client] Cannot open output file: " << fullpath << "\n";
    // drain
    char tmp[4096];
    size_t drained = 0;
    while (drained < fileSize) {
      int chunk = recv(sock, tmp, sizeof(tmp), 0);
      if (chunk <= 0) break;
      drained += chunk;
    }
    return;
  }

=======
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
>>>>>>> Stashed changes
  char buffer[4096];
  size_t bytesReceived = 0;
  while (bytesReceived < fileSize) {
    size_t remaining = fileSize - bytesReceived;
    size_t toReceive = std::min(sizeof(buffer), remaining);

    if (!recv_all(sock, buffer, toReceive)) {
      std::cerr << "[Client] Failed to receive file chunk at offset "
                << bytesReceived << "\n";
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
    std::cout << "[Client] Progress: " << bytesReceived << "/" << fileSize
              << " bytes\n";
  }
<<<<<<< Updated upstream
  file.close();
  std::cout << "[Client] File received: " << fullpath << " (" << bytesReceived
            << " bytes)\n";
=======

  file.close();

  if (bytesReceived == fileSize) {
    std::cout << "[Client] Successfully received file: " << fullpath << " ("
              << bytesReceived << " bytes)\n";
  } else {
    std::cerr << "[Client] File transfer incomplete. Received " << bytesReceived
              << "/" << fileSize << " bytes\n";
  }
>>>>>>> Stashed changes
}

// map command -> (email subject, local filename)
std::map<std::string, std::pair<std::string, std::string>> fileCommands = {
    {"get_screenshot", {"Screenshot from server", "screenshot.png"}},
    {"get_picture", {"Picture from server", "picture.png"}},
    {"list_program", {"Program list from server", "programs.txt"}},
<<<<<<< Updated upstream
=======
    {"list_process",
     {"Process list from server", "processes.txt"}},  // <-- thêm mới
>>>>>>> Stashed changes
    {"keylogger", {"Keylogger log from server", "keylog.txt"}}};

void process_command(const std::string &command,
                     const std::string &senderEmail) {
<<<<<<< Updated upstream
  SOCKET sock = connect_with_retry("192.168.221.48", 8888);
=======
  SOCKET sock = connect_with_retry("127.0.0.1", 8888);
>>>>>>> Stashed changes
  if (sock == INVALID_SOCKET) {
    std::cerr << "[Client] Could not connect to server after retries\n";
    return;
  }

<<<<<<< Updated upstream
  std::string payload = senderEmail + "\n" + command + "\n";
  send(sock, payload.c_str(), static_cast<int>(payload.size()), 0);
=======
  set_socket_timeout(sock, 30000);

  // Gửi email + command cho server xử lý
  std::string payload = senderEmail + "\n" + command + "\n";
  if (!send_all(sock, payload.c_str(), payload.size())) {
    std::cerr << "[Client] Failed to send command payload\n";
    closesocket(sock);
    return;
  }
>>>>>>> Stashed changes

  if (fileCommands.count(command)) {
    // Các command nhận file bình thường
    auto [subject, localFile] = fileCommands[command];
    receive_file(sock, localFile);
    std::string fullpath = saveDir + localFile;
<<<<<<< Updated upstream
    send_email_with_attachment(senderEmail, subject,
                               "Requested file from server", fullpath);
=======

    if (std::filesystem::exists(fullpath) &&
        std::filesystem::file_size(fullpath) > 0) {
      send_email_with_attachment(senderEmail, subject,
                                 "Requested file from server", fullpath);
    } else {
      std::cerr << "[Client] File not received properly, skipping email\n";
    }
  } else if (command.rfind("send_file", 0) == 0) {
    // Xử lý riêng cho send_file <path>
    std::string filePath = command.substr(9);
    filePath.erase(0, filePath.find_first_not_of(" \t"));  // trim

    std::string filename = std::filesystem::path(filePath).filename().string();
    receive_file(sock, filename);

    std::string fullpath = saveDir + filename;
    if (std::filesystem::exists(fullpath) &&
        std::filesystem::file_size(fullpath) > 0) {
      send_email_with_attachment(senderEmail, "Requested file from server",
                                 "Requested file from server", fullpath);
    } else {
      std::cerr << "[Client] File not received properly, skipping email\n";
    }
>>>>>>> Stashed changes
  } else {
    execute_command(command);
  }

  closesocket(sock);
}

int main() {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    std::cerr << "[Client] WSAStartup failed\n";
    return 1;
<<<<<<< Updated upstream
=======
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
>>>>>>> Stashed changes
  }

  curl_global_init(CURL_GLOBAL_DEFAULT);
  std::cout << "[Client] Agent started.\n";

  while (running.load()) {
    auto commands =
        fetch_email_commands();  // phải trả về vector<pair<cmd,sender>>
    for (auto &[cmd, senderEmail] : commands) {
      process_command(cmd, senderEmail);
>>>>>>> Stashed changes
    }
  }

<<<<<<< Updated upstream
  closesocket(clientSocket);
=======
  curl_global_cleanup();
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
  WSACleanup();
  return 0;
}