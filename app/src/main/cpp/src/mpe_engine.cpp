#include "mpe_engine.h"

#include <algorithm>
#include <cmath>

MPEEngine::MPEEngine(MidiOutput& midiOutput) : midiOutput_(midiOutput) {
    channelUsed_.fill(false);
    channelUsed_[kMasterChannel] = true;
}

int MPEEngine::allocateChannel() {
    for (uint8_t ch = kMemberChannelFirst; ch <= kMemberChannelLast; ++ch) {
        if (!channelUsed_[ch]) {
            channelUsed_[ch] = true;
            return ch;
        }
    }
    return -1;
}

void MPEEngine::releaseChannel(uint8_t channel) {
    if (channel >= kMemberChannelFirst && channel <= kMemberChannelLast) {
        channelUsed_[channel] = false;
    }
}

void MPEEngine::noteOn(int32_t touchId, float xNorm, float yNorm, float pressureNorm) {
    VoiceState* slot = findFreeVoiceSlot();
    const int channel = allocateChannel();
    if (slot == nullptr || channel < 0) {
        return;
    }

    slot->active = true;
    slot->pointerId = touchId;
    slot->channel = static_cast<uint8_t>(channel);
    slot->startX = xNorm;
    slot->x = xNorm;
    slot->y = yNorm;
    slot->pressure = pressureNorm;
    slot->note = mapXToNote(xNorm);

    midiOutput_.sendNoteOn(slot->channel, slot->note, 100);
    midiOutput_.sendPitchBend(slot->channel, 8192);
    midiOutput_.sendCC74(slot->channel, normToMidi7(1.0f - yNorm));
    midiOutput_.sendChannelPressure(slot->channel, normToMidi7(pressureNorm));
}

void MPEEngine::noteMove(int32_t touchId, float xNorm, float yNorm, float pressureNorm) {
    VoiceState* voice = findVoiceByPointer(touchId);
    if (voice == nullptr) {
        return;
    }
    voice->x = xNorm;
    voice->y = yNorm;
    voice->pressure = pressureNorm;

    const float deltaX = xNorm - voice->startX;
    midiOutput_.sendPitchBend(voice->channel, mapDeltaXToBend(deltaX));
    midiOutput_.sendCC74(voice->channel, normToMidi7(1.0f - yNorm));
    midiOutput_.sendChannelPressure(voice->channel, normToMidi7(pressureNorm));
}

void MPEEngine::noteOff(int32_t touchId) {
    VoiceState* voice = findVoiceByPointer(touchId);
    if (voice == nullptr) {
        return;
    }
    midiOutput_.sendNoteOff(voice->channel, voice->note, 64);
    releaseChannel(voice->channel);
    *voice = VoiceState{};
}

const std::array<MPEEngine::VoiceState, MPEEngine::kMaxVoices>& MPEEngine::voices() const {
    return voices_;
}

MPEEngine::VoiceState* MPEEngine::findVoiceByPointer(int32_t pointerId) {
    for (auto& voice : voices_) {
        if (voice.active && voice.pointerId == pointerId) {
            return &voice;
        }
    }
    return nullptr;
}

MPEEngine::VoiceState* MPEEngine::findFreeVoiceSlot() {
    for (auto& voice : voices_) {
        if (!voice.active) {
            return &voice;
        }
    }
    return nullptr;
}

uint8_t MPEEngine::mapXToNote(float xNorm) const {
    constexpr uint8_t minNote = 48;  // C3
    constexpr uint8_t maxNote = 84;  // C6
    const float clamped = std::clamp(xNorm, 0.0f, 1.0f);
    return static_cast<uint8_t>(minNote + std::round(clamped * (maxNote - minNote)));
}

uint16_t MPEEngine::mapDeltaXToBend(float deltaX) const {
    const float bend = 8192.0f + (std::clamp(deltaX, -1.0f, 1.0f) * 8192.0f);
    return static_cast<uint16_t>(std::clamp(bend, 0.0f, 16383.0f));
}

uint8_t MPEEngine::normToMidi7(float v) const {
    return static_cast<uint8_t>(std::round(std::clamp(v, 0.0f, 1.0f) * 127.0f));
}
