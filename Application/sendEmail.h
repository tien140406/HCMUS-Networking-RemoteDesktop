#pragma once
#include "lib.h"
#include "config.h"

void send_email_with_attachment(const string& toEmail, const string& subject,
                                const string& body, const string& filepath);