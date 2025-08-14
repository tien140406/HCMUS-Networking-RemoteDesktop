#include "ClientHandle/checkCommand.h"
#include "ClientHandle/commandUtils.h"
#include "ClientUI/CreateUI.h"
#include "ClientUI/AdminUI/RemoteAdminUI.h"
#include <csignal>
#include <atomic>
#include <thread>
#include <iostream>
#include <curl/curl.h>

RemoteAdminUI g_ui;
std::atomic<bool> running(true);

void signal_handler(int) {
    running.store(false);
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    run_ui();

    curl_global_cleanup();
    std::cout << "[Client] Client stopped." << std::endl;
    return 0;
}
