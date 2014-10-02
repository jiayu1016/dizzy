#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <android_native_app_glue.h>
#include <memory>
#include <EGL/egl.h>
#include <GLES/gl.h>

class AInputEvent;
struct android_app;

namespace dzy {

class NativeApp;
class AppContext {
public:
    AppContext(NativeApp* nativeApp);
    ~AppContext();

    NativeApp* getNativeApp();

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
    AppContext(AppContext const &);
    AppContext& operator=(AppContext const &);

    const char* eglStatusStr() const;

    NativeApp* mNativeApp;
    bool mRequestQuit;
    bool mRequestRender;

    // egl
    EGLDisplay mDisplay;
    EGLContext mEglContext;
    EGLSurface mSurface;
    EGLint mWidth;
    EGLint mHeight;
};

} // namespace dzy

#endif
