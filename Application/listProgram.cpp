#include "listProgram.h"
#include <filesystem>

// ========== FUNCTIONS FOR LISTING PROCESSES (đang chạy) ==========

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

  cout << "Process list written to: " << std::filesystem::absolute(filename) << endl;


  Sleep(1000);
<<<<<<< Updated upstream
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
=======
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

// ========== FUNCTIONS FOR LISTING INSTALLED PROGRAMS ==========

void list_programs() {
  const string filename = "installed_programs.txt";
  ofstream outFile(filename);
  if (!outFile) {
    cerr << "Failed to create output file." << endl;
    return;
  }

  outFile << "=== Installed Programs ===" << endl;

  // Scan registry for installed programs
  HKEY hKey;
  const char* subKey =
      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey) ==
      ERROR_SUCCESS) {
    DWORD index = 0;
    char keyName[256];
    DWORD keyNameSize = sizeof(keyName);

    while (RegEnumKeyExA(hKey, index, keyName, &keyNameSize, NULL, NULL, NULL,
                         NULL) == ERROR_SUCCESS) {
      HKEY hSubKey;
      if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) ==
          ERROR_SUCCESS) {
        char displayName[512];
        DWORD displayNameSize = sizeof(displayName);
        DWORD type;

        if (RegQueryValueExA(hSubKey, "DisplayName", NULL, &type,
                             (BYTE*)displayName,
                             &displayNameSize) == ERROR_SUCCESS) {
          char version[256] = "";
          DWORD versionSize = sizeof(version);
          RegQueryValueExA(hSubKey, "DisplayVersion", NULL, &type,
                           (BYTE*)version, &versionSize);

          char publisher[256] = "";
          DWORD publisherSize = sizeof(publisher);
          RegQueryValueExA(hSubKey, "Publisher", NULL, &type, (BYTE*)publisher,
                           &publisherSize);

          outFile << "- " << displayName;
          if (strlen(version) > 0) {
            outFile << " (Version: " << version << ")";
          }
          if (strlen(publisher) > 0) {
            outFile << " [Publisher: " << publisher << "]";
          }
          outFile << endl;
        }
        RegCloseKey(hSubKey);
      }

      index++;
      keyNameSize = sizeof(keyName);
    }
    RegCloseKey(hKey);
  }

  // Also check 32-bit programs on 64-bit systems
  const char* subKey32 =
      "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey32, 0, KEY_READ, &hKey) ==
      ERROR_SUCCESS) {
    DWORD index = 0;
    char keyName[256];
    DWORD keyNameSize = sizeof(keyName);

    while (RegEnumKeyExA(hKey, index, keyName, &keyNameSize, NULL, NULL, NULL,
                         NULL) == ERROR_SUCCESS) {
      HKEY hSubKey;
      if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) ==
          ERROR_SUCCESS) {
        char displayName[512];
        DWORD displayNameSize = sizeof(displayName);
        DWORD type;

        if (RegQueryValueExA(hSubKey, "DisplayName", NULL, &type,
                             (BYTE*)displayName,
                             &displayNameSize) == ERROR_SUCCESS) {
          char version[256] = "";
          DWORD versionSize = sizeof(version);
          RegQueryValueExA(hSubKey, "DisplayVersion", NULL, &type,
                           (BYTE*)version, &versionSize);

          char publisher[256] = "";
          DWORD publisherSize = sizeof(publisher);
          RegQueryValueExA(hSubKey, "Publisher", NULL, &type, (BYTE*)publisher,
                           &publisherSize);

          outFile << "- " << displayName << " (32-bit)";
          if (strlen(version) > 0) {
            outFile << " (Version: " << version << ")";
          }
          if (strlen(publisher) > 0) {
            outFile << " [Publisher: " << publisher << "]";
          }
          outFile << endl;
        }
        RegCloseKey(hSubKey);
      }

      index++;
      keyNameSize = sizeof(keyName);
    }
    RegCloseKey(hKey);
  }

  outFile << "==========================" << endl;
  outFile.close();

  cout << "Installed programs list written to: "
       << std::filesystem::absolute(filename) << endl;

  Sleep(1000);
  ifstream test_file("installed_programs.txt");
  if (test_file.good()) {
    test_file.close();
    send_email_with_attachment("serverbottestmmt@gmail.com",
                               "Installed Programs List",
                               "Attached is the list of installed programs.",
                               "installed_programs.txt");
  } else {
    cerr << "Failed to create installed programs list file" << endl;
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

  // Scan registry for installed programs
  HKEY hKey;
  const char* subKey =
      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey) ==
      ERROR_SUCCESS) {
    DWORD index = 0;
    char keyName[256];
    DWORD keyNameSize = sizeof(keyName);

    while (RegEnumKeyExA(hKey, index, keyName, &keyNameSize, NULL, NULL, NULL,
                         NULL) == ERROR_SUCCESS) {
      HKEY hSubKey;
      if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) ==
          ERROR_SUCCESS) {
        char displayName[512];
        DWORD displayNameSize = sizeof(displayName);
        DWORD type;

        if (RegQueryValueExA(hSubKey, "DisplayName", NULL, &type,
                             (BYTE*)displayName,
                             &displayNameSize) == ERROR_SUCCESS) {
          char version[256] = "";
          DWORD versionSize = sizeof(version);
          RegQueryValueExA(hSubKey, "DisplayVersion", NULL, &type,
                           (BYTE*)version, &versionSize);

          char publisher[256] = "";
          DWORD publisherSize = sizeof(publisher);
          RegQueryValueExA(hSubKey, "Publisher", NULL, &type, (BYTE*)publisher,
                           &publisherSize);

          outFile << "- " << displayName;
          if (strlen(version) > 0) {
            outFile << " (Version: " << version << ")";
          }
          if (strlen(publisher) > 0) {
            outFile << " [Publisher: " << publisher << "]";
          }
          outFile << "\n";
        }
        RegCloseKey(hSubKey);
      }

      index++;
      keyNameSize = sizeof(keyName);
    }
    RegCloseKey(hKey);
  }

  // Also check 32-bit programs on 64-bit systems
  const char* subKey32 =
      "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey32, 0, KEY_READ, &hKey) ==
      ERROR_SUCCESS) {
    DWORD index = 0;
    char keyName[256];
    DWORD keyNameSize = sizeof(keyName);

    while (RegEnumKeyExA(hKey, index, keyName, &keyNameSize, NULL, NULL, NULL,
                         NULL) == ERROR_SUCCESS) {
      HKEY hSubKey;
      if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) ==
          ERROR_SUCCESS) {
        char displayName[512];
        DWORD displayNameSize = sizeof(displayName);
        DWORD type;

        if (RegQueryValueExA(hSubKey, "DisplayName", NULL, &type,
                             (BYTE*)displayName,
                             &displayNameSize) == ERROR_SUCCESS) {
          char version[256] = "";
          DWORD versionSize = sizeof(version);
          RegQueryValueExA(hSubKey, "DisplayVersion", NULL, &type,
                           (BYTE*)version, &versionSize);

          char publisher[256] = "";
          DWORD publisherSize = sizeof(publisher);
          RegQueryValueExA(hSubKey, "Publisher", NULL, &type, (BYTE*)publisher,
                           &publisherSize);

          outFile << "- " << displayName << " (32-bit)";
          if (strlen(version) > 0) {
            outFile << " (Version: " << version << ")";
          }
          if (strlen(publisher) > 0) {
            outFile << " [Publisher: " << publisher << "]";
          }
          outFile << "\n";
        }
        RegCloseKey(hSubKey);
      }

      index++;
      keyNameSize = sizeof(keyName);
    }
    RegCloseKey(hKey);
  }

  outFile << "==========================" << endl;
  outFile.close();

  cout << "[Server] Installed programs list saved to: "
       << fs::absolute(outFilePath) << endl;
>>>>>>> Stashed changes
}