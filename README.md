# Android Vulkan MPE MIDI Controller (NDK / C++17)

This project is a native Android app (NativeActivity) that demonstrates a low-latency architecture for an MPE MIDI surface:

- **Core (pure C++):** `MPEEngine` channel/voice logic.
- **Platform:** touch handling through `AInputEvent`, MIDI output through `AMidiOutputPort`.
- **Renderer:** Vulkan-only rendering thread consuming immutable snapshots.

## Architecture

- **Input thread (looper):** handles `ACTION_DOWN/POINTER_DOWN/MOVE/UP/POINTER_UP`, updates `MPEEngine`, publishes render snapshot.
- **Render thread:** reads latest snapshot from lock-free double buffer and renders via Vulkan.

## MPE behavior

- Channel 1 is master.
- Channels 2-16 are allocated per active note.
- Each touch maps to a unique member channel with independent pitch bend, CC74, and channel pressure.

## Build

1. Install Android SDK + NDK.
2. Open in Android Studio.
3. Build and run `app` module.

## MIDI wiring notes

`MidiOutput` expects a valid `AMidiOutputPort*` set through `attachOutputPort()`. The sample focuses on architecture/perf and leaves device/session setup to host integration.
