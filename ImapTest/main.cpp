#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <regex>
#include <windows.h>
#include <winbase.h>
#include <shellapi.h>
#include <map>
#include <sstream>
#include <fstream>

using namespace std;

string app_password = "qljb lntt dobh rtfe";
string gmail_username = "serverbottestmmt@gmail.com";

size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                     string* output) {
  size_t totalSize = size * nmemb;
  output->append((char*)contents, totalSize);
  return totalSize;
}

string base64_decode(const string& encoded_data) {
  ofstream tempIn("temp_base64.txt");
  tempIn << encoded_data;
  tempIn.close();

  system("certutil -decode temp_base64.txt decoded_output.txt > nul");

  ifstream result("decoded_output.txt");
  stringstream ss;
  ss << result.rdbuf();
  result.close();

  remove("temp_base64.txt");
  remove("decoded_output.txt");

  return ss.str();
}

void execute_command(const string& command) {
  cout << "[Command]: " << command << endl;

  if (command.find("start_program") != string::npos) {
    string prog = command.substr(command.find("start_program") + 13);
    ShellExecuteA(NULL, "open", prog.c_str(), NULL, NULL, SW_SHOWDEFAULT);
  } else if (command.find("shutdown") != string::npos) {
    system("shutdown /s /t 0");
  } else {
    cout << "Command not found!" << endl;
  }
}

void check_email_commands() {
  CURL* curl;
  CURLcode res;
  string readBuffer;

  curl = curl_easy_init();
  if (curl) {
    string url = "imaps://imap.gmail.com:993/INBOX";
    curl_easy_setopt(curl, CURLOPT_USERNAME, gmail_username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, app_password.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "C:/curl/cacert.pem");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "SEARCH UNSEEN");

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      cerr << "Error when find email: " << curl_easy_strerror(res) << endl;
    } else {
      vector<string> ids;
      smatch match;
      regex idRegex(R"(\b\d+\b)");
      string temp = readBuffer;
      while (regex_search(temp, match, idRegex)) {
        ids.push_back(match.str(0));
        temp = match.suffix();
      }

      for (const string& id : ids) {
        string fetch_url = url + "/;UID=" + id;
        readBuffer.clear();
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,
                         ("FETCH " + id + " BODY.PEEK[]").c_str());
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) continue;


        smatch b64match;
        smatch match;
        regex base64Re(
            R"(Content-Transfer-Encoding:\s*base64\s+([A-Za-z0-9+/=\r\n]+))");
        regex plainTextRe(R"(\r\n\r\n([\s\S]+))");

        string decoded;
        if (regex_search(readBuffer, match, base64Re)) {
          string encoded = match.str(1);
          decoded = base64_decode(encoded);
        } else if (regex_search(readBuffer, match, plainTextRe)) {
          decoded = match.str(1);
        }
       
        std::regex b64("=?utf-8\\?B\\?(.*?)\\?=");
        if (regex_search(readBuffer, b64match, b64)) {
          string encoded = b64match.str(1);
          string decoded = base64_decode(encoded);
          istringstream iss(decoded);
          string line;
          while (getline(iss, line)) {
            if (line.find("start_program") != string::npos ||
                line.find("shutdown") != string::npos ||
                line.find("COMMAND") != string::npos) {
              execute_command(line);
            }
          }
        }
      }
    }
    curl_easy_cleanup(curl);
  }
}

int main() {
  check_email_commands();
  return 0;
}
