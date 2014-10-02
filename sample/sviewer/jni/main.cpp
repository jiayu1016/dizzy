#include <memory>
#include "log.h"
#include "native_app.h"

using namespace dzy;
using namespace std;

class SViewApp : public NativeApp {
public:
    virtual bool initApp();
    virtual bool releaseApp();
    virtual bool initView();
    virtual bool releaseView();
    virtual bool drawScene();
};

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

namespace dzy {

shared_ptr<NativeApp> nativeInit() {
    shared_ptr<NativeApp> na(new SViewApp);
    return na;
}

} // namespace dzy
