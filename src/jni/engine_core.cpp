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
    mApp->onAppCmd = EngineCore::handleAppCmd;
    mApp->onInputEvent = EngineCore::handleInputEvent;
    mEngineContext.reset(new EngineContext);
    mEngineContext->init(shared_from_this());

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

int32_t EngineCore::handleInputEvent(struct android_app* app, AInputEvent* event) {
    EngineCore *engineCore = (EngineCore *)(app->userData);
    return engineCore->inputEvent(event);
}

void EngineCore::handleAppCmd(struct android_app* app, int32_t cmd) {
    EngineCore *engineCore = (EngineCore *)(app->userData);
    engineCore->appCmd(cmd);
}

int32_t EngineCore::inputMotionEvent(int action) {
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

int32_t EngineCore::inputEvent(AInputEvent* event) {
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

int32_t EngineCore::inputKeyEvent(int action, int code) {
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

bool EngineCore::create() {
    return true;
}

void EngineCore::destory() {
}

bool EngineCore::update(long interval) {
    return true;
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
