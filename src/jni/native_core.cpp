#include <memory>
#include "log.h"
#include "engine_context.h"
#include "scene.h"
#include "render.h"
#include "native_core.h"

using namespace std;

namespace dzy {

NativeCore::NativeCore() {
    ALOGD("NativeCore::NativeCore()");
}

NativeCore::~NativeCore() {
    ALOGD("NativeCore::~NativeCore()");
}

bool NativeCore::init(struct android_app* app) {
    mApp = app;
    mApp->userData = this;
    mApp->onAppCmd = NativeCore::handleAppCmd;
    mApp->onInputEvent = NativeCore::handleInputEvent;
    mEngineContext.reset(new EngineContext);
    mEngineContext->init(shared_from_this());
    bool ret = initActivity();
    if (!ret) ALOGE("Init NativeCore class failed\n");
    return ret;
}

void NativeCore::fini() {
    releaseActivity();
}

int32_t NativeCore::handleInputEvent(struct android_app* app, AInputEvent* event) {
    NativeCore *nativeCore = (NativeCore *)(app->userData);
    return nativeCore->inputEvent(event);
}

void NativeCore::handleAppCmd(struct android_app* app, int32_t cmd) {
    NativeCore *nativeCore = (NativeCore *)(app->userData);
    nativeCore->appCmd(cmd);
}

int32_t NativeCore::inputMotionEvent(int action) {
    int32_t ret = 1;
    switch(action) {
        case AMOTION_EVENT_ACTION_DOWN:
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            break;
        case AMOTION_EVENT_ACTION_UP:
            break;
    }
    return ret;
}

int32_t NativeCore::inputEvent(AInputEvent* event) {
    int32_t ret = 0;
    switch(AInputEvent_getType(event)) {
        case AINPUT_EVENT_TYPE_KEY:
            ret = inputKeyEvent(AKeyEvent_getAction(event), AKeyEvent_getKeyCode(event));
            break;
        case AINPUT_EVENT_TYPE_MOTION:
            ret = inputMotionEvent(AMotionEvent_getAction(event));
            break;
        default:
            ALOGW("Unknown input event");
            break;
    }
    return ret;
}

void NativeCore::appCmd(int32_t cmd) {
    switch (cmd) {
        case APP_CMD_START:
            break;
        case APP_CMD_RESUME:
            break;
        case APP_CMD_GAINED_FOCUS:
            break;

        case APP_CMD_PAUSE:
            break;
        case APP_CMD_LOST_FOCUS:
            break;
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_STOP:
            break;

        case APP_CMD_INIT_WINDOW:
            getEngineContext()->initDisplay();
            getEngineContext()->setRenderState(true);
            break;
        case APP_CMD_TERM_WINDOW:
            getEngineContext()->setRenderState(false);
            getEngineContext()->releaseDisplay();
            break;

        case APP_CMD_WINDOW_RESIZED:
            // Not implemented in NativeActivity framework ?
            break;

        case APP_CMD_DESTROY:
            break;
        default:
            break;
    }
}

int32_t NativeCore::inputKeyEvent(int action, int code) {
    int32_t ret = 0;
    if (action == AKEY_EVENT_ACTION_DOWN) {
        switch(code) {
            case AKEYCODE_BACK:
                ALOGD("AKEYCODE_BACK");
                getEngineContext()->requestQuit();
                break;
            default:
                ALOGD("Not supported key code: %d", code);
                break;
        }
    }
    return ret;
}

bool NativeCore::initActivity() {
    return true;
}

bool NativeCore::releaseActivity() {
    return true;
}

shared_ptr<EngineContext> NativeCore::getEngineContext() {
    return mEngineContext;
}

void NativeCore::mainLoop() {
    int ident;
    int events;
    struct android_poll_source* source;
    shared_ptr<EngineContext> engineContext(getEngineContext());
    app_dummy();

    while (true) {
        // If not animating, block forever waiting for events.
        // otherwise,  loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident = ALooper_pollAll(
                    engineContext->isRendering() ? 0 : -1,
                    NULL, &events, (void**)&source)) >= 0) {
            if (ident == LOOPER_ID_MAIN || ident == LOOPER_ID_INPUT) {
                if (source != NULL) {
                    source->process(mApp, source);
                }
            }

            if (mApp->destroyRequested != 0) {
                ALOGD("destroy request received");
                return;
            }
        }

        // check needQuit() to give an early chance to stop drawing.
        if (engineContext->isRendering() && !engineContext->needQuit()) {
            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            getEngineContext()->updateDisplay();
        }
    }

}

} // namespace

extern dzy::NativeCore * dzyCreateNativeActivity();

void android_main(struct android_app* app) {
    shared_ptr<dzy::NativeCore> nativeCore(dzyCreateNativeActivity());
    if (!nativeCore->init(app))
        return;
    nativeCore->mainLoop();
    nativeCore->fini();
}
