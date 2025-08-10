#include "listProgram.h"
#include <filesystem>
#include <set>

// Hàm để lấy danh sách các chương trình được cài đặt
void list_programs() {
  const string filename = "program_list.txt";
  ofstream outFile(filename);
  if (!outFile) {
    cerr << "Failed to create output file." << endl;
    return;
  }

  outFile << "=== Installed Programs ===" << endl;

  // Lấy danh sách từ registry
  HKEY hKey;
  const char* subkey =
      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ, &hKey) ==
      ERROR_SUCCESS) {
    DWORD index = 0;
    char keyName[256];
    DWORD keyNameSize;

    while (true) {
      keyNameSize = sizeof(keyName);
      if (RegEnumKeyExA(hKey, index++, keyName, &keyNameSize, NULL, NULL, NULL,
                        NULL) != ERROR_SUCCESS) {
        break;
      }

      // Mở subkey để lấy thông tin
      HKEY hSubKey;
      if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) ==
          ERROR_SUCCESS) {
        char displayName[256] = {0};
        DWORD displayNameSize = sizeof(displayName);
        DWORD valueType;

        // Lấy tên hiển thị của chương trình
        if (RegQueryValueExA(hSubKey, "DisplayName", NULL, &valueType,
                             (LPBYTE)displayName,
                             &displayNameSize) == ERROR_SUCCESS) {
          if (strlen(displayName) > 0) {
            outFile << "- " << displayName << endl;
          }
        }
        RegCloseKey(hSubKey);
      }
    }
    RegCloseKey(hKey);
  } else {
    outFile << "Failed to access registry." << endl;
  }

  outFile << "==========================" << endl;
  outFile.close();

  cout << "Program list written to: " << std::filesystem::absolute(filename)
       << endl;

  Sleep(1000);
  ifstream test_file("program_list.txt");
  if (test_file.good()) {
    test_file.close();
    send_email_with_attachment(
        "serverbottestmmt@gmail.com", "Installed Program List",
        "Attached is the list of installed programs.", "program_list.txt");
  } else {
    cerr << "Failed to create program list file" << endl;
  }
}

// Hàm để lấy danh sách các process đang chạy
void list_processes() {
  const string filename = "process_list.txt";
  ofstream outFile(filename);
  if (!outFile) {
    cerr << "Failed to create output file." << endl;
    return;
  }

  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE) {
    cerr << "Failed to create process snapshot." << endl;
    return;
  }

  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  outFile << "=== Running Processes ===" << endl;
  if (Process32First(hSnapshot, &pe32)) {
    do {
      // Convert WCHAR to std::string (ASCII-safe)
      string processName;
      for (int i = 0; pe32.szExeFile[i] != '\0'; ++i) {
        processName += static_cast<char>(pe32.szExeFile[i]);
      }

      outFile << "- " << processName << " (PID: " << pe32.th32ProcessID << ")"
              << endl;

    } while (Process32Next(hSnapshot, &pe32));
  } else {
    outFile << "Failed to retrieve process list." << endl;
  }

  CloseHandle(hSnapshot);
  outFile << "==========================" << endl;
  outFile.close();

  cout << "Process list written to: " << std::filesystem::absolute(filename)
       << endl;

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

  if (!std::filesystem::remove(filename))
    cerr << "Failed to delete the file." << endl;
}

void list_programs_to_file(const string& outFilePath) {
  namespace fs = std::filesystem;

  // Tạo thư mục nếu chưa có
  fs::create_directories(fs::path(outFilePath).parent_path());

  ofstream outFile(outFilePath);
  if (!outFile) {
    cerr << "[Server] Failed to create output file: " << outFilePath << endl;
    return;
  }

  outFile << "=== Installed Programs ===" << endl;

  // Lấy danh sách từ registry
  HKEY hKey;
  const char* subkey =
      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ, &hKey) ==
      ERROR_SUCCESS) {
    DWORD index = 0;
    char keyName[256];
    DWORD keyNameSize;

    while (true) {
      keyNameSize = sizeof(keyName);
      if (RegEnumKeyExA(hKey, index++, keyName, &keyNameSize, NULL, NULL, NULL,
                        NULL) != ERROR_SUCCESS) {
        break;
      }

      // Mở subkey để lấy thông tin
      HKEY hSubKey;
      if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) ==
          ERROR_SUCCESS) {
        char displayName[256] = {0};
        DWORD displayNameSize = sizeof(displayName);
        DWORD valueType;

        // Lấy tên hiển thị của chương trình
        if (RegQueryValueExA(hSubKey, "DisplayName", NULL, &valueType,
                             (LPBYTE)displayName,
                             &displayNameSize) == ERROR_SUCCESS) {
          if (strlen(displayName) > 0) {
            outFile << "- " << displayName << endl;
          }
        }
        RegCloseKey(hSubKey);
      }
    }
    RegCloseKey(hKey);
  } else {
    outFile << "Failed to access registry." << endl;
  }

  outFile << "==========================" << endl;
  outFile.close();

  cout << "[Server] Program list saved to: " << fs::absolute(outFilePath)
       << endl;
}

void list_processes_to_file(const string& outFilePath) {
  namespace fs = std::filesystem;

  // Tạo thư mục nếu chưa có
  fs::create_directories(fs::path(outFilePath).parent_path());

  ofstream outFile(outFilePath);
  if (!outFile) {
    cerr << "[Server] Failed to create output file: " << outFilePath << endl;
    return;
  }

  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE) {
    cerr << "[Server] Failed to create process snapshot." << endl;
    return;
  }

  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  outFile << "=== Running Processes ===" << endl;
  if (Process32First(hSnapshot, &pe32)) {
    do {
#ifdef UNICODE
      wstring ws(pe32.szExeFile);
      string processName(ws.begin(), ws.end());
#else
      string processName(pe32.szExeFile);
#endif
      outFile << "- " << processName << " (PID: " << pe32.th32ProcessID
              << ")\n";
    } while (Process32Next(hSnapshot, &pe32));
  } else {
    outFile << "Failed to retrieve process list." << endl;
  }

  CloseHandle(hSnapshot);
  outFile << "==========================" << endl;
  outFile.close();

  cout << "[Server] Process list saved to: " << fs::absolute(outFilePath)
       << endl;
}