#include "native_app.h"

using namespace std;

namespace dzy {

NativeApp::NativeApp(struct android_app* app) :
    mApp(app) {
}

void NativeApp::registerCallback(AppContext* appContext, 
    void (*cmd)(struct android_app*, int32_t),
    int32_t (*input)(struct android_app*, AInputEvent*)){
    mApp->userData = appContext;
    mApp->onAppCmd = cmd;
    mApp->onInputEvent = input;
}

} // namespace

