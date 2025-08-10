#include "sendScreenshot.h"

void send_screenshot() { cout << "zz"; }

struct MonitorInfo {
  RECT rect;
  HMONITOR hMonitor;
};

std::vector<MonitorInfo> monitors;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT lprcMonitor,
                              LPARAM) {
  MonitorInfo info;
  info.hMonitor = hMonitor;
  info.rect = *lprcMonitor;
  monitors.push_back(info);
  return TRUE;
}

void SetDPIAwareness() {
  HMODULE hUser32 = LoadLibraryA("user32.dll");
  if (hUser32) {
    auto setProcessDpiAwarenessContext =
        (BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT))GetProcAddress(
            hUser32, "SetProcessDpiAwarenessContext");
    if (setProcessDpiAwarenessContext) {
      setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    }
    FreeLibrary(hUser32);
  }
}

void GetPhysicalScreenDimensions(int& x, int& y, int& width, int& height) {
  monitors.clear();
  EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
  if (monitors.empty()) {
    x = 0;
    y = 0;
    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);
    return;
  }
  RECT boundingRect = monitors[0].rect;
  for (const auto& monitor : monitors) {
    UnionRect(&boundingRect, &boundingRect, &monitor.rect);
  }
  x = boundingRect.left;
  y = boundingRect.top;
  width = boundingRect.right - boundingRect.left;
  height = boundingRect.bottom - boundingRect.top;
}

void take_screenshot(const std::string& outFile) {
  namespace fs = std::filesystem;
  fs::create_directories(fs::path(outFile).parent_path());

  SetDPIAwareness();

  int screen_x, screen_y, screen_width, screen_height;
  GetPhysicalScreenDimensions(screen_x, screen_y, screen_width, screen_height);

  HDC hScreenDC = GetDC(NULL);
  HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
  HBITMAP hBitmap =
      CreateCompatibleBitmap(hScreenDC, screen_width, screen_height);
  HGDIOBJ oldBitmap = SelectObject(hMemoryDC, hBitmap);

  BitBlt(hMemoryDC, 0, 0, screen_width, screen_height, hScreenDC, screen_x,
         screen_y, SRCCOPY | CAPTUREBLT);

  BITMAP bmp;
  GetObject(hBitmap, sizeof(BITMAP), &bmp);

  BITMAPINFOHEADER bi{};
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = bmp.bmWidth;
  bi.biHeight = -bmp.bmHeight;  // top-down
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;

  cv::Mat screenshot(abs(bi.biHeight), bi.biWidth, CV_8UC4);
  GetDIBits(hScreenDC, hBitmap, 0, abs(bi.biHeight), screenshot.data,
            (BITMAPINFO*)&bi, DIB_RGB_COLORS);

  cv::Mat bgr_screenshot;
  cv::cvtColor(screenshot, bgr_screenshot, cv::COLOR_BGRA2BGR);

  if (!cv::imwrite(outFile, bgr_screenshot)) {
    std::cerr << "[Server] Failed to save screenshot to: " << outFile << "\n";
  } else {
    std::cout << "[Server] Screenshot saved to: " << outFile << "\n";
  }

  SelectObject(hMemoryDC, oldBitmap);
  DeleteObject(hBitmap);
  DeleteDC(hMemoryDC);
  ReleaseDC(NULL, hScreenDC);
}