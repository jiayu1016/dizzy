#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <android_native_app_glue.h>
#include <memory>
#include <string>
#include <EGL/egl.h>
#include "utils.h"

class AInputEvent;
class AAssetManager;
struct android_app;

namespace dzy {

class NativeApp;
class Scene;
class Render;
class AppContext
    : public std::enable_shared_from_this<AppContext>
    , private noncopyable {
public:
    explicit AppContext(NativeApp* nativeApp);
    ~AppContext();

    // OS specific
    static const std::string    getAppName();
    static const std::string    getAppName(pid_t pid);
    const std::string           getExternalDataDir();
    const std::string           getInternalDataDir();

    NativeApp*                  getNativeApp();
    AAssetManager*              getAssetManager();
    std::shared_ptr<Render>     getRender();

    // gfx system
    bool            initDisplay();
    void            releaseDisplay();
    bool            updateDisplay(std::shared_ptr<Scene> scene);

    void            requestQuit();
    bool            needQuit();
    void            setRenderState(bool rendering);
    bool            isRendering();

    EGLDisplay      getEGLDisplay();
    EGLContext      getEGLContext();
    EGLSurface      getEGLSurface();
    EGLint          getSurfaceWidth();
    EGLint          getSurfaceHeight();

private:
    const char* eglStatusStr() const;

    NativeApp*              mNativeApp;
    AAssetManager*          mAssetManager;
    std::shared_ptr<Render> mRender;

    bool                    mRequestQuit;
    bool                    mRendering;

    // egl
    EGLDisplay              mDisplay;
    EGLContext              mEglContext;
    EGLSurface              mSurface;
    EGLint                  mWidth;
    EGLint                  mHeight;
};

} // namespace dzy

#endif
