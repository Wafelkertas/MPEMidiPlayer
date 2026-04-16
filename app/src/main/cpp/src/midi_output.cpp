#include "midi_output.h"

#include <android/log.h>

namespace {
constexpr char kTag[] = "MidiOutput";
}

void MidiOutput::attachOutputPort(AMidiOutputPort* port) {
    outputPort_ = port;
}

void MidiOutput::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    send3(static_cast<uint8_t>(0x90u | (channel & 0x0Fu)), note, velocity);
}

void MidiOutput::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    send3(static_cast<uint8_t>(0x80u | (channel & 0x0Fu)), note, velocity);
}

void MidiOutput::sendPitchBend(uint8_t channel, uint16_t bend14) {
    const uint8_t lsb = static_cast<uint8_t>(bend14 & 0x7F);
    const uint8_t msb = static_cast<uint8_t>((bend14 >> 7) & 0x7F);
    send3(static_cast<uint8_t>(0xE0u | (channel & 0x0Fu)), lsb, msb);
}

void MidiOutput::sendCC74(uint8_t channel, uint8_t value) {
    send3(static_cast<uint8_t>(0xB0u | (channel & 0x0Fu)), 74, value);
}

void MidiOutput::sendChannelPressure(uint8_t channel, uint8_t value) {
    send2(static_cast<uint8_t>(0xD0u | (channel & 0x0Fu)), value);
}

void MidiOutput::send3(uint8_t status, uint8_t data1, uint8_t data2) {
    if (outputPort_ == nullptr) {
        return;
    }
    const uint8_t msg[3] = {status, static_cast<uint8_t>(data1 & 0x7F), static_cast<uint8_t>(data2 & 0x7F)};
    AMidiOutputPort_send(outputPort_, msg, sizeof(msg));
}

void MidiOutput::send2(uint8_t status, uint8_t data1) {
    if (outputPort_ == nullptr) {
        return;
    }
    const uint8_t msg[2] = {status, static_cast<uint8_t>(data1 & 0x7F)};
    AMidiOutputPort_send(outputPort_, msg, sizeof(msg));
}
