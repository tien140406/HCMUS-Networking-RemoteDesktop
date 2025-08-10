#pragma once
#include "lib.h"

bool wait_for_recording_complete(int timeoutSeconds = 10);

void recording_loop();
void start_recording(const std::string& outFilePath);
void stop_recording();