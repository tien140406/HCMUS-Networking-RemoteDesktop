#pragma once
#include "lib.h"

void run_recording_and_save(const std::string& outFilePath,
                            int duration_seconds);

namespace RecordingConfig {
constexpr int DEFAULT_FPS = 20;
constexpr int DEFAULT_WIDTH = 1280;
constexpr int DEFAULT_HEIGHT = 720;
constexpr int MAX_CAPTURE_ATTEMPTS = 5;
constexpr int MAX_CONSECUTIVE_EMPTY_FRAMES = 10;
}