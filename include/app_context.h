#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <memory>

namespace dzy {

class NativeApp;
class AppContext {
public:
    AppContext();
    ~AppContext();

    bool update();
    static void handleAppCmd(struct android_app* app, int32_t cmd);    
    static int32_t handleInputEvent(struct android_app* app, AInputEvent* event);
    void requestQuit();
    bool needQuit();
    void requestRender();
    void stopRender();
    bool needRender();

private:
    AppContext(AppContext const &);
    AppContext& operator=(AppContext const &);

    void appCmd(int32_t cmd);
    int32_t inputEvent(AInputEvent* event);
    void inputKeyEvent(int action, int code);
    void inputMotionEvent(int action);

    //std::shared_ptr<NativeApp> mNativeApp;
    NativeApp *mNativeApp;

    bool mRequestQuit;
    bool mRequestRender;
};

} // namespace dzy

#endif
