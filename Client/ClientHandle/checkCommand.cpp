#include "checkCommand.h"
#include <ws2tcpip.h>

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
    return true;
    return false;
}

static inline void trim_inplace(std::string& s) {
    auto not_space = [](unsigned char c){ return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
}

static std::string header_block(const std::string& raw) {
    // headers end at first blank line
    std::size_t pos = raw.find("\r\n\r\n");
    if (pos == std::string::npos) pos = raw.find("\n\n");
    return (pos == std::string::npos) ? raw : raw.substr(0, pos);
}

static std::string unfold_headers(std::string h) {
    // Convert folded headers: CRLF + SP/HT (or LF + SP/HT) -> single space
    static const std::regex fold_re(R"((\r?\n)[ \t]+)");
    return std::regex_replace(h, fold_re, " ");
}

static bool iequals_prefix(const std::string& line, const std::string& keyColon) {
    if (line.size() < keyColon.size()) return false;
    for (size_t i = 0; i < keyColon.size(); ++i) {
        if (std::tolower((unsigned char)line[i]) != std::tolower((unsigned char)keyColon[i]))
            return false;
    }
    return true;
}

static bool get_header_value(const std::string& headers_unfolded,
                             const char* key, std::string& out) {
    std::istringstream iss(headers_unfolded);
    std::string line;
    std::string keyColon = std::string(key) + ":";

    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (iequals_prefix(line, keyColon)) {
            out = line.substr(keyColon.size());
            trim_inplace(out);
            return true;
        }
    }
    return false;
}

bool extract_email_info(const std::string& rawEmail, std::string& subject, std::string& sender) {
    subject.clear();
    sender.clear();

    // 1) isolate headers and unfold
    std::string headers = header_block(rawEmail);
    headers = unfold_headers(headers);

    // 2) subject
    get_header_value(headers, "Subject", subject); // ok if missing; we'll return false at end

    // 3) from -> extract address
    std::string fromLine;
    if (get_header_value(headers, "From", fromLine)) {
        std::smatch m;
        // Prefer address inside <...>
        if (std::regex_search(fromLine, m, std::regex(R"(<([^>]+)>)"))) {
            sender = m[1].str();
        } else {
            // Fallback: any bare email in the value
            if (std::regex_search(fromLine, m, std::regex(R"(([A-Za-z0-9._%+\-]+@[A-Za-z0-9.\-]+\.[A-Za-z]{2,}))"))) {
                sender = m[1].str();
            }
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
            cout << "[Client] found not match subject or sender.\n";
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