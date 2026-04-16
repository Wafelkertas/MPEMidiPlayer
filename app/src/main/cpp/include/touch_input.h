#pragma once

#include <android/input.h>

#include "mpe_engine.h"
#include "render_state.h"

class TouchInput {
public:
    TouchInput(MPEEngine& mpeEngine, DoubleBufferedRenderState& renderState);

    int32_t handleInputEvent(AInputEvent* event, int32_t width, int32_t height);

private:
    void updateRenderSnapshot();

    MPEEngine& mpeEngine_;
    DoubleBufferedRenderState& renderState_;
};
