#include <memory>
#include <string>
#include "log.h"
#include "native_app.h"
#include "render.h"
#include "scene.h"

using namespace dzy;
using namespace std;

class SViewApp : public NativeApp {
public:
    SViewApp() { ALOGD("SViewApp::SViewApp()"); };
    virtual ~SViewApp() { ALOGD("SViewApp::~SViewApp()"); };

    virtual bool initApp();
    virtual bool releaseApp();
    virtual bool initView(std::shared_ptr<Scene> scene);
    virtual bool releaseView();
    virtual bool drawScene(std::shared_ptr<Scene> scene);
};

bool SViewApp::initApp() {
    ALOGD("SViewapp::initApp()");
    bool ret = false;

    shared_ptr<AppContext> appContext(getAppContext());
    shared_ptr<Scene> currentScene(SceneManager::get()->getCurrentScene());
    currentScene->listAssetFiles(appContext, "");
    currentScene->listAssetFiles(appContext, "mesh");

    ret = currentScene->loadColladaAsset(appContext, "cube_triangulate.dae");
    if (!ret) return ret;
/*
    ret = currentScene->loadColladaAsset(appContext, "Cinema4D.dae");
    if (!ret) return ret;

    ret = currentScene->loadColladaAsset(appContext, "ConcavePolygon.dae");
    if (!ret) return ret;

    ret = currentScene->loadColladaAsset(appContext, "astroboy_walk.dae");
    if (!ret) return ret;

    ret = currentScene->loadColladaAsset(appContext, "mesh/duck.dae");
    if (!ret) return ret;

    ret = currentScene->load(appContext, "/sdcard/dzy/data.anim");
    if (!ret) return ret;
*/
    return ret;
}

bool SViewApp::releaseApp() {
    ALOGD("SViewapp::releaseApp()");
    return true;
}

bool SViewApp::initView(shared_ptr<Scene> scene) {
    ALOGD("SViewapp::initView()");
    return getCurrentRender()->init(scene);
}

bool SViewApp::releaseView() {
    ALOGD("SViewapp::releaseView()");
    return getCurrentRender()->release();
}

bool SViewApp::drawScene(shared_ptr<Scene> scene) {
    shared_ptr<Render> render(getCurrentRender());

    return render->drawScene(scene);
}

void android_main(struct android_app* app) {
    shared_ptr<dzy::NativeApp> nativeApp(new SViewApp);
    shared_ptr<dzy::Scene> scene(
        SceneManager::get()->createScene(SceneManager::SCENE_TYPE_FLAT));
    if (!nativeApp->init(app))
        return;
    nativeApp->mainLoop();
    nativeApp->fini();
}
