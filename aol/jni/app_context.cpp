#include <android/sensor.h>
#include <android_native_app_glue.h>
#include "log.h"
#include "app_context.h"

namespace dzy {

AppContext::AppContext()
    : mNativeApp(NULL) {
}

AppContext::~AppContext() {
}

void AppContext::handleAppCmd(struct android_app* app, int32_t cmd) {
    AppContext* context = (AppContext*) app->userData;

    switch (cmd) {
        case APP_CMD_START:
            ALOGD("APP_CMD_START");
            break;
        case APP_CMD_RESUME:
            ALOGD("APP_CMD_RESUME");
            break;
        case APP_CMD_PAUSE:
            ALOGD("APP_CMD_PAUSE");
            break;
        case APP_CMD_SAVE_STATE:
            ALOGD("APP_CMD_SAVE_STATE");
            break;
        case APP_CMD_INIT_WINDOW:
            ALOGD("APP_CMD_INIT_WINDOW");
            break;
        case APP_CMD_WINDOW_RESIZED:
            // Unsupported
            ALOGD("APP_CMD_WINDOW_RESIZED");
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed.
            ALOGD("APP_CMD_TERM_WINDOW");
            break;
        case APP_CMD_GAINED_FOCUS:
            ALOGD("APP_CMD_GAINED_FOCUS");
            break;
        case APP_CMD_LOST_FOCUS:
            ALOGD("APP_CMD_LOST_FOCUS");
            break;
        case APP_CMD_STOP:
            ALOGD("APP_CMD_STOP");
            break;
        case APP_CMD_DESTROY:
            ALOGD("APP_CMD_DESTROY");
            break;
        default:
            break;
    }
}

int32_t AppContext::handleInputEvent(struct android_app* app, AInputEvent* event) {
    AppContext* context = (AppContext*) app->userData;
    if (context) {
        return 1;
    }

    switch(AInputEvent_getType(event)) {
        case AINPUT_EVENT_TYPE_KEY:
            ALOGD("AINPUT_EVENT_TYPE_KEY");
            break;
        case AINPUT_EVENT_TYPE_MOTION:
            switch(AMotionEvent_getAction(event)) {
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
            break;
    }

    return 1;
}

} // namespace dzy

using namespace dzy;
void android_main(struct android_app* app) {
    app_dummy();
    
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
        
        do {
            //if(!init.Run()) {
            //    ANativeActivity_finish(state->activity);
            //    break;
            //}
        } while( 0 );
    }

    ANativeActivity_finish(app->activity);
}
