#include <memory>
#include <string>
#include "log.h"
#include "native_app.h"
#include "scene.h"

using namespace dzy;
using namespace std;

class SViewApp : public NativeApp {
public:
    SViewApp() { ALOGD("SViewApp::SViewApp()"); };
    virtual ~SViewApp() { ALOGD("SViewApp::~SViewApp()"); };

    virtual bool initApp();
    virtual bool releaseApp();
    virtual bool initView();
    virtual bool releaseView();
    virtual bool drawScene();
};

bool SViewApp::initApp() {
    ALOGD("SViewapp::initApp()");
    bool ret = false;

    shared_ptr<AppContext> appContext(getAppContext());
    shared_ptr<Scene> currentScene(getCurrentScene());
    currentScene->listAssetFiles(appContext, "");
    currentScene->listAssetFiles(appContext, "mesh");

    ret = currentScene->loadAsset(appContext, "cube_triangulate.dae");
    if (!ret) return ret;

    ret = currentScene->loadAsset(appContext, "Cinema4D.dae");
    if (!ret) return ret;

    ret = currentScene->loadAsset(appContext, "ConcavePolygon.dae");
    if (!ret) return ret;

    ret = currentScene->loadAsset(appContext, "mesh/duck.dae");
    if (!ret) return ret;

    ret = currentScene->load(appContext, "/sdcard/dzy/data.anim");
    if (!ret) return ret;

    return ret;
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
    shared_ptr<dzy::NativeApp> nativeApp(new SViewApp);
    shared_ptr<dzy::Scene> scene(
        SceneManager::createScene(SceneManager::SCENE_TYPE_FLAT));
    if (!nativeApp->init(app, scene))
        return;
    nativeApp->mainLoop();
    nativeApp->fini();
}
