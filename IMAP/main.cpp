#include "C:/Users/Tien/HCMUS-Networking-RemoteDesktop/Application/lib.h"
#include "C:/Users/Tien/HCMUS-Networking-RemoteDesktop/Application/checkCommand.h"

int main() {
  curl_global_init(CURL_GLOBAL_DEFAULT);

  check_email_commands();
  curl_global_cleanup();

  system("pause");
  return 0;
}