#include <memory>
#include <string>
#include "log.h"
#include "native_app.h"
#include "app_context.h"
#include "render.h"
#include "scene.h"

using namespace dzy;
using namespace std;

class SViewApp : public NativeApp {
public:
    virtual bool initApp();
    virtual bool releaseApp();
    virtual bool initView();
    virtual bool releaseView();
    virtual bool drawScene();

private:
    shared_ptr<Render>  mRender;
};

bool SViewApp::initApp() {
    bool ret = false;

    shared_ptr<AppContext> appContext(getAppContext());
    if (!appContext) return false;

    appContext->listAssetFiles("");

    shared_ptr<Scene> scene(SceneManager::loadColladaAsset(appContext, "4meshes.dae"));
    if (!scene) return false;

    SceneManager::get()->addScene(scene);
    SceneManager::get()->setCurrentScene(scene);

    return true;
}

bool SViewApp::releaseApp() {
    return true;
}

bool SViewApp::initView() {
    shared_ptr<AppContext> appContext(getAppContext());
    if (!appContext) return false;

    mRender = appContext->getDefaultRender();
    if (!mRender) return false;

    shared_ptr<Scene> scene(SceneManager::get()->getCurrentScene());
    if (!scene) return false;

    return mRender->init(scene);
}

bool SViewApp::releaseView() {
    return mRender->release();
}

bool SViewApp::drawScene() {
    shared_ptr<Scene> scene(SceneManager::get()->getCurrentScene());
    if (!scene) return false;

    return mRender->drawScene(scene);
}

NativeApp * dzyCreateNativeActivity() {
    return new SViewApp;
}
