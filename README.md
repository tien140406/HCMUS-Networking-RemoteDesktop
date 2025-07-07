# 📬 Email Command Execution Program

## 📝 Mô tả

Chương trình này kết nối tới Gmail thông qua IMAP, kiểm tra email **chưa đọc** chứa lệnh và **thực thi các lệnh điều khiển từ xa** trên hệ điều hành Windows.

Ứng dụng viết bằng **C++**, sử dụng các thư viện **libcurl**, **OpenCV**, và một số API của Windows.

## 📁 Cấu trúc dự án

```
HCMUS-Networking-RemoteDesktop/
├── application/           # Các chức năng điều khiển máy
│   ├── checkCommand.cpp   # Kiểm tra email và phân tích lệnh
│   ├── shutdownProgram.cpp
│   ├── executeCommand.cpp
│   ├── sendEmail.cpp
│   ├── listProgram.cpp
│   ├── screenshot.cpp
│   ├── listDrives.cpp
│   ├── restart.cpp
│   └── ...
├── imap/
│   └── main.cpp           # File chính, có hàm main()
├── build.bat              # Script build tự động bằng MSYS2
├── CMakeLists.txt         # File cấu hình CMake
```

## 🛠 Yêu cầu hệ thống

- ✅ Windows 10 hoặc mới hơn
- ✅ Kết nối Internet
- ✅ [MSYS2](https://www.msys2.org/) (MinGW64) cài đặt đúng
- ✅ Libcurl & OpenCV được cài bằng `pacman`
- ✅ File chứng chỉ `cacert.pem` tại `C:/curl/`
- ✅ Tài khoản Gmail:
  - Đã bật **IMAP**
  - Đã tạo **App Password** (không phải mật khẩu thường)

## 🧰 Hướng dẫn cài đặt MSYS2 và thư viện

### 🔹 Bước 1: Cài MSYS2

1. Tải về tại: https://www.msys2.org/
2. Cài đặt, rồi mở **MSYS2 MinGW 64-bit** terminal
3. Cập nhật:
   ```bash
   pacman -Syu
   ```

### 🔹 Bước 2: Cài các gói cần thiết

```bash
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-libcurl
pacman -S mingw-w64-x86_64-opencv
pacman -S mingw-w64-x86_64-cmake
```

> Lưu ý: tất cả thư viện sẽ được cài vào `C:\msys64\mingw64\`

## ⚙ Cấu hình Gmail

- Tài khoản: `serverbottestmmt@gmail.com`
- App Password: `qljb lntt dobh rtfe`

> 🔒 **Cảnh báo bảo mật**: Không dùng tài khoản Gmail chính. Tạo một tài khoản phụ chuyên để chạy chương trình này.

## 🧩 Các chức năng đã có

| Lệnh email               | Chức năng                                   |
| ------------------------ | ------------------------------------------- |
| `start_program[path]`    | Khởi chạy chương trình tại đường dẫn đã cho |
| `shutdown`               | Tắt máy ngay lập tức                        |
| `shutdown_program[name]` | Tắt tiến trình theo tên                     |
| `send_file[path]`        | Gửi lại file được yêu cầu qua email         |
| `restart`                | Khởi động lại máy tính                      |
| `screenshot`             | Chụp màn hình và gửi ảnh đính kèm           |
| `list_exe`               | Liệt kê các tiến trình đang chạy (.exe)     |
| `list_drives`            | Liệt kê danh sách ổ đĩa đang kết nối        |

## 🏗 Cách build chương trình

1. Mở terminal **Windows CMD** hoặc **Git Bash**
2. Chạy file `build.bat`:
   ```bash
   ./build.bat
   ```
   File này sẽ:
   - Gọi CMake
   - Tự động biên dịch
   - Copy toàn bộ `.dll` từ MSYS2 vào thư mục `build/`
3. Sau khi thành công, bạn sẽ có:
   ```
   build/
   ├── IMAP.exe
   ├── *.dll
   ```

## 🚀 Cách chạy chương trình

Từ terminal Git Bash hoặc CMD:

```bash
cd build
./IMAP.exe
```

Chương trình sẽ:

1. Kết nối đến Gmail qua IMAP (`imap.gmail.com:993`)
2. Kiểm tra email chưa đọc
3. Phân tích nội dung
4. Thực thi lệnh nếu hợp lệ

## 🛡 Cảnh báo bảo mật

⚠ **Rất quan trọng**:

- Mọi lệnh đều được thực thi **không xác minh người gửi**
- Bất kỳ ai có quyền gửi email đến Gmail bạn cấu hình đều có thể điều khiển máy bạn
- Chỉ dùng trong mục đích học tập / demo có kiểm soát
- Không bao giờ dùng trên máy thật chứa thông tin quan trọng
