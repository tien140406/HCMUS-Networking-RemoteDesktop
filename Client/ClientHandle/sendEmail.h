#pragma once
#include "lib.h"
#include "commandUtils.h"

string getMimeType(const string& extension);
void send_email_with_attachment(const string& toEmail, const string& subject,
                                const string& body, const string& filepath);
void send_file_via_email(const std::string& toEmail, const std::string& subject,
                         const std::string& body, const std::string& filepath);