#include "../Application/lib.h"
#include "../Application/executeCommand.h"

static string trim(const string& s) {
  size_t a = s.find_first_not_of(" \r\n\t");
  if (a == string::npos) return "";
  size_t b = s.find_last_not_of(" \r\n\t");
  return s.substr(a, b - a + 1);
}

int main() {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    cerr << "[Server] WSAStartup error: " << WSAGetLastError() << endl;
    return 1;
  }

  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == INVALID_SOCKET) {
    cerr << "[Server] Cannot create socket: " << WSAGetLastError() << endl;
    WSACleanup();
    return 1;
  }

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8888);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    cerr << "[Server] Bind error: " << WSAGetLastError() << endl;
    closesocket(serverSocket);
    WSACleanup();
    return 1;
  }

  if (listen(serverSocket, 5) == SOCKET_ERROR) {
    cerr << "[Server] Listen error: " << WSAGetLastError() << endl;
    closesocket(serverSocket);
    WSACleanup();
    return 1;
  }

  cout << "[Server] Listening on port 8888...\n";

  while (true) {
    sockaddr_in clientAddr{};
    int clientSize = sizeof(clientAddr);
    int clientSocket =
        accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
    if (clientSocket == INVALID_SOCKET) {
      cerr << "[Server] Accept error: " << WSAGetLastError() << endl;
      continue;
    }

    cout << "[Server] Client connected.\n";

    char buffer[4096] = {};
    int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
      cerr << "[Server] Receive failed or connection closed.\n";
      closesocket(clientSocket);
      continue;
    }
    buffer[bytes] = '\0';
    string payload = buffer;

    std::istringstream iss(payload);
    string sender_email;
    if (!std::getline(iss, sender_email)) {
      cerr << "[Server] Malformed payload: missing sender\n";
      string nack = "Malformed payload";
      send(clientSocket, nack.c_str(), static_cast<int>(nack.size()), 0);
      closesocket(clientSocket);
      continue;
    }
    sender_email = trim(sender_email);
    cout << "[Server] Sender: " << sender_email << endl;

    string line;
    while (std::getline(iss, line)) {
      line = trim(line);
      if (line.empty()) continue;
      cout << "[Server] Executing command: " << line << " for " << sender_email
           << endl;
      execute_command_with_sender(sender_email, line);
    }

    string ack = "Commands processed";
    send(clientSocket, ack.c_str(), static_cast<int>(ack.size()), 0);
    closesocket(clientSocket);
    cout << "[Server] Connection closed.\n";
  }

  closesocket(serverSocket);
  WSACleanup();
  return 0;
}