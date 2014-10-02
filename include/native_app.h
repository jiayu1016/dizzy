#ifndef NATIVE_APP_H
#define NATIVE_APP_H

#include <memory>
#include <android_native_app_glue.h>

namespace dzy {

class AppContext;
class NativeApp {
public:
    NativeApp(struct android_app* app);

    void registerCallback(AppContext* appContext, 
        void (*cmd)(struct android_app*, int32_t),
        int32_t (*input)(struct android_app*, AInputEvent*));

    virtual bool initApp() { return true; };
    virtual bool releaseApp() { return true; };
    virtual bool initView() { return true; };
    virtual bool releaseView() { return true; };
    virtual bool drawScene() { return true; };

    friend class AppContext;
private:
    struct android_app* mApp;
};

} // namespace dzy

#endif
