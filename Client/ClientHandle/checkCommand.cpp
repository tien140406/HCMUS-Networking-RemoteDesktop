#include "checkCommand.h"

// Add allowed senders list - configure these addresses
const vector<string> ALLOWED_SENDERS = {
    "serverbottestmmt@gmail.com",
    "minhtientr06@gmail.com",
    "nguyencaothong189@gmail.com",
    "ngohuynhtham@gmail.com"
};

const string REQUIRED_SUBJECT = "REMOTE CONTROL";

string extract_plain_text_from_email(const string& rawEmail) {
    smatch match;
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

bool is_sender_allowed(const string& senderEmail) {
    for (const string& allowed : ALLOWED_SENDERS) {
        if (senderEmail.find(allowed) != string::npos) {
            return true;
        }
    }
    return false;
}

bool extract_email_info(const string& rawEmail, string& subject, string& sender) {
    smatch match;
    // Extract subject
    regex subjectRe(R"(Subject:\s*(.+))", regex::icase);
    if (regex_search(rawEmail, match, subjectRe)) {
        subject = match[1].str();
        // Remove \r\n and trim
        subject.erase(remove(subject.begin(), subject.end(), '\r'), subject.end());
        subject.erase(remove(subject.begin(), subject.end(), '\n'), subject.end());
        subject.erase(0, subject.find_first_not_of(" \t"));
        subject.erase(subject.find_last_not_of(" \t") + 1);
    }
    
    // Extract sender
    regex senderRe(R"(From:\s*.*<([^>]+)>)", regex::icase);
    if (regex_search(rawEmail, match, senderRe)) {
        sender = match[1].str();
    } else {
        // Fallback: try to extract email without brackets
        regex senderRe2(R"(From:\s*([^\r\n\s]+@[^\r\n\s]+))", regex::icase);
        if (regex_search(rawEmail, match, senderRe2)) {
            sender = match[1].str();
        }
    }
    
    return !subject.empty() && !sender.empty();
}

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t totalSize = size * nmemb;
    string* buffer = static_cast<string*>(userdata);
    buffer->append(ptr, totalSize);
    return totalSize;
}

vector<std::pair<std::string, std::string>> fetch_email_commands() {
    std::vector<std::pair<std::string, std::string>> result;
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return result;
    }

    curl_easy_setopt(curl, CURLOPT_USERNAME, gmail_username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, app_password.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_CAINFO, ca_bundle_path.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com/INBOX");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "UID SEARCH UNSEEN");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Error searching emails: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return result;
    }

    // Get UID list
    std::vector<std::string> ids;
    std::regex idRegex(R"(\b\d+\b)");
    for (std::sregex_iterator it(readBuffer.begin(), readBuffer.end(), idRegex), end;
         it != end; ++it) {
        ids.push_back(it->str());
    }

    // Process each email
    for (const auto& id : ids) {
        readBuffer.clear();
        std::string fetchUrl = "imaps://imap.gmail.com/INBOX/;UID=" + id;
        curl_easy_setopt(curl, CURLOPT_URL, fetchUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, nullptr);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Failed to fetch email UID " << id << ": " << curl_easy_strerror(res) << std::endl;
            continue;
        }

        // Extract email info and validate
        std::string subject, sender;
        if (!extract_email_info(readBuffer, subject, sender)) {
            continue;
        }

        // Apply same filtering as check_email_commands
        if (subject != REQUIRED_SUBJECT || !is_sender_allowed(sender)) {
            continue;
        }

        std::string decoded = extract_plain_text_from_email(readBuffer);
        std::istringstream iss(decoded);
        std::string line;
        while (std::getline(iss, line)) {
            line.erase(0, line.find_first_not_of(" \r\n\t"));
            line.erase(line.find_last_not_of(" \r\n\t") + 1);
            if (!line.empty()) {
                result.emplace_back(line, sender);
            }
        }
    }

    curl_easy_cleanup(curl);
    return result;
}