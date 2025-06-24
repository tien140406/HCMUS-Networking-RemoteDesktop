# Email Command Execution Program - README

## 📝 Mô tả
Chương trình này kiểm tra hộp thư Gmail để tìm các email chưa đọc chứa lệnh và thực thi chúng trên hệ thống Windows cục bộ.

## 🛠 Yêu cầu hệ thống
1. Hệ điều hành Windows
2. Đã cài đặt libcurl và cấu hình đúng cách
3. File `cacert.pem` (curl-ca-bundle) đặt trong thư mục `C:/curl/`
4. Kết nối Internet
5. Tài khoản Gmail đã bật IMAP
6. Mật khẩu ứng dụng đã được tạo cho tài khoản Gmail

## ⚙ Cấu hình
Chương trình sử dụng thông tin đăng nhập được mã hóa cứng:
- **Username**: serverbottestmmt@gmail.com
- **App Password**: qljb lntt dobh rtfe

⚠ **Cảnh báo bảo mật**:  
Những thông tin đăng nhập này có thể nhìn thấy trong mã nguồn. Để bảo mật, bạn nên:
1. Tạo một tài khoản Gmail mới chuyên dụng
2. Tạo mật khẩu ứng dụng mới
3. Cập nhật thông tin đăng nhập trong `main.cpp`
4. Biên dịch lại chương trình

## ⌨ Lệnh được hỗ trợ
- `start_program[path_to_program]` - Khởi chạy chương trình được chỉ định
- `shutdown` - Tắt máy tính ngay lập tức

📧 **Lưu ý**:  
Các lệnh phải được đặt trong nội dung email gửi đến tài khoản đã cấu hình.

## 📥 Cài đặt
1. Biên dịch chương trình với trình biên dịch C++ hỗ trợ libcurl
2. Đảm bảo tất cả các phụ thuộc đã sẵn sàng
3. Đặt file thực thi ở vị trí mong muốn

## 🚀 Cách sử dụng
Chạy chương trình và nó sẽ:
1. Kết nối với Gmail qua IMAP
2. Kiểm tra email chưa đọc
3. Phân tích nội dung email để tìm lệnh
4. Thực thi các lệnh hợp lệ tìm thấy

## ⚠ Cảnh báo bảo mật quan trọng
- Chương trình này thực thi lệnh từ email mà không xác minh
- Bất kỳ ai có thể gửi email đến tài khoản này đều có thể chạy lệnh trên hệ thống của bạn
- Chỉ sử dụng trong môi trường được kiểm soát hoặc với các biện pháp bảo mật bổ sung