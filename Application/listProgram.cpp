#include "listProgram.h"
#include <filesystem>
#include <set>

// Hàm để lấy danh sách các chương trình đang chạy (running programs)
// Hàm để lấy danh sách các chương trình đang chạy (running programs - chỉ GUI
// apps)
void list_programs_to_file(const string& outFilePath) {
  namespace fs = std::filesystem;

  // Tạo thư mục nếu chưa có
  fs::create_directories(fs::path(outFilePath).parent_path());

  ofstream outFile(outFilePath);
  if (!outFile) {
    cerr << "[Server] Failed to create output file: " << outFilePath << endl;
    return;
  }

  outFile << "=== Running Programs ===" << endl;

  // Sử dụng set để tránh trùng lặp tên chương trình
  std::set<string> uniquePrograms;

  // Enumerate all visible windows
  EnumWindows(
      [](HWND hwnd, LPARAM lParam) -> BOOL {
        std::set<string>* programs =
            reinterpret_cast<std::set<string>*>(lParam);

        // Chỉ lấy window có thể nhìn thấy và không phải là child window
        if (!IsWindowVisible(hwnd) || GetParent(hwnd) != NULL) {
          return TRUE;  // Continue enumeration
        }

        // Lấy window title để loại bỏ các window trống hoặc hệ thống
        char windowTitle[256];
        GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));

        // Bỏ qua các window không có title hoặc có title rỗng
        if (strlen(windowTitle) == 0) {
          return TRUE;
        }

        // Lấy process ID từ window handle
        DWORD processId;
        GetWindowThreadProcessId(hwnd, &processId);

        // Mở process để lấy tên executable
        HANDLE hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        if (hProcess) {
          char processPath[MAX_PATH];
          DWORD pathSize = sizeof(processPath);

          // Lấy tên file executable
          if (QueryFullProcessImageNameA(hProcess, 0, processPath, &pathSize)) {
            // Lấy tên file từ đường dẫn đầy đủ
            string fullPath(processPath);
            size_t lastSlash = fullPath.find_last_of("\\/");
            string processName = (lastSlash != string::npos)
                                     ? fullPath.substr(lastSlash + 1)
                                     : fullPath;

            // Loại bỏ các process hệ thống và background
            if (processName != "dwm.exe" && processName != "winlogon.exe" &&
                processName != "csrss.exe" &&
                processName != "explorer.exe" &&  // Sẽ được xử lý riêng
                processName != "sihost.exe" && processName != "ctfmon.exe" &&
                processName != "SearchUI.exe" &&
                processName != "StartMenuExperienceHost.exe" &&
                processName != "ShellExperienceHost.exe" &&
                processName != "ApplicationFrameHost.exe" &&
                processName.find("svchost.exe") == string::npos) {
              // Xử lý đặc biệt cho Windows Explorer
              if (processName == "explorer.exe") {
                programs->insert("Windows Explorer");
              } else {
                // Loại bỏ extension .exe để hiển thị đẹp hơn
                if (processName.length() > 4 &&
                    processName.substr(processName.length() - 4) == ".exe") {
                  processName = processName.substr(0, processName.length() - 4);
                }
                programs->insert(processName);
              }
            }
          }
          CloseHandle(hProcess);
        }

        return TRUE;  // Continue enumeration
      },
      reinterpret_cast<LPARAM>(&uniquePrograms));

  // Ghi danh sách unique programs
  if (uniquePrograms.empty()) {
    outFile << "No running programs found." << endl;
  } else {
    for (const auto& program : uniquePrograms) {
      outFile << "- " << program << endl;
    }
  }

  outFile << "==========================" << endl;
  outFile.close();

  cout << "[Server] Running programs list saved to: "
       << fs::absolute(outFilePath) << endl;
}

// Hàm để lấy danh sách các process đang chạy (với PID chi tiết)
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
    outFile << "Failed to create process snapshot." << endl;
    outFile.close();
    return;
  }

  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  outFile << "=== Running Processes (with PID) ===" << endl;

  if (Process32First(hSnapshot, &pe32)) {
    do {
      // Convert WCHAR to string
#ifdef UNICODE
      wstring ws(pe32.szExeFile);
      string processName(ws.begin(), ws.end());
#else
      string processName(pe32.szExeFile);
#endif

      outFile << "- " << processName << " (PID: " << pe32.th32ProcessID << ")"
              << endl;

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

// Hàm phụ trợ để lấy danh sách installed programs (nếu cần)
void list_installed_programs_to_file(const string& outFilePath) {
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

  cout << "[Server] Installed programs list saved to: "
       << fs::absolute(outFilePath) << endl;
}