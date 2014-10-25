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
    explicit AppContext();
    ~AppContext();
    // init non-trivial class members that cannot be put in ctor
    void                        init(std::shared_ptr<NativeApp> nativeApp);

    // OS specific
    static const std::string    getAppName();
    static const std::string    getAppName(pid_t pid);
    const std::string           getExternalDataDir();
    const std::string           getInternalDataDir();
    bool                        listAssetFiles(const std::string &dir);

    AAssetManager*              getAssetManager();
    std::shared_ptr<NativeApp>  getNativeApp();
    std::shared_ptr<Render>     getDefaultRender();

    // gfx system
    bool            initDisplay();
    void            releaseDisplay();
    bool            updateDisplay();

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
    const char*     eglStatusStr() const;

    AAssetManager*              mAssetManager;
    std::weak_ptr<NativeApp>    mNativeApp;
    std::shared_ptr<Render>     mRender;

    bool                        mRequestQuit;
    bool                        mRendering;

    // egl
    EGLDisplay                  mDisplay;
    EGLContext                  mEglContext;
    EGLSurface                  mSurface;
    EGLint                      mWidth;
    EGLint                      mHeight;

    // OS specific
    std::string                 mInternalDataPath;
};

} // namespace dzy

#endif
