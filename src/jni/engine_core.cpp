#include <memory>
#include "log.h"
#include "engine_context.h"
#include "scene.h"
#include "render.h"
#include "utils.h"
#include "engine_core.h"

using namespace std;

namespace dzy {

EngineCore::EngineCore()
    : mFirstFrameUpdated(false)
    , mJNIEnv(nullptr) {
    ALOGV("EngineCore::EngineCore()");
}

EngineCore::~EngineCore() {
    ALOGV("EngineCore::~EngineCore()");
}

bool EngineCore::init(struct android_app* app) {
    mApp = app;
    mApp->userData = this;
    mApp->onAppCmd = EngineCore::onAppCmd;
    mApp->onInputEvent = EngineCore::onInputEvent;
    mEngineContext.reset(new EngineContext);
    mEngineContext->init(shared_from_this());
    mDoubleTapDetector.SetConfiguration(mApp->config);
    mDragDetector.SetConfiguration(mApp->config);
    mPinchDetector.SetConfiguration(mApp->config);

    JNIEnv *env;
    mApp->activity->vm->AttachCurrentThread(&env, 0);
    if (!env) {
        ALOGE("failed to attach native activity thread");
        return false;
    }
    mJNIEnv = env;
    bool ret = create();
    if (!ret) ALOGE("Init Activity failed\n");
    return ret;
}

void EngineCore::fini() {
    destory();
    mApp->activity->vm->DetachCurrentThread();
}

int32_t EngineCore::onInputEvent(struct android_app* app, AInputEvent* event) {
    EngineCore *engineCore = (EngineCore *)(app->userData);
    bool handled = engineCore->inputEvent(event);
    if (!handled) return 0;
    else return 1;
}

void EngineCore::onAppCmd(struct android_app* app, int32_t cmd) {
    EngineCore *engineCore = (EngineCore *)(app->userData);
    engineCore->appCmd(cmd);
}

bool EngineCore::inputMotionEvent(int action) {
    bool ret = handleMotion(action);
    if (!ret) {
        switch(action) {
            case AMOTION_EVENT_ACTION_DOWN:
                ret = true;
                break;
            case AMOTION_EVENT_ACTION_MOVE:
                ret = true;
                break;
            case AMOTION_EVENT_ACTION_UP:
                ret = true;
                break;
        }
    }
    return ret;
}

bool EngineCore::inputEvent(AInputEvent* event) {
    bool handled = false;
    bool doubleTapHandled = false;
    bool dragHandled = false;
    bool pinchHandled = false;

    switch(AInputEvent_getType(event)) {
        case AINPUT_EVENT_TYPE_KEY:
            handled = inputKeyEvent(AKeyEvent_getAction(event), AKeyEvent_getKeyCode(event));
            break;
        case AINPUT_EVENT_TYPE_MOTION: {
            ndk_helper::GESTURE_STATE doubleTapState = mDoubleTapDetector.Detect(event);
            ndk_helper::GESTURE_STATE dragState = mDragDetector.Detect(event);
            ndk_helper::GESTURE_STATE pinchState = mPinchDetector.Detect(event);
            if (doubleTapState == ndk_helper::GESTURE_STATE_ACTION) {
                doubleTapHandled = handleDoubleTap();
            } else {
                if (dragState & ndk_helper::GESTURE_STATE_START) {
                    ndk_helper::Vec2 v;
                    mDragDetector.GetPointer(v);
                    float x, y;
                    v.Value(x, y);
                    dragHandled = handleDrag(GESTURE_DRAG_START, x, y);
                } else if (dragState & ndk_helper::GESTURE_STATE_MOVE) {
                    ndk_helper::Vec2 v;
                    mDragDetector.GetPointer(v);
                    float x, y;
                    v.Value(x, y);
                    dragHandled = handleDrag(GESTURE_DRAG_MOVE, x, y);
                } else if (dragState & ndk_helper::GESTURE_STATE_END) {
                    dragHandled = handleDrag(GESTURE_DRAG_END, 0, 0);
                }

                if (pinchState & ndk_helper::GESTURE_STATE_START) {
                    ndk_helper::Vec2 v1;
                    ndk_helper::Vec2 v2;
                    mPinchDetector.GetPointers(v1, v2);
                    float x1, y1, x2, y2;
                    v1.Value(x1, y1);
                    v2.Value(x2, y2);
                    pinchHandled = handlePinch(GESTURE_PINCH_START, x1, y1, x2, y2);
                } else if (pinchState & ndk_helper::GESTURE_STATE_MOVE) {
                    ndk_helper::Vec2 v1;
                    ndk_helper::Vec2 v2;
                    mPinchDetector.GetPointers(v1, v2);
                    float x1, y1, x2, y2;
                    v1.Value(x1, y1);
                    v2.Value(x2, y2);
                    pinchHandled = handlePinch(GESTURE_PINCH_MOVE, x1, y1, x2, y2);
                }
            }

            if (!doubleTapHandled && !dragHandled && !pinchHandled)
                handled = inputMotionEvent(AMotionEvent_getAction(event));
            break;
        }
        default:
            ALOGW("Unknown input event");
            break;
    }

    return doubleTapHandled || dragHandled || pinchHandled || handled;
}

bool EngineCore::updateFrame() {
    if (!mFirstFrameUpdated) {
        mLastUpdated = std::chrono::high_resolution_clock::now();
        mFirstFrameUpdated = true;
    }
    long interval = MeasureDuration::getInterval(mLastUpdated);
    mLastUpdated = std::chrono::high_resolution_clock::now();
    return update(interval);
}

void EngineCore::appCmd(int32_t cmd) {
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
            if (getEngineContext()->initDisplay())
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

bool EngineCore::inputKeyEvent(int action, int code) {
    bool handled = false;
    if (action == AKEY_EVENT_ACTION_DOWN) {
        // only handle key in action down
        handled = handleKey(code);
        if (!handled) {
            switch(code) {
                case AKEYCODE_BACK:
                    ALOGV("AKEYCODE_BACK");
                    getEngineContext()->requestQuit();
                    // need framework to further process this key event
                    handled = false;
                    break;
                default:
                    ALOGD("Not supported key code: %d", code);
                    break;
            }
        }
        // AKEYCODE_BACK needs special attention
        else if (code == AKEYCODE_BACK) {
            ALOGV("AKEYCODE_BACK");
            // need framework to further process this key event
            handled = false;
            getEngineContext()->requestQuit();
        }
    }
    return handled;
}

bool EngineCore::create() {
    return true;
}

void EngineCore::destory() {
}

bool EngineCore::start() {
    return true;
}

void EngineCore::stop() {
}

bool EngineCore::update(long interval) {
    return true;
}

bool EngineCore::handleMotion(int action) {
    return false;
}

bool EngineCore::handleKey(int code) {
    return false;
}

bool EngineCore::handleDoubleTap() {
    return false;
}

bool EngineCore::handleDrag(GestureState state, float x, float y) {
    return false;
}

bool EngineCore::handlePinch(GestureState state, float x1, float y1, float x2, float y2) {
    return false;
}

string EngineCore::getIntentString(const string& name) {
    jobject nativeActivityObj = mApp->activity->clazz;
    jclass nativeActivityClass = mJNIEnv->GetObjectClass(nativeActivityObj);
    jmethodID getIntentMethodID = mJNIEnv->GetMethodID(
        nativeActivityClass, "getIntent", "()Landroid/content/Intent;");
    jobject intentObj = mJNIEnv->CallObjectMethod(
        nativeActivityObj, getIntentMethodID);
    jclass intentClass = mJNIEnv->GetObjectClass(intentObj);

    jmethodID getStringExtraMethodID = mJNIEnv->GetMethodID(intentClass,
        "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");
    jstring jsString = (jstring)mJNIEnv->CallObjectMethod(intentObj,
        getStringExtraMethodID, mJNIEnv->NewStringUTF(name.c_str()));
    if (!jsString) return "";

    const char *rawString = mJNIEnv->GetStringUTFChars(jsString, 0);
    string ret(rawString);
    mJNIEnv->ReleaseStringUTFChars(jsString, rawString);

    return ret;
}

shared_ptr<EngineContext> EngineCore::getEngineContext() {
    return mEngineContext;
}

void EngineCore::mainLoop() {
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

extern dzy::EngineCore * engine_main();

void android_main(struct android_app* app) {
    shared_ptr<dzy::EngineCore> engineCore(engine_main());
    if (!engineCore->init(app))
        return;
    engineCore->mainLoop();
    engineCore->fini();
}
