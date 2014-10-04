#include <android/asset_manager.h>
#include <fstream>
#include <assert.h>
#include "log.h"
#include "app_context.h"
#include "scene.h"

using namespace std;

namespace dzy {

Scene::Scene() :
    mSceneData(NULL),
    mSceneSize(0) {
}

Scene::~Scene() {
}

FlatScene::FlatScene() {
}

FlatScene::~FlatScene() {
}

bool FlatScene::loadAsset(shared_ptr<AppContext> appContext,
    const string assetFile) {    
    bool ret = false;
    AAssetManager *assetManager = appContext->getAssetManager();
    assert(assetManager != NULL);

    AAsset* asset = AAssetManager_open(assetManager,
        assetFile.c_str(), AASSET_MODE_BUFFER);

    if (!asset) {
        ALOGE("Failed to open asset: %s", assetFile.c_str());
        return ret;
    }

    off_t length = AAsset_getLength(asset);
    char * buffer = new char[length];
    size_t sz = AAsset_read(asset, buffer, length);
    AAsset_close(asset);
    if (sz == length) ret = true;
    else {
        ALOGE("Partial read %d bytes", sz);
        delete[] buffer;
        return ret;
    }

    // process the buffer
    buffer[length - 1] = 0;
    ALOGD("%s: %s", assetFile.c_str(), buffer);
    delete[] buffer;

    return ret;
}

bool FlatScene::load(shared_ptr<AppContext> appContext,
    const string file) {
    bool ret = false;
    ifstream ifs(file.c_str(), ifstream::binary);
    if (!ifs) {
        ALOGE("ifs ctor failed for %s", file.c_str());
        return ret;
    }

    ifs.seekg(0, ifs.end);
    int length = ifs.tellg();
    ifs.seekg(0, ifs.beg);
    char * buffer = new char[length];
    ifs.read(buffer, length);
    ifs.close();
    if (ifs) ret = true;
    else {
        ALOGE("Partial read %d bytes", ifs.gcount());
        delete[] buffer;
        return ret;
    }

    // process the buffer
    buffer[length - 1] = 0;
    ALOGD("%s: %s", file.c_str(), buffer);
    delete[] buffer;

    return ret;
}

bool FlatScene::listAssetFiles(shared_ptr<AppContext> appContext) {
    bool ret = false;
    AAssetManager *assetManager = appContext->getAssetManager();
    assert(assetManager != NULL);

    AAssetDir* assetDir = AAssetManager_openDir(assetManager, "");

    if (!assetDir) {
        ALOGE("Failed to open asset root dir");
        return ret;
    }

    const char * assetFile = NULL;
    while (assetFile = AAssetDir_getNextFileName(assetDir)) {
        ALOGD("%s", assetFile);
    }

    AAssetDir_close(assetDir);

    ret = true;
    return ret;
}

SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
}

shared_ptr<Scene> SceneManager::createScene(SceneType sceneType) {
    shared_ptr<Scene> scene(NULL);
    if (sceneType == SCENE_TYPE_FLAT)
        scene.reset(new FlatScene);
    return scene;
}

} // namespace dzy
