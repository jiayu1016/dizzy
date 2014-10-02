#ifndef NATIVE_APP_H
#define NATIVE_APP_H

#include <memory>
#include <android_native_app_glue.h>

namespace dzy {

class AppContext;
class NativeApp {
public:
    NativeApp(struct android_app* app);
    ~NativeApp();

    bool init();
    void fini();

    std::shared_ptr<AppContext> getAppContext();
    void mainLoop();
    static void handleAppCmd(struct android_app* app, int32_t cmd);    

    /*
     * return 0, the framework will continue to handle the event
     * return 1, the framework will stop to handle the event
     */
    static int32_t handleInputEvent(struct android_app* app, AInputEvent* event);


    // event processing
    void appCmd(int32_t cmd);
    int32_t inputEvent(AInputEvent* event);
    int32_t inputKeyEvent(int action, int code);
    int32_t inputMotionEvent(int action);

    // subclass interface
    virtual bool initApp();
    virtual bool releaseApp();
    virtual bool initView();
    virtual bool releaseView();
    virtual bool drawScene();

    friend class AppContext;
private:
    std::shared_ptr<AppContext> mAppContext;
    struct android_app * mApp;
};

} // namespace dzy

#endif
