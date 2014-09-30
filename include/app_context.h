#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

namespace dzy {

class NativeApp;
class AppContext {
public:
    AppContext();
    ~AppContext();
    static void handleAppCmd(struct android_app* app, int32_t cmd);    
    static int32_t handleInputEvent(struct android_app* app, AInputEvent* event);

private:
    AppContext(AppContext const &);
    AppContext& operator=(AppContext const &);


    NativeApp *mNativeApp;
};

} // namespace dzy

#endif
