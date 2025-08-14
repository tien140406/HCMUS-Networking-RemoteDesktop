#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <mfobjects.h>
#include <mferror.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <wrl/client.h>
#include <glad/glad.h>

using Microsoft::WRL::ComPtr;

class VideoPlayer {
public:
    VideoPlayer() : textureID(0), playing(false) {}
    ~VideoPlayer() { Cleanup(); }

    bool Init() {
        HRESULT hr = MFStartup(MF_VERSION);
        return SUCCEEDED(hr);
    }

    bool LoadMedia(const std::string& filepath) {
        Cleanup();

        HRESULT hr = MFCreateMediaPlayer(NULL, FALSE, 0, &player);
        if (FAILED(hr)) return false;

        std::wstring wpath(filepath.begin(), filepath.end());
        hr = player->SetURL(wpath.c_str());
        if (FAILED(hr)) return false;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        playing = false;
        return true;
    }

    void PlayVideo() {
        if (player) {
            player->Play();
            playing = true;
        }
    }

    void PauseVideo() {
        if (player) {
            player->Pause();
            playing = false;
        }
    }

    void SeekVideo(float seconds) {
        if (!player) return;
        PROPVARIANT pos;
        PropVariantInit(&pos);
        pos.vt = VT_I8;
        pos.hVal.QuadPart = static_cast<LONGLONG>(seconds * 10000000.0); // 100ns units
        player->SetPosition(&pos);
        PropVariantClear(&pos);
    }

    void UpdateVideoTexture() {
        if (!player) return;

        // This is a placeholder — in reality you would grab the frame's surface from the MF player
        // and upload it to the OpenGL texture here.
        // For simple demo: keep using existing textureID.
    }

    GLuint GetTextureID() const { return textureID; }

    void Cleanup() {
        if (player) {
            player->Shutdown();
            player.Reset();
        }
        if (textureID) {
            glDeleteTextures(1, &textureID);
            textureID = 0;
        }
        playing = false;
    }

private:
    ComPtr<IMFPMediaPlayer> player;
    GLuint textureID;
    bool playing;
};

static VideoPlayer g_video;

bool InitVideoSystem() { return g_video.Init(); }
bool LoadMedia(const std::string& path) { return g_video.LoadMedia(path); }
void PlayVideo() { g_video.PlayVideo(); }
void PauseVideo() { g_video.PauseVideo(); }
void SeekVideo(float seconds) { g_video.SeekVideo(seconds); }
void UpdateVideoTexture() { g_video.UpdateVideoTexture(); }
GLuint GetVideoTexture() { return g_video.GetTextureID(); }