#include <stdlib.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include "log.h"
#include "native_app.h"
#include "app_context.h"

using namespace std;

namespace dzy {

AppContext::AppContext() :
    mRequestQuit(false),
    mRequestRender(false),
    mNativeApp(nativeInit()) {
    if (!mNativeApp->initApp()) {
        ALOGE("Init NativeApp class failed\n");
        exit(-1);
    }
}

AppContext::~AppContext() {
}

bool AppContext::update() {
    if (needQuit())
        return false;

    return true;
}

void AppContext::inputKeyEvent(int action, int code) {
    if (action == AKEY_EVENT_ACTION_DOWN) {
        switch(code) {
            case AKEYCODE_BACK:
                ALOGD("AKEYCODE_BACK");
                requestQuit();
                break;
            default:
                ALOGD("Not supported key code: %d", code);
                break;
        }
    }
}

void AppContext::inputMotionEvent(int action) {
    switch(action) {
        case AMOTION_EVENT_ACTION_DOWN:
            ALOGD("AMOTION_EVENT_ACTION_DOWN");
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            ALOGD("AMOTION_EVENT_ACTION_MOVE");
            break;
        case AMOTION_EVENT_ACTION_UP:
            ALOGD("AMOTION_EVENT_ACTION_UP");
            break;
    }
}

int32_t AppContext::inputEvent(AInputEvent* event) {
    switch(AInputEvent_getType(event)) {
        case AINPUT_EVENT_TYPE_KEY:
            inputKeyEvent(AKeyEvent_getAction(event), AKeyEvent_getKeyCode(event));
            break;
        case AINPUT_EVENT_TYPE_MOTION:
            inputMotionEvent(AMotionEvent_getAction(event));
            break;
        default:
            ALOGW("Unknown input event");
            break;
    }
    return 1;
}

void AppContext::appCmd(int32_t cmd) {
    switch (cmd) {
        case APP_CMD_START:
            ALOGD("APP_CMD_START");
            break;
        case APP_CMD_RESUME:
            ALOGD("APP_CMD_RESUME");
            break;
        case APP_CMD_GAINED_FOCUS:
            ALOGD("APP_CMD_GAINED_FOCUS");
            requestRender();
            break;

        case APP_CMD_PAUSE:
            ALOGD("APP_CMD_PAUSE");
            break;
        case APP_CMD_LOST_FOCUS:
            ALOGD("APP_CMD_LOST_FOCUS");
            stopRender();
            break;
        case APP_CMD_SAVE_STATE:
            ALOGD("APP_CMD_SAVE_STATE");
            break;
        case APP_CMD_STOP:
            ALOGD("APP_CMD_STOP");
            break;

        case APP_CMD_INIT_WINDOW:
            ALOGD("APP_CMD_INIT_WINDOW");
            //mNativeApp->initView();
            break;
        case APP_CMD_TERM_WINDOW:
            ALOGD("APP_CMD_TERM_WINDOW");
            //mNativeApp->releaseView();
            break;

        case APP_CMD_WINDOW_RESIZED:
            // Not implemented in NativeActivity framework ?
            ALOGD("APP_CMD_WINDOW_RESIZED");
            break;

        case APP_CMD_DESTROY:
            ALOGD("APP_CMD_DESTROY");
            break;
        default:
            break;
    }
}

int32_t AppContext::handleInputEvent(struct android_app* app, AInputEvent* event) {
    //shared_ptr<AppContext> context(static_cast<AppContext *>(app->userData));
    AppContext *context= (AppContext *)(app->userData);
    return context->inputEvent(event);
}

void AppContext::handleAppCmd(struct android_app* app, int32_t cmd) {
    //shared_ptr<AppContext> context(static_cast<AppContext *>(app->userData));
    AppContext *context= (AppContext *)(app->userData);
    context->appCmd(cmd);
}

void AppContext::requestQuit() {
    mRequestQuit = true;
}

bool AppContext::needQuit() {
    return mRequestQuit;
}

void AppContext::requestRender() {
    mRequestRender = true;
}

void AppContext::stopRender() {
    mRequestRender = false;
}

bool AppContext::needRender() {
    return mRequestRender;
}


} // namespace dzy

using namespace dzy;
void android_main(struct android_app* app) {
    app_dummy();

    //shared_ptr<AppContext> context(new AppContext);
    AppContext context;
    app->userData = &context;
    app->onAppCmd = AppContext::handleAppCmd;
    app->onInputEvent = AppContext::handleInputEvent;

    int ident;
    int events;
    struct android_poll_source* source;

    while (1) {
        while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) {
                source->process(app, source);
            }

            if (app->destroyRequested != 0) {
                ALOGD("exit android_main");
                return;
            }
        }

        // seperate render loop, or couple render loop in ALooper, which one is better?
        do {
            if(!context.update()) {
                ANativeActivity_finish(app->activity);
                break;
            }
        } while(0);

        if (context.needQuit()) break;
    }

    ANativeActivity_finish(app->activity);
}
