#ifndef NATIVE_APP_H
#define NATIVE_APP_H

#include <memory>
#include <android_native_app_glue.h>

namespace dzy {

class AppContext;
class NativeApp {
public:
    explicit NativeApp(struct android_app* app);
    ~NativeApp();

    bool init();
    void fini();
    void mainLoop();

    // event processing
    virtual void appCmd(int32_t cmd);
    virtual int32_t inputKeyEvent(int action, int code);
    virtual int32_t inputMotionEvent(int action);

    // main interface for derived class
    virtual bool initApp() = 0;
    virtual bool releaseApp() = 0;
    virtual bool initView() = 0;
    virtual bool releaseView() = 0;
    virtual bool drawScene() = 0;

    std::shared_ptr<AppContext> getAppContext();

    friend class AppContext;
private:
    NativeApp(NativeApp const &);
    NativeApp & operator=(NativeApp const &);

    /*
     * return 0, the framework will continue to handle the event
     * return 1, the framework will stop to handle the event
     */
    static void handleAppCmd(struct android_app* app, int32_t cmd);
    static int32_t handleInputEvent(struct android_app* app, AInputEvent* event);
    int32_t inputEvent(AInputEvent* event);

    std::shared_ptr<AppContext> mAppContext;
    struct android_app * mApp;
};

} // namespace dzy

#endif
