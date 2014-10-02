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
    AppContext(std::shared_ptr<NativeApp> nativeApp);
    ~AppContext();

    void mainLoop();

    // gfx system
    bool initDisplay();
    void releaseDisplay();

    bool update();
    static void handleAppCmd(struct android_app* app, int32_t cmd);    

    /*
     * return 0, the framework will continue to handle the event
     * return 1, the framework will stop to handle the event
     */
    static int32_t handleInputEvent(struct android_app* app, AInputEvent* event);

    void requestQuit();
    bool needQuit();
    void requestRender();
    void stopRender();
    bool needRender();

private:
    AppContext(AppContext const &);
    AppContext& operator=(AppContext const &);

    // event processing
    void appCmd(int32_t cmd);
    int32_t inputEvent(AInputEvent* event);
    int32_t inputKeyEvent(int action, int code);
    int32_t inputMotionEvent(int action);

    const char* eglStatusStr() const;

    std::shared_ptr<NativeApp> mNativeApp;
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
