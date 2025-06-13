#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    cerr << "[Server] WSAStartup error: " << WSAGetLastError() << endl;
    return 1;
  }

  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == INVALID_SOCKET) {
    cerr << "[Server] Can not create socket: " << WSAGetLastError() << endl;
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
