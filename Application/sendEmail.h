#pragma once
#include "lib.h"
#include "config.h"

string getMimeType(const string& extension);
void send_email_with_attachment(const string& toEmail, const string& subject,
                                const string& body, const string& filepath);