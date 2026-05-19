//
// Created by Stěpán Toman on 18.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_AUDIO_HPP
#define TRANSLUCENCEWORKSPACE_AUDIO_HPP

#include <T_Core.hpp>
#include "Application.hpp"

class Audio {
public:
    Audio(Application& application, const std::string& filePath);
    ~Audio();
    void play();
    void stop(int fadeOutFrames = 0);
    void loopAmount(int arg0 = 0);
    void pause();
    void resume();
    void setGain(float gain);

    bool isStarted();
    bool isStopped();
    bool isPaused();
    int getLoopAmount();
    float getGain();

    const std::string& getAudioFilePath();
    MIX_Audio* getAudio();
    MIX_Track* getTrack();

private:
    Application& app;
    const std::string& filePath;
    MIX_Audio* audio;
    MIX_Track* track;
};
#endif //TRANSLUCENCEWORKSPACE_AUDIO_HPP
