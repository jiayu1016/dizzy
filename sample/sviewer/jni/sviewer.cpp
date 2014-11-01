#include <memory>
#include <string>
#include "log.h"
#include "native_core.h"
#include "engine_context.h"
#include "render.h"
#include "scene_graph.h"
#include "scene.h"

using namespace dzy;
using namespace std;

class SViewer : public NativeCore {
public:
    virtual bool initActivity();
    virtual bool releaseActivity();
    virtual bool initView();
    virtual bool releaseView();
    virtual bool drawScene();

private:
    shared_ptr<Render>  mRender;
};

bool SViewer::initActivity() {
    bool ret = false;

    shared_ptr<EngineContext> engineContext(getEngineContext());
    if (!engineContext) return false;

    engineContext->listAssetFiles("");

    shared_ptr<Scene> scene(SceneManager::loadColladaAsset(engineContext, "4meshes.dae"));
    if (!scene) return false;

    SceneManager::get()->addScene(scene);
    SceneManager::get()->setCurrentScene(scene);

    return true;
}

bool SViewer::releaseActivity() {
    return true;
}

bool SViewer::initView() {
    shared_ptr<EngineContext> engineContext(getEngineContext());
    if (!engineContext) return false;

    mRender = engineContext->getDefaultRender();
    if (!mRender) return false;

    shared_ptr<Scene> scene(SceneManager::get()->getCurrentScene());
    if (!scene) return false;

    shared_ptr<Node> rootNode(scene->getRootNode());
    if (rootNode) {
        rootNode->setAutoProgram();
        rootNode->init();
    }
    return mRender->init();
}

bool SViewer::releaseView() {
    return mRender->release();
}

bool SViewer::drawScene() {
    shared_ptr<Scene> scene(SceneManager::get()->getCurrentScene());
    if (!scene) return false;

    return mRender->drawScene(scene);
}

NativeCore * dzyCreateNativeActivity() {
    return new SViewer;
}
