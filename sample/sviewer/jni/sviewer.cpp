#include <memory>
#include <string>
#include "log.h"
#include "engine_core.h"
#include "engine_context.h"
#include "render.h"
#include "scene_graph.h"
#include "mesh.h"
#include "scene.h"

using namespace dzy;
using namespace std;

class SViewer : public EngineCore {
public:
    virtual bool create();
    virtual bool start();
    virtual bool update(long interval);
    virtual shared_ptr<Scene> getScene();
private:
    shared_ptr<Scene>   mScene;
    string              mSceneFileName;
};

bool SViewer::create() {
    mSceneFileName = getIntentString("modelName");
    return true;
}

bool SViewer::start() {
    shared_ptr<EngineContext> engineContext(getEngineContext());

    mScene = Scene::loadColladaAsset(engineContext, mSceneFileName);
    if (!mScene) return false;

    shared_ptr<Node> rootNode(mScene->getRootNode());
    rootNode->dumpHierarchy();
    return true;
}

bool SViewer::update(long interval) {
    return true;
}

shared_ptr<Scene> SViewer::getScene() {
    return mScene;
}

EngineCore * engine_main() {
    return new SViewer;
}
