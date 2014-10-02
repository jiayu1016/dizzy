#include <stdlib.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include "log.h"
#include "native_app.h"
#include "app_context.h"

using namespace std;

namespace dzy {

AppContext::AppContext(struct android_app *app) :
    mRequestQuit(false),
    mRequestRender(false),
    mApp(app),
    mNativeApp(nativeInit()),
    mDisplay(EGL_NO_DISPLAY),
    mEglContext(EGL_NO_CONTEXT),
    mSurface(EGL_NO_SURFACE) {
    mApp->userData = this;
    mApp->onAppCmd = AppContext::handleAppCmd;
    mApp->onInputEvent = AppContext::handleInputEvent;

    if (!mNativeApp->initApp()) {
        ALOGE("Init NativeApp class failed\n");
        exit(-1);
    }
}

AppContext::~AppContext() {
    mNativeApp->releaseApp();
}

bool AppContext::initDisplay() {                                                                      
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(mApp->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, mApp->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        ALOGW("Unable to eglMakeCurrent");
        return false;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    mDisplay = display;
    mEglContext = context;
    mSurface = surface;
    mWidth = w;
    mHeight = h;

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    mNativeApp->initView();
}

const char* AppContext::eglStatusStr() const
{
    EGLint error = eglGetError();

    switch (error) {
        case EGL_SUCCESS: return "EGL_SUCCESS";
        case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
        case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
        case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
        case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
        case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
        case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
        case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
        default: return "UNKNOWN_EGL_ERROR";
    }
}

void AppContext::releaseDisplay() {
    mNativeApp->releaseView();
    if (mDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (mEglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(mDisplay, mEglContext);
        }
        if (mSurface != EGL_NO_SURFACE) {
            eglDestroySurface(mDisplay, mSurface);
        }
        eglTerminate(mDisplay);
    }
    mDisplay    = EGL_NO_DISPLAY;
    mEglContext = EGL_NO_CONTEXT;
    mSurface    = EGL_NO_SURFACE;
}

bool AppContext::update() {
    if (needQuit())
        return false;

    return true;
}

int32_t AppContext::inputKeyEvent(int action, int code) {
    int32_t ret = 0;
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
    return ret;
}

int32_t AppContext::inputMotionEvent(int action) {
    int32_t ret = 1;
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
    return ret;
}

int32_t AppContext::inputEvent(AInputEvent* event) {
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
            initDisplay();
            break;
        case APP_CMD_TERM_WINDOW:
            ALOGD("APP_CMD_TERM_WINDOW");
            releaseDisplay();
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
    AppContext *context = (AppContext *)(app->userData);
    return context->inputEvent(event);
}

void AppContext::handleAppCmd(struct android_app* app, int32_t cmd) {
    AppContext *context = (AppContext *)(app->userData);
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

    shared_ptr<AppContext> context(new AppContext(app));

    int ident;
    int events;
    struct android_poll_source* source;

    while (1) {
        while ((ident = ALooper_pollAll(-1, NULL, &events, (void**)&source)) >= 0) {
            if (ident == LOOPER_ID_MAIN || ident == LOOPER_ID_INPUT) {
                if (source != NULL) {
                    source->process(app, source);
                }
            }

            if (app->destroyRequested != 0) {
                ALOGD("destroy request received");
                context->releaseDisplay();
                return;
            }
        }
    }
}
