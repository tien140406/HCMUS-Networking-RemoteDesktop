#include "sendEmail.h"
#include <map>
#include <filesystem>

string getMimeType(const string& extension) {
  static const std::map<string, string> mimeTypes = {
    {".jpg",  "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png",  "image/png"},
    {".txt",  "text/plain"}, 
  };
  auto it = mimeTypes.find(extension);
  return it != mimeTypes.end() ? it->second : "application/octet-stream";
}

void send_email_with_attachment(const std::string& toEmail,
                                const std::string& subject,
                                const std::string& body,
                                const std::string& filepath) {
  CURL* curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Failed to initialize curl" << std::endl;
    return;
  }

  struct curl_slist* recipients = nullptr;
  curl_mime* mime = curl_mime_init(curl);
  curl_mimepart* part = nullptr;

  curl_easy_setopt(curl, CURLOPT_USERNAME, gmail_username.c_str());
  curl_easy_setopt(curl, CURLOPT_PASSWORD, app_password.c_str());
  curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
  curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
  curl_easy_setopt(curl, CURLOPT_CAINFO, ca_bundle_path.c_str());
  curl_easy_setopt(curl, CURLOPT_MAIL_FROM, gmail_username.c_str());

  recipients = curl_slist_append(recipients, toEmail.c_str());
  curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

  // Create email headers part
  part = curl_mime_addpart(mime);
  std::string headers = "To: " + toEmail +
                        "\r\n"
                        "From: " +
                        gmail_username +
                        "\r\n"
                        "Subject: " +
                        subject +
                        "\r\n"
                        "MIME-Version: 1.0\r\n";
  curl_mime_data(part, headers.c_str(), CURL_ZERO_TERMINATED);
  curl_mime_type(part, "text/plain");

  // Add body part
  part = curl_mime_addpart(mime);
  curl_mime_data(part, body.c_str(), CURL_ZERO_TERMINATED);
  curl_mime_type(part, "text/plain");

  std::filesystem::path filePathObj(filepath);
  string filename = filePathObj.filename().string();
  string extension = filePathObj.extension().string();
  string mimeType = getMimeType(extension);

  // Add attachment part
  part = curl_mime_addpart(mime);
  curl_mime_filedata(part, filepath.c_str());
  curl_mime_filename(part, filename.c_str());
  curl_mime_type(part, mimeType.c_str());
  curl_mime_encoder(part, "base64");

  curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
              << std::endl;
  } else {
    std::cout << "Email sent successfully!" << std::endl;
  }

  curl_mime_free(mime);
  curl_slist_free_all(recipients);
  curl_easy_cleanup(curl);
}