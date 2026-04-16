#pragma once

#include "midi_output.h"

#include <array>
#include <cstdint>

class MPEEngine {
public:
    static constexpr uint8_t kMasterChannel = 0;   // MIDI channel 1.
    static constexpr uint8_t kMemberChannelFirst = 1;  // MIDI channel 2.
    static constexpr uint8_t kMemberChannelLast = 15;  // MIDI channel 16.
    static constexpr size_t kMaxVoices = 10;

    struct VoiceState {
        bool active = false;
        int32_t pointerId = -1;
        uint8_t channel = 0;
        uint8_t note = 60;
        float startX = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        float pressure = 0.0f;
    };

    explicit MPEEngine(MidiOutput& midiOutput);

    int allocateChannel();
    void releaseChannel(uint8_t channel);

    void noteOn(int32_t touchId, float xNorm, float yNorm, float pressureNorm);
    void noteMove(int32_t touchId, float xNorm, float yNorm, float pressureNorm);
    void noteOff(int32_t touchId);

    const std::array<VoiceState, kMaxVoices>& voices() const;

private:
    VoiceState* findVoiceByPointer(int32_t pointerId);
    VoiceState* findFreeVoiceSlot();

    uint8_t mapXToNote(float xNorm) const;
    uint16_t mapDeltaXToBend(float deltaX) const;
    uint8_t normToMidi7(float v) const;

    MidiOutput& midiOutput_;
    std::array<bool, 16> channelUsed_{};
    std::array<VoiceState, kMaxVoices> voices_{};
};
