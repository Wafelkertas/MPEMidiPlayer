#include "touch_input.h"

#include <algorithm>

TouchInput::TouchInput(MPEEngine& mpeEngine, DoubleBufferedRenderState& renderState)
    : mpeEngine_(mpeEngine), renderState_(renderState) {}

int32_t TouchInput::handleInputEvent(AInputEvent* event, int32_t width, int32_t height) {
    if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION || width <= 0 || height <= 0) {
        return 0;
    }

    const int32_t action = AMotionEvent_getAction(event);
    const int32_t masked = action & AMOTION_EVENT_ACTION_MASK;
    const int32_t index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                          AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    auto normX = [width](float x) { return std::clamp(x / static_cast<float>(width), 0.0f, 1.0f); };
    auto normY = [height](float y) { return std::clamp(y / static_cast<float>(height), 0.0f, 1.0f); };

    if (masked == AMOTION_EVENT_ACTION_DOWN || masked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        const int32_t pointerId = AMotionEvent_getPointerId(event, index);
        mpeEngine_.noteOn(pointerId,
                          normX(AMotionEvent_getX(event, index)),
                          normY(AMotionEvent_getY(event, index)),
                          AMotionEvent_getPressure(event, index));
        updateRenderSnapshot();
        return 1;
    }

    if (masked == AMOTION_EVENT_ACTION_MOVE) {
        const size_t count = AMotionEvent_getPointerCount(event);
        for (size_t i = 0; i < count; ++i) {
            const int32_t pointerId = AMotionEvent_getPointerId(event, static_cast<size_t>(i));
            mpeEngine_.noteMove(pointerId,
                                normX(AMotionEvent_getX(event, static_cast<size_t>(i))),
                                normY(AMotionEvent_getY(event, static_cast<size_t>(i))),
                                AMotionEvent_getPressure(event, static_cast<size_t>(i)));
        }
        updateRenderSnapshot();
        return 1;
    }

    if (masked == AMOTION_EVENT_ACTION_UP || masked == AMOTION_EVENT_ACTION_POINTER_UP ||
        masked == AMOTION_EVENT_ACTION_CANCEL) {
        const int32_t pointerId = AMotionEvent_getPointerId(event, index);
        mpeEngine_.noteOff(pointerId);
        updateRenderSnapshot();
        return 1;
    }

    return 0;
}

void TouchInput::updateRenderSnapshot() {
    RenderSnapshot snapshot{};
    const auto& voices = mpeEngine_.voices();
    for (size_t i = 0; i < voices.size(); ++i) {
        if (!voices[i].active) {
            continue;
        }
        snapshot.touches[i].active = true;
        snapshot.touches[i].x = voices[i].x;
        snapshot.touches[i].y = voices[i].y;
        snapshot.touches[i].pressure = voices[i].pressure;
        snapshot.touches[i].channel = voices[i].channel;
    }
    renderState_.publish(snapshot);
}
