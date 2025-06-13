#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
  WSADATA wsa;
  WSAStartup(MAKEWORD(2, 2), &wsa);

  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8888);
  inet_pton(AF_INET, "127.0.0.1",
            &serverAddr.sin_addr);

  connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
  cout << "Connected to server\n";

  while (true) {
    string command;
    cout << "Enter command (or 'exit'): ";
    getline(cin, command);

    if (command == "exit") break;

    send(clientSocket, command.c_str(), command.length(), 0);

    char buffer[1024] = {};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
      cout << "[Server's respond] " << buffer << endl;
    }
  }

  closesocket(clientSocket);
  WSACleanup();
  return 0;
}
