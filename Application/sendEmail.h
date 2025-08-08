#pragma once
#include "lib.h"
#include "config.h"

string getMimeType(const string& extension);
bool validateFile(const string& filepath);
bool isValidEmail(const string& email);
string sanitizeHeaderValue(const string& value);
void send_email_with_attachment(const string& toEmail, const string& subject,
                                const string& body, const string& filepath);
namespace EmailConfig {
    constexpr const char* SMTP_SERVER = "smtps://smtp.gmail.com:465";
    constexpr long CONNECTION_TIMEOUT = 30L;  // seconds
    constexpr long TOTAL_TIMEOUT = 300L;      // seconds (5 minutes)
    constexpr size_t MAX_ATTACHMENT_SIZE = 25 * 1024 * 1024; // 25MB
}
static int progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, 
                           curl_off_t ultotal, curl_off_t ulnow);