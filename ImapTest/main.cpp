#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <regex>
#include <windows.h>
#include <shellapi.h>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstdlib>  // for system()
#include <algorithm> // for remove()

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
    const string temp_in = "temp_base64_" + to_string(GetCurrentProcessId()) + ".txt";
    const string temp_out = "decoded_output_" + to_string(GetCurrentProcessId()) + ".txt";
    
    ofstream tempIn(temp_in);
    if (!tempIn) {
        cerr << "Failed to create temporary input file" << endl;
        return "";
    }
    tempIn << encoded_data;
    tempIn.close();

    string command = "certutil -decode \"" + temp_in + "\" \"" + temp_out + "\" > nul";
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

void execute_command(const string& command) {
    cout << "[Command]: " << command << endl;

    // Trim whitespace from command
    string trimmed_cmd = command;
    trimmed_cmd.erase(trimmed_cmd.begin(), find_if(trimmed_cmd.begin(), trimmed_cmd.end(), [](int ch) { return !isspace(ch); }));
    trimmed_cmd.erase(find_if(trimmed_cmd.rbegin(), trimmed_cmd.rend(), [](int ch) { return !isspace(ch); }).base(), trimmed_cmd.end());

    if (trimmed_cmd.empty()) {
        cout << "Empty command received" << endl;
        return;
    }

    if (trimmed_cmd.find("start_program") == 0) {
        string prog = trimmed_cmd.substr(13); // after "start_program"
        prog.erase(prog.begin(), find_if(prog.begin(), prog.end(), [](int ch) { return !isspace(ch); })); // trim left
        
        if (!prog.empty()) {
            cout << "Starting program: " << prog << endl;
            HINSTANCE result = ShellExecuteA(NULL, "open", prog.c_str(), NULL, NULL, SW_SHOWDEFAULT);
            if (reinterpret_cast<INT_PTR>(result) <= 32) {
                cerr << "Failed to start program. Error code: " << reinterpret_cast<INT_PTR>(result) << endl;
            }
        }
    }
    else if (trimmed_cmd.find("shutdown") == 0) {
        cout << "Initiating system shutdown..." << endl;
        if (system("shutdown /s /t 0") != 0) {
            cerr << "Shutdown command failed" << endl;
        }
    }
    else {
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
    regex plainTextRe(R"(Content-Type:\s*text/plain.*?\r\n\r\n([\s\S]*?)\r\n--)", regex::icase);
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
    //curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com/INBOX?SEARCH=UNSEEN");
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
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, nullptr);  // clear previous custom command

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        
        if (res != CURLE_OK) {
            cerr << "Failed to fetch email UID " << id << ": " << curl_easy_strerror(res) << endl;
            continue;
        }
        
        cout << "=== FETCHED RAW EMAIL ===\n" << readBuffer << "\n=========================\n";
        string decoded = extract_plain_text_from_email(readBuffer);
        cerr << "=== DECODED ===\n" << decoded << "\n=================\n";

        // Process commands
        istringstream iss(decoded);
        string line;
        while (getline(iss, line)) {
            cout << line << '\n'; 
            if (line.find("start_program") != string::npos ||
                line.find("shutdown") != string::npos ||
                line.find("COMMAND") != string::npos) {
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
    //test_command_execution();

    // Cleanup curl
    curl_global_cleanup();

    system("pause"); 
    return 0;
}