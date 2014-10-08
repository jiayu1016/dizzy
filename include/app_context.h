#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <android_native_app_glue.h>
#include <memory>
#include <string>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include "utils.h"

class AInputEvent;
class AAssetManager;
struct android_app;

namespace dzy {

class NativeApp;
class Scene;
class AppContext : private noncopyable {
public:
    explicit AppContext(NativeApp* nativeApp);
    ~AppContext();

    // OS specific
    static const std::string getAppName();
    static const std::string getAppName(pid_t pid);
    const std::string getExternalDataDir();
    const std::string getInternalDataDir();

    NativeApp*                  getNativeApp();
    AAssetManager*              getAssetManager();

    // gfx system
    bool initDisplay();
    void releaseDisplay();

    bool update();

    void requestQuit();
    bool needQuit();
    void requestRender();
    void stopRender();
    bool needRender();

private:
    const char* eglStatusStr() const;

    NativeApp*              mNativeApp;
    AAssetManager*          mAssetManager;
    bool                    mRequestQuit;
    bool                    mRequestRender;

    // egl
    EGLDisplay              mDisplay;
    EGLContext              mEglContext;
    EGLSurface              mSurface;
    EGLint                  mWidth;
    EGLint                  mHeight;
};

} // namespace dzy

#endif
