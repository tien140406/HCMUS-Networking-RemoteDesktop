#include "RemoteAdminUI.h"
#include <cctype>
#include <unordered_set>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static std::string GetFileExtension(const std::string& filepath) {
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos) return "";
    std::string ext = filepath.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
    return ext;
}

static bool IsImageExtension(const std::string& ext) {
    static const std::unordered_set<std::string> exts = { "png", "jpg", "jpeg", "bmp", "tga" };
    return exts.count(ext) > 0;
}

static bool IsVideoExtension(const std::string& ext) {
    static const std::unordered_set<std::string> exts = { "mp4", "avi", "mov", "mkv", "flv", "wmv" };
    return exts.count(ext) > 0;
}

ImageData RemoteAdminUI::LoadImageTexture(const std::string& filepath) {
    ImageData img;
    int channels;
    unsigned char* data = stbi_load(filepath.c_str(), &img.width, &img.height, &channels, 0);
    if (data) {
        glGenTextures(1, &img.texture);
        glBindTexture(GL_TEXTURE_2D, img.texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        GLenum format = (channels == 4) ? GL_RGBA :
                        (channels == 1) ? GL_RED : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, img.width, img.height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    return img;
}

void RemoteAdminUI::LoadMedia(const std::string& filepath) {
    std::string ext = GetFileExtension(filepath);

    if (IsImageExtension(ext)) {
        // Clear any playing video
        StopVideo();
        
        ImageData img = LoadImageTexture(filepath); 
        if (img.texture != 0) { 
            SetResultImage(img.texture, img.width, img.height, filepath); 
        }
    }
    else if (IsVideoExtension(ext)) {
        // Clear any displayed image
        ClearResult();
        PlayVideo(filepath);
    }
    else {
        SetResultText("Unsupported file format: " + ext);
    }
}

void RemoteAdminUI::PlayVideo(const std::string& filepath) {
    // Stop any current video
    StopVideo();
    
    // Open video file
    if (!currentVideo.open(filepath)) {
        SetResultText("Failed to open video: " + filepath);
        return;
    }
    
    // Get video properties
    videoFPS = currentVideo.get(cv::CAP_PROP_FPS);
    videoFrameCount = static_cast<int>(currentVideo.get(cv::CAP_PROP_FRAME_COUNT));
    currentVideoFrame = 0;
    
    // Set video as playing
    isVideoPlaying = true;
    isVideoPaused = false;
    lastFrameTime = std::chrono::steady_clock::now();
    
    // Create video texture
    if (videoTexture == 0) {
        glGenTextures(1, &videoTexture);
    }
    
    // Load first frame
    UpdateVideoFrame();
    
    // Set as current result
    cv::Mat frame;
    if (currentVideo.read(frame)) {
        currentVideo.set(cv::CAP_PROP_POS_FRAMES, 0); // Reset to first frame
        currentResultTexture = videoTexture;
        currentResultWidth = frame.cols;
        currentResultHeight = frame.rows;
        currentResultFileName = filepath;
    }
}

void RemoteAdminUI::UpdateVideoFrame() {
    if (!isVideoPlaying || isVideoPaused) return;
    
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - lastFrameTime).count();
    
    // Check if it's time for next frame
    if (elapsed >= (1.0 / videoFPS)) {
        cv::Mat frame;
        if (currentVideo.read(frame)) {
            // Update texture with new frame
            GLuint texture = CreateTextureFromMat(frame);
            if (texture != 0) {
                if (videoTexture != 0) {
                    glDeleteTextures(1, &videoTexture);
                }
                videoTexture = texture;
                currentResultTexture = videoTexture;
            }
            
            currentVideoFrame++;
            lastFrameTime = now;
        } else {
            // End of video - loop or stop
            currentVideo.set(cv::CAP_PROP_POS_FRAMES, 0);
            currentVideoFrame = 0;
        }
    }
}

GLuint RemoteAdminUI::CreateTextureFromMat(const cv::Mat& mat) {
    if (mat.empty()) return 0;
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // OpenCV uses BGR, OpenGL expects RGB
    cv::Mat rgbMat;
    cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rgbMat.cols, rgbMat.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbMat.data);
    
    return texture;
}

void RemoteAdminUI::PlayPauseVideo() {
    if (isVideoPlaying) {
        isVideoPaused = !isVideoPaused;
        if (!isVideoPaused) {
            lastFrameTime = std::chrono::steady_clock::now();
        }
    }
}

void RemoteAdminUI::StopVideo() {
    if (isVideoPlaying) {
        isVideoPlaying = false;
        isVideoPaused = false;
        currentVideo.release();
        
        if (videoTexture != 0) {
            glDeleteTextures(1, &videoTexture);
            videoTexture = 0;
        }
        
        currentVideoFrame = 0;
        videoFrameCount = 0;
        videoFPS = 0.0;
    }
}

void RemoteAdminUI::SeekVideo(int frame) {
    if (isVideoPlaying && frame >= 0 && frame < videoFrameCount) {
        currentVideo.set(cv::CAP_PROP_POS_FRAMES, frame);
        currentVideoFrame = frame;
        
        // Update frame immediately
        cv::Mat mat;
        if (currentVideo.read(mat)) {
            GLuint texture = CreateTextureFromMat(mat);
            if (texture != 0) {
                if (videoTexture != 0) {
                    glDeleteTextures(1, &videoTexture);
                }
                videoTexture = texture;
                currentResultTexture = videoTexture;
            }
        }
        
        // Reset frame position for continuous playback
        currentVideo.set(cv::CAP_PROP_POS_FRAMES, frame);
        lastFrameTime = std::chrono::steady_clock::now();
    }
}