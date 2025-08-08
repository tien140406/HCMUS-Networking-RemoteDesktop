#include "sendEmail.h"
#include <map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>

// Enhanced MIME type detection
string getMimeType(const string& extension) {
    static const std::map<string, string> mimeTypes = {
        // Images
        {".jpg",  "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png",  "image/png"},
        {".gif",  "image/gif"},
        {".bmp",  "image/bmp"},
        {".webp", "image/webp"},
        
        // Videos
        {".avi",  "video/x-msvideo"},
        {".mp4",  "video/mp4"},
        {".mov",  "video/quicktime"},
        
        // Text
        {".txt",  "text/plain"},
        {".html", "text/html"},
        {".css",  "text/css"},
        {".js",   "application/javascript"},
        
        // Documents
        {".pdf",  "application/pdf"},
        {".doc",  "application/msword"},
        {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {".zip",  "application/zip"},
        {".rar",  "application/x-rar-compressed"}
    };
    
    string lower_ext = extension;
    std::transform(lower_ext.begin(), lower_ext.end(), lower_ext.begin(), ::tolower);
    
    auto it = mimeTypes.find(lower_ext);
    return it != mimeTypes.end() ? it->second : "application/octet-stream";
}

// Validate file before attempting to send
bool validateFile(const string& filepath) {
    try {
        if (!std::filesystem::exists(filepath)) {
            cerr << "File does not exist: " << filepath << endl;
            return false;
        }
        
        if (!std::filesystem::is_regular_file(filepath)) {
            cerr << "Path is not a regular file: " << filepath << endl;
            return false;
        }
        
        auto file_size = std::filesystem::file_size(filepath);
        if (file_size == 0) {
            cerr << "File is empty: " << filepath << endl;
            return false;
        }
        
        // Check if file is too large (25MB limit for Gmail)
        const size_t MAX_FILE_SIZE = 25 * 1024 * 1024; // 25MB
        if (file_size > MAX_FILE_SIZE) {
            cerr << "File too large (" << file_size << " bytes, max: " << MAX_FILE_SIZE << "): " << filepath << endl;
            return false;
        }
        
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        cerr << "Filesystem error validating file: " << e.what() << endl;
        return false;
    }
}

// Enhanced email validation
bool isValidEmail(const string& email) {
    if (email.empty()) return false;
    
    // Simple regex-like validation
    size_t at_pos = email.find('@');
    if (at_pos == string::npos || at_pos == 0 || at_pos == email.length() - 1) {
        return false;
    }
    
    size_t dot_pos = email.find('.', at_pos);
    if (dot_pos == string::npos || dot_pos == email.length() - 1) {
        return false;
    }
    
    return true;
}

// Sanitize strings for email headers
string sanitizeHeaderValue(const string& value) {
    string sanitized = value;
    
    // Remove or replace problematic characters
    std::replace(sanitized.begin(), sanitized.end(), '\r', ' ');
    std::replace(sanitized.begin(), sanitized.end(), '\n', ' ');
    std::replace(sanitized.begin(), sanitized.end(), '\0', ' ');
    
    // Trim whitespace
    sanitized.erase(0, sanitized.find_first_not_of(" \t"));
    sanitized.erase(sanitized.find_last_not_of(" \t") + 1);
    
    return sanitized;
}

// Callback for curl progress/debugging
static int progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, 
                           curl_off_t ultotal, curl_off_t ulnow) {
    if (ultotal > 0) {
        double percent = (double)ulnow / (double)ultotal * 100.0;
        if ((int)percent % 10 == 0) { // Print every 10%
            cout << "Upload progress: " << std::fixed << std::setprecision(1) << percent << "%" << endl;
        }
    }
    return 0;
}

void send_email_with_attachment(const std::string& toEmail,
                                const std::string& subject,
                                const std::string& body,
                                const std::string& filepath) {
    
    // Input validation
    if (!isValidEmail(toEmail)) {
        cerr << "Invalid email address: " << toEmail << endl;
        return;
    }
    
    if (subject.empty()) {
        cerr << "Subject cannot be empty" << endl;
        return;
    }
    
    if (!validateFile(filepath)) {
        return; // Error already logged in validateFile
    }
    
    // Initialize curl
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to initialize curl" << endl;
        return;
    }

    struct curl_slist* recipients = nullptr;
    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part = nullptr;
    CURLcode res = CURLE_OK;

    try {
        // Sanitize input strings
        string safe_to = sanitizeHeaderValue(toEmail);
        string safe_subject = sanitizeHeaderValue(subject);
        string safe_body = body; // Body can contain newlines

        // Set authentication and connection options
        curl_easy_setopt(curl, CURLOPT_USERNAME, gmail_username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, app_password.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_bundle_path.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, gmail_username.c_str());
        
        // Set timeout options
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // 5 minutes total timeout
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L); // 30 seconds connection timeout
        
        // Set progress callback for large files
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);

        recipients = curl_slist_append(recipients, safe_to.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        if (!mime) {
            throw std::runtime_error("Failed to initialize MIME structure");
        }

        // Create text body part
        part = curl_mime_addpart(mime);
        if (!part) {
            throw std::runtime_error("Failed to create MIME text part");
        }
        
        curl_mime_data(part, safe_body.c_str(), CURL_ZERO_TERMINATED);
        curl_mime_type(part, "text/plain; charset=UTF-8");
        curl_mime_encoder(part, "quoted-printable");

        // Handle file attachment
        std::filesystem::path filePathObj(filepath);
        string filename = filePathObj.filename().string();
        string extension = filePathObj.extension().string();
        string mimeType = getMimeType(extension);

        cout << "Attaching file: " << filename << " (" << mimeType << ")" << endl;

        // Create attachment part
        part = curl_mime_addpart(mime);
        if (!part) {
            throw std::runtime_error("Failed to create MIME attachment part");
        }
        
        // Use filedata instead of data for better memory efficiency with large files
        CURLcode file_result = curl_mime_filedata(part, filepath.c_str());
        if (file_result != CURLE_OK) {
            throw std::runtime_error("Failed to attach file: " + string(curl_easy_strerror(file_result)));
        }
        
        curl_mime_filename(part, filename.c_str());
        curl_mime_type(part, mimeType.c_str());
        curl_mime_encoder(part, "base64");

        // Set the complete message
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        
        // Add custom headers
        struct curl_slist* headers = nullptr;
        string from_header = "From: " + string(gmail_username);
        string to_header = "To: " + safe_to;
        string subject_header = "Subject: " + safe_subject;
        
        headers = curl_slist_append(headers, from_header.c_str());
        headers = curl_slist_append(headers, to_header.c_str());
        headers = curl_slist_append(headers, subject_header.c_str());
        headers = curl_slist_append(headers, "MIME-Version: 1.0");
        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Enable verbose output only in debug mode
        #ifdef DEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        #endif

        cout << "Sending email to: " << safe_to << endl;
        cout << "Subject: " << safe_subject << endl;
        
        // Perform the email send
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            
            // Provide more specific error messages
            switch (res) {
                case CURLE_LOGIN_DENIED:
                    cerr << "Authentication failed. Check username/password." << endl;
                    break;
                case CURLE_COULDNT_CONNECT:
                    cerr << "Could not connect to SMTP server." << endl;
                    break;
                case CURLE_SSL_CONNECT_ERROR:
                    cerr << "SSL connection error. Check certificate bundle." << endl;
                    break;
                case CURLE_OPERATION_TIMEDOUT:
                    cerr << "Operation timed out." << endl;
                    break;
                default:
                    break;
            }
        } else {
            cout << "Email sent successfully!" << endl;
            
            // Log email details
            try {
                auto file_size = std::filesystem::file_size(filepath);
                cout << "Attachment size: " << (file_size / 1024) << " KB" << endl;
            } catch (...) {
                // Ignore errors getting file size for logging
            }
        }
        
        // Cleanup
        curl_slist_free_all(headers);
        
    } catch (const std::exception& e) {
        cerr << "Exception in send_email_with_attachment: " << e.what() << endl;
        res = CURLE_FAILED_INIT; // Set error state
    }

    // Always cleanup curl resources
    if (mime) {
        curl_mime_free(mime);
    }
    if (recipients) {
        curl_slist_free_all(recipients);
    }
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        cerr << "Email sending failed!" << endl;
    }
}