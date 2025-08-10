#pragma once
#include "lib.h"
#include "config.h"
#include "checkCommand.h"
#include "executeCommand.h"

string extract_plain_text_from_email(const string& rawEmail);

void check_email_commands();

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);

vector<std::pair<std::string, std::string>> fetch_email_commands();
