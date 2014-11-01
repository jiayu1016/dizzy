#include <memory>
#include <string>
#include "log.h"
#include "native_core.h"
#include "engine_context.h"
#include "render.h"
#include "scene_graph.h"
#include "mesh.h"
#include "scene.h"

using namespace dzy;
using namespace std;

class SViewer : public NativeCore {
public:
    virtual bool initView();
    virtual bool releaseView();
    virtual bool drawScene();

private:
    shared_ptr<Render>  mRender;
};

bool SViewer::initView() {
    shared_ptr<EngineContext> engineContext(getEngineContext());
    if (!engineContext) return false;

    engineContext->listAssetFiles("");

    shared_ptr<Scene> scene(SceneManager::loadColladaAsset(engineContext, "4meshes.dae"));
    if (!scene) {
        ALOGE("failed to load collada scene");
        return false;
    }

    SceneManager::get()->addScene(scene);
    SceneManager::get()->setCurrentScene(scene);

    mRender = engineContext->getDefaultRender();
    if (!mRender) return false;

    shared_ptr<Node> rootNode(scene->getRootNode());
    if (!rootNode) return false;

    shared_ptr<CubeMesh> cube(new CubeMesh());
    shared_ptr<GeoNode> cubeNode(new GeoNode(cube));
    rootNode->attachChild(cubeNode);

    rootNode->setAutoProgram();
    rootNode->init();
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
