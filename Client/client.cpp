#include "../Application/lib.h"
#include "../Application/checkCommand.h"

static std::atomic<bool> running(true);

void signal_handler(int) { running.store(false); }

int main() {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  curl_global_init(CURL_GLOBAL_DEFAULT);
  cout << "[Agent] Email polling agent started." << endl;

  while (running.load()) {
    check_email_commands();
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }

  cout << "[Agent] Shutting down." << endl;
  curl_global_cleanup();
  return 0;
}