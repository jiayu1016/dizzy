#include <memory>
#include "log.h"
#include "native_app.h"

using namespace dzy;
using namespace std;

class SViewApp : public NativeApp {
public:
    SViewApp(struct android_app *app);

    virtual bool initApp();
    virtual bool releaseApp();
    virtual bool initView();
    virtual bool releaseView();
    virtual bool drawScene();
};

SViewApp::SViewApp(struct android_app *app) :
    NativeApp(app) {
}

bool SViewApp::initApp() {
    ALOGD("SViewapp::initApp()");
    return true;
}

bool SViewApp::releaseApp() {
    ALOGD("SViewapp::releaseApp()");
    return true;
}

bool SViewApp::initView() {
    ALOGD("SViewapp::initView()");
    return true;
}

bool SViewApp::releaseView() {
    ALOGD("SViewapp::releaseView()");
    return true;
}

bool SViewApp::drawScene() {
    ALOGD("SViewapp::drawScene()");
    return true;
}

void android_main(struct android_app* app) {
    shared_ptr<dzy::NativeApp> nativeApp(new SViewApp(app));
    nativeApp->mainLoop();
}
