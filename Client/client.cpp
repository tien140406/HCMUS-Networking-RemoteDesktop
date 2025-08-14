#include "ClientHandle/checkCommand.h"
#include "ClientHandle/commandUtils.h"
#include "ClientUI/CreateUI.h"
#include <csignal>
#include <atomic>
#include <thread>
#include <iostream>
#include <curl/curl.h>

static std::atomic<bool> running(true);

void signal_handler(int) {
    running.store(false);
}

int main() {
    create_client_UI();
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::cout << "[Client] Starting email monitoring..." << std::endl;

    while (running.load()) {
        auto commands = fetch_email_commands();
        for (auto &[cmd, senderEmail] : commands) {
            std::cout << "[Client] Processing command from " << senderEmail << ": " << cmd << std::endl;
            process_command(cmd, senderEmail);
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    curl_global_cleanup();
    std::cout << "[Client] Client stopped." << std::endl;
    return 0;
}
