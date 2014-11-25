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
    virtual bool initView();
    virtual bool releaseView();
    virtual shared_ptr<Scene> getScene();
private:
    shared_ptr<Scene> mScene;
};

bool SViewer::initView() {
    shared_ptr<EngineContext> engineContext(getEngineContext());
    engineContext->listAssetFiles("");

    mScene = Scene::loadColladaAsset(engineContext, "4meshes.dae");
    if (!mScene) {
        ALOGE("failed to load collada scene");
        return false;
    }

    shared_ptr<Node> rootNode(mScene->getRootNode());
    if (!rootNode) return false;

    shared_ptr<Node> myNode(new Node("mynode"));

    shared_ptr<NodeObj> sphereNode(rootNode->getChild("Sphere"));
    if (sphereNode) {
        sphereNode->resetTransform();
        sphereNode->translate(3.5f, 2.5f, 0.f);
        myNode->attachChild(sphereNode);
    }

    shared_ptr<NodeObj> torusGeometry(rootNode->getChild("Geometry-Torus"));
    if (torusGeometry) {
        torusGeometry->resetTransform();
        torusGeometry->translate(0.f, 0.f, 3.f);
        myNode->attachChild(torusGeometry);
    }

    shared_ptr<CubeMesh> cube(new CubeMesh("mycube"));
    shared_ptr<Geometry> cubeGeo(new Geometry(cube));
    cubeGeo->rotate(-0.25f * 3.1415927, 0.f, 1.f, 0.f);
    cubeGeo->translate(3.f, 0.f, 0.f);
    if (torusGeometry) {
        shared_ptr<Material> material = torusGeometry->getMaterial();
        cubeGeo->setMaterial(material);
    }
    myNode->attachChild(cubeGeo);

    rootNode->attachChild(myNode);

    rootNode->dumpHierarchy();
    return true;
}

bool SViewer::releaseView() {
    return true;
}

shared_ptr<Scene> SViewer::getScene() {
    return mScene;
}

EngineCore * engine_main() {
    return new SViewer;
}
