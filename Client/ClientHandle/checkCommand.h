#pragma once
#include "commandUtils.h"
#include "lib.h"

string extract_plain_text_from_email(const string& rawEmail);

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);

vector<std::pair<std::string, std::string>> fetch_email_commands();
