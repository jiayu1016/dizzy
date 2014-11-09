#include <android/asset_manager.h>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include "log.h"
#include "native_core.h"
#include "scene.h"
#include "render.h"
#include "program.h"
#include "engine_context.h"

using namespace std;

namespace dzy {

EngineContext::EngineContext()
    : mRequestQuit  (false)
    , mRendering    (false)
    , mRender       (new Render)
    , mDisplay      (EGL_NO_DISPLAY)
    , mEglContext   (EGL_NO_CONTEXT)
    , mSurface      (EGL_NO_SURFACE) {
    ALOGD("EngineContext::EngineContext()");
}

EngineContext::~EngineContext() {
    ALOGD("EngineContext::~EngineContext()");
}

void EngineContext::init(shared_ptr<NativeCore> nativeCore) {
    mAssetManager = nativeCore->mApp->activity->assetManager;
    mInternalDataPath = nativeCore->mApp->activity->internalDataPath;
    mNativeCore = nativeCore;
}

bool EngineContext::initDisplay() {
    shared_ptr<NativeCore> nativeCore(getNativeCore());
    if (!nativeCore) {
        ALOGE("NativeCore released before EngineContext");
        return false;
    }
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_DEFAULT_DISPLAY) {
        ALOGE("Unable to connect window system: %s", eglStatusStr());
        return false;
    }
    EGLint majorVersion, minorVersion;
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

    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs)
        && numConfigs < 1){
        ALOGE("Unable to choose egl config: %s", eglStatusStr());
        return false;
    }

    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(nativeCore->mApp->window, 0, 0, format);

    EGLSurface surface = eglCreateWindowSurface(display, config, nativeCore->mApp->window, NULL);
    if (surface == EGL_NO_SURFACE) {
        ALOGW("Unable to create surface: %s", eglStatusStr());
        return false;
    }
    EGLint contextAttrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    EGLContext context = eglCreateContext(display, config, NULL, contextAttrs);
    if (context == EGL_NO_CONTEXT) {
        ALOGW("Unable to create context: %s", eglStatusStr());
        return false;
    }
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        ALOGW("Unable to eglMakeCurrent");
        return false;
    }

    ALOGD("%s", glGetString(GL_VENDOR));
    ALOGD("%s", glGetString(GL_RENDERER));
    ALOGD("%s", glGetString(GL_VERSION));
    ALOGD("%s", glGetString(GL_EXTENSIONS));

    EGLint w, h;
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    mDisplay    = display;
    mEglContext = context;
    mSurface    = surface;
    mWidth      = w;
    mHeight     = h;
    mRender->setEngineContext(shared_from_this());

    // ProgramManager::preCompile must be called after setting up egl context
    //ProgramManager::get()->preCompile(shared_from_this());
    nativeCore->initView();
}

const char* EngineContext::eglStatusStr() const {
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

void EngineContext::releaseDisplay() {
    shared_ptr<NativeCore> nativeCore(getNativeCore());
    if (!nativeCore) {
        ALOGE("NativeCore released before EngineContext");
        return;
    }
    nativeCore->releaseView();
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

bool EngineContext::updateDisplay() {
    shared_ptr<NativeCore> nativeCore(getNativeCore());
    if (!nativeCore) {
        ALOGE("NativeCore released before EngineContext");
        return false;
    }
    nativeCore->drawScene();
    return true;
}

AAssetManager* EngineContext::getAssetManager() {
    return mAssetManager;
}

shared_ptr<NativeCore> EngineContext::getNativeCore() {
    return mNativeCore.lock();
}

shared_ptr<Render> EngineContext::getDefaultRender() {
    return mRender;
}

const string EngineContext::getAppName() {
    return getAppName(getpid());
}

const string EngineContext::getAppName(pid_t pid) {
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

const string EngineContext::getExternalDataDir() {
    stringstream ss;
    ss << "/sdcard/" << getAppName();
    return ss.str();
}

const string EngineContext::getInternalDataDir() {
    if (!mInternalDataPath.empty()) {
        string appName = getAppName();
        ALOGW("app's internal data path set to /data/data/%s",
            appName.c_str());
        return "/data/data/" + appName;
    }
    return mInternalDataPath;
}

bool EngineContext::listAssetFiles(const string &dir) {
    assert(mAssetManager != NULL);

    AAssetDir* assetDir = AAssetManager_openDir(mAssetManager, dir.c_str());
    if (!assetDir) {
        ALOGE("Failed to open asset root dir");
        return false;
    }

    const char * assetFile = NULL;
    while (assetFile = AAssetDir_getNextFileName(assetDir)) {
        ALOGD("%s", assetFile);
    }

    AAssetDir_close(assetDir);

    return true;
}

void EngineContext::requestQuit() {
    mRequestQuit = true;
}

bool EngineContext::needQuit() {
    return mRequestQuit;
}

void EngineContext::setRenderState(bool rendering) {
    mRendering = rendering;
}

bool EngineContext::isRendering() {
    return mRendering;
}

EGLDisplay EngineContext::getEGLDisplay() {
    return mDisplay;
}

EGLContext EngineContext::getEGLContext() {
    return mEglContext;
}

EGLSurface EngineContext::getEGLSurface() {
    return mSurface;
}

EGLint EngineContext::getSurfaceWidth() {
    return mWidth;
}

EGLint EngineContext::getSurfaceHeight() {
    return mHeight;
}


} // namespace dzy
