//
// Created by Stěpán Toman on 18.05.2026.
//

#include <Audio.hpp>

Audio::Audio(Application& application, const std::string& filePath) : filePath(filePath), app(application) {
    audio = MIX_LoadAudio(app.getMixer(), filePath.c_str(), true);
    if (!audio) {
        app.triggerError("Couldn't load audio file");
        return;
    }

    track = MIX_CreateTrack(app.getMixer());
    if (!track) {
        app.triggerError("Couldn't create track");
        return;
    }

    MIX_SetTrackAudio(track, audio);
}

Audio::~Audio() {
    MIX_DestroyAudio(audio);
    MIX_DestroyTrack(track);
}

void Audio::play() {
    MIX_PlayTrack(track, 0);
}

void Audio::stop(int fadeOutFrames) {
    MIX_StopTrack(track, fadeOutFrames);
}

void Audio::loopAmount(int arg0) {
    MIX_SetTrackLoops(track, arg0);
}

void Audio::pause() {
    MIX_PauseTrack(track);
}

void Audio::resume() {
    MIX_ResumeTrack(track);
}

void Audio::setGain(float gain) {
    MIX_SetTrackGain(track, gain);
}

bool Audio::isStarted() {
    return MIX_TrackPlaying(track);
}

bool Audio::isStopped() {
    return !isStarted();
}

bool Audio::isPaused() {
    return MIX_TrackPaused(track);
}

int Audio::getLoopAmount() {
    return MIX_GetTrackLoops(track);
}

float Audio::getGain() {
    return MIX_GetTrackGain(track);
}

const std::string& Audio::getAudioFilePath() {
    return filePath;
}

MIX_Audio* Audio::getAudio() {
    return audio;
}

MIX_Track* Audio::getTrack() {
    return track;
}
