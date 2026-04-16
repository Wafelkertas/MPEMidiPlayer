#pragma once

#include <array>
#include <atomic>
#include <cstdint>

struct RenderTouchPoint {
    bool active = false;
    float x = 0.0f;
    float y = 0.0f;
    float pressure = 0.0f;
    uint8_t channel = 0;
};

struct RenderSnapshot {
    std::array<RenderTouchPoint, 10> touches{};
};

class DoubleBufferedRenderState {
public:
    void publish(const RenderSnapshot& snapshot) {
        buffers_[writeIndex_] = snapshot;
        readIndex_.store(writeIndex_, std::memory_order_release);
        writeIndex_ = 1 - writeIndex_;
    }

    RenderSnapshot consumeLatest() const {
        return buffers_[readIndex_.load(std::memory_order_acquire)];
    }

private:
    std::array<RenderSnapshot, 2> buffers_{};
    mutable std::atomic<int> readIndex_{0};
    int writeIndex_ = 1;
};
