#include <android/log.h>
#include <android_native_app_glue.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "midi_output.h"
#include "mpe_engine.h"
#include "touch_input.h"
#include "vulkan_renderer.h"

namespace {
constexpr char kTag[] = "MPEApp";

struct AppContext {
    android_app* app = nullptr;
    std::atomic<bool> running{true};
    std::atomic<bool> rendererReady{false};

    MidiOutput midiOutput;
    MPEEngine mpeEngine{midiOutput};
    DoubleBufferedRenderState renderState;
    TouchInput touchInput{mpeEngine, renderState};
    VulkanRenderer renderer;

    std::thread renderThread;
};

void renderLoop(AppContext* ctx) {
    constexpr auto frameInterval = std::chrono::milliseconds(16);
    while (ctx->running.load(std::memory_order_relaxed)) {
        if (!ctx->rendererReady.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        const RenderSnapshot snapshot = ctx->renderState.consumeLatest();
        ctx->renderer.render(snapshot);
        std::this_thread::sleep_for(frameInterval);
    }
}

void handleCmd(android_app* app, int32_t cmd) {
    auto* ctx = reinterpret_cast<AppContext*>(app->userData);
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != nullptr && ctx->renderer.initialize(app->window)) {
                ctx->rendererReady.store(true, std::memory_order_release);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            ctx->rendererReady.store(false, std::memory_order_release);
            ctx->renderer.shutdown();
            break;
        default:
            break;
    }
}

int32_t handleInput(android_app* app, AInputEvent* event) {
    auto* ctx = reinterpret_cast<AppContext*>(app->userData);
    int32_t width = 0;
    int32_t height = 0;
    if (app->window != nullptr) {
        width = ANativeWindow_getWidth(app->window);
        height = ANativeWindow_getHeight(app->window);
    }
    return ctx->touchInput.handleInputEvent(event, width, height);
}
}  // namespace

void android_main(android_app* app) {
    app_dummy();

    AppContext ctx;
    ctx.app = app;
    app->userData = &ctx;
    app->onAppCmd = handleCmd;
    app->onInputEvent = handleInput;

    ctx.renderThread = std::thread(renderLoop, &ctx);

    while (ctx.running.load(std::memory_order_relaxed)) {
        int events;
        android_poll_source* source;
        while (ALooper_pollOnce(0, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0) {
            if (source != nullptr) {
                source->process(app, source);
            }
            if (app->destroyRequested) {
                ctx.running.store(false, std::memory_order_relaxed);
                break;
            }
        }
    }

    ctx.rendererReady.store(false, std::memory_order_release);
    ctx.renderer.shutdown();

    if (ctx.renderThread.joinable()) {
        ctx.renderThread.join();
    }
}
