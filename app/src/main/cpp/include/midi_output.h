#pragma once

#include <android/midi.h>
#include <array>
#include <cstdint>

class MidiOutput {
public:
    MidiOutput() = default;

    void attachOutputPort(AMidiOutputPort* port);
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendPitchBend(uint8_t channel, uint16_t bend14);
    void sendCC74(uint8_t channel, uint8_t value);
    void sendChannelPressure(uint8_t channel, uint8_t value);

private:
    void send3(uint8_t status, uint8_t data1, uint8_t data2);
    void send2(uint8_t status, uint8_t data1);

    AMidiOutputPort* outputPort_ = nullptr;
};
