#include <android/asset_manager.h>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include "log.h"
#include "native_app.h"
#include "scene.h"
#include "render.h"
#include "app_context.h"

using namespace std;

namespace dzy {

AppContext::AppContext(NativeApp* nativeApp)
    : mRequestQuit  (false)
    , mRendering    (false)
    , mNativeApp    (nativeApp)
    , mAssetManager (nativeApp->mApp->activity->assetManager)
    , mDisplay      (EGL_NO_DISPLAY)
    , mEglContext   (EGL_NO_CONTEXT)
    , mSurface      (EGL_NO_SURFACE) {
    ALOGD("AppContext::AppContext()");
    RenderManager::get()->createDefaultRender();
}

AppContext::~AppContext() {
    ALOGD("AppContext::~AppContext()");
}

bool AppContext::initDisplay() {
    EGLint      w, h, format, numConfigs;
    EGLConfig   config;
    EGLSurface  surface;
    EGLContext  context;
    EGLint      majorVersion;
    EGLint      minorVersion;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_DEFAULT_DISPLAY) {
        ALOGE("Unable to connect window system: %s", eglStatusStr());
        return false;
    }
    if (!eglInitialize(display, &majorVersion, &minorVersion)) {
        ALOGE("Unable to initialize egl: %s", eglStatusStr());
        return false;
    }

    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
        EGL_BLUE_SIZE,          8,
        EGL_GREEN_SIZE,         8,
        EGL_RED_SIZE,           8,
        EGL_ALPHA_SIZE,         8,
        EGL_DEPTH_SIZE,         8,
        EGL_STENCIL_SIZE,       8,
        EGL_SAMPLE_BUFFERS,     1,
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs)
        && numConfigs < 1){
        ALOGE("Unable to choose egl config: %s", eglStatusStr());
        return false;
    }

    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(mNativeApp->mApp->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, mNativeApp->mApp->window, NULL);
    if (surface == EGL_NO_SURFACE) {
        ALOGW("Unable to create surface: %s", eglStatusStr());
        return false;
    }
    EGLint contextAttrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    context = eglCreateContext(display, config, NULL, contextAttrs);
    if (context == EGL_NO_CONTEXT) {
        ALOGW("Unable to create context: %s", eglStatusStr());
        return false;
    }
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        ALOGW("Unable to eglMakeCurrent");
        return false;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    mDisplay    = display;
    mEglContext = context;
    mSurface    = surface;
    mWidth      = w;
    mHeight     = h;

    mNativeApp->initView(mNativeApp->getCurrentScene());
    mNativeApp->drawScene(mNativeApp->getCurrentScene());
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

bool AppContext::updateDisplay(shared_ptr<Scene> scene) {
    mNativeApp->drawScene(scene);
    return true;
}

NativeApp* AppContext::getNativeApp() {
    return mNativeApp;
}

AAssetManager* AppContext::getAssetManager() {
    return mAssetManager;
}

const string AppContext::getAppName() {
    return getAppName(getpid());
}

const string AppContext::getAppName(pid_t pid) {
    stringstream ss;
    ss << "/proc/" << pid << "/cmdline";

    ifstream ifs(ss.str(), ifstream::in);
    ss.clear();
    char c = ifs.get();
    while (ifs.good()) {
        ss << c;
        c = ifs.get();
    }
    ifs.close();
    
    return ss.str();

}

const string AppContext::getExternalDataDir() {
    stringstream ss;
    ss << "/sdcard/" << getAppName();
    return ss.str();
}

const string AppContext::getInternalDataDir() {
    const char *idp = mNativeApp->mApp->activity->internalDataPath;
    if (!idp) {
        string appName = getAppName();
        ALOGW("app's internal data path set to /data/data/%s",
            appName.c_str());
        return "/data/data/" + appName;
    }
    return idp;
}

void AppContext::requestQuit() {
    mRequestQuit = true;
}

bool AppContext::needQuit() {
    return mRequestQuit;
}

void AppContext::setRenderState(bool rendering) {
    mRendering = rendering;
}

bool AppContext::isRendering() {
    return mRendering;
}

} // namespace dzy
