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
        //cubeGeo->setMaterial(material);
    }
    myNode->attachChild(cubeGeo);

    rootNode->attachChild(myNode);

    rootNode->dumpHierarchy();

    rootNode->initGpuData();

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
