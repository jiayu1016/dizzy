#include <algorithm>
#include <android/asset_manager.h>
#include <fstream>
#include <assert.h>
#include <math.h>
#include "engine_context.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "log.h"
#include "assimp_adapter.h"
#include "utils.h"
#include "render.h"
#include "program.h"
#include "scene_graph.h"
#include "mesh.h"
#include "material.h"
#include "camera.h"
#include "scene.h"

using namespace std;

namespace dzy {

Scene::Scene()
    : mRootNode(new Node("root"))
    , mActiveCamera(-1) {
}

Scene::~Scene() {
    ALOGV("Scene::~Scene()");
}

shared_ptr<Camera> Scene::getActiveCamera() {
    // TODO: support muliple cameras in a scene
    if (mActiveCamera == -1) {
        if (getNumCameras() > 0) mActiveCamera = 0;
    }
    if (mActiveCamera == -1) {
        shared_ptr<Camera> camera(new Camera("default_camera"));
        camera->setPostion(DEFAULT_CAMERA_POS);
        camera->setLookAt(DEFAULT_CAMERA_CENTER);
        camera->setUp(DEFAULT_CAMERA_UP);
        mCameras.push_back(camera);
        mActiveCamera = 0;
    }
    return mCameras[mActiveCamera];
}

shared_ptr<Camera> Scene::getCamera(int idx) {
    if (idx < 0) return nullptr;
    if (idx < mCameras.size())
        return mCameras[idx];
    return nullptr;
}

shared_ptr<Light> Scene::getLight(int idx) {
    if (idx < 0) return nullptr;
    if (idx < mLights.size())
        return mLights[idx];
    return nullptr;
}

bool Scene::atLeastOneMeshHasVertexPosition() {
    for (size_t i=0; i<getNumMeshes(); i++) {
        if (mMeshes[i]->hasVertexPositions()) return true;
    }
    return false;
}

bool Scene::atLeastOneMeshHasVertexColor() {
    for (size_t i=0; i<getNumMeshes(); i++) {
        if (mMeshes[i]->hasVertexColors()) return true;
    }
    return false;
}

bool Scene::atLeastOneMeshHasNormal() {
    for (size_t i=0; i<getNumMeshes(); i++) {
        if (mMeshes[i]->hasVertexNormals()) return true;
    }
    return false;
}

shared_ptr<Scene> Scene::loadColladaFromFile(const string &file) {
    ifstream ifs(file.c_str(), ifstream::binary);
    if (!ifs) {
        ALOGE("ifstream ctor failed for %s", file.c_str());
        return nullptr;
    }

    ifs.seekg(0, ifs.end);
    int length = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    unique_ptr<char[]> buffer(new char[length]);
    ifs.read(buffer.get(), length);
    ifs.close();
    if (!ifs) {
        ALOGE("Partial read %d bytes", ifs.gcount());
        return nullptr;
    }

    // process the buffer
    buffer.get()[length - 1] = 0;

    return loadColladaFromMemory(buffer.get(), length, file);
}

shared_ptr<Scene> Scene::loadColladaFromAsset(
    shared_ptr<EngineContext> engineContext,
    const string &assetFile) {
    AAssetManager *assetManager = engineContext->getAssetManager();
    assert(assetManager != NULL);

    AAsset* asset = AAssetManager_open(assetManager,
        assetFile.c_str(), AASSET_MODE_BUFFER);

    if (!asset) {
        ALOGE("Failed to open asset: %s", assetFile.c_str());
        return nullptr;
    }

    off_t length = AAsset_getLength(asset);
    unique_ptr<char[]> buffer(new char[length]);
    size_t sz = AAsset_read(asset, buffer.get(), length);
    AAsset_close(asset);
    if (sz != length) {
        ALOGE("Partial read %d bytes", sz);
        return nullptr;
    }

    return loadColladaFromMemory(buffer.get(), length, assetFile);
}

std::shared_ptr<Scene> Scene::loadColladaFromMemory(
    const char *buf, int length, const string& fileName) {
    MeasureDuration importDuration;
    Assimp::Importer importer;
    const aiScene *scene(importer.ReadFileFromMemory(
        buf, length,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType));
    if (!scene) {
        ALOGE("assimp failed to load %s: %s", fileName.c_str(),
            importer.GetErrorString());
        return nullptr;
    }

    if (scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
        ALOGE("%s: incomplete scene data loaded", fileName.c_str());
        return nullptr;
    }
    if (scene->mRootNode == NULL) {
        ALOGE("%s: root node null", fileName.c_str());
        return nullptr;
    }

    shared_ptr<Scene> s(new Scene);
    long long importTime = importDuration.getMilliSeconds();

    ALOGV("%s: %u meshes, %u materials, %u animations, %u textures, %u lights, %u cameras",
        fileName.c_str(), scene->mNumMeshes, scene->mNumMaterials, scene->mNumAnimations,
        scene->mNumTextures, scene->mNumLights, scene->mNumCameras);

    MeasureDuration cvtDuration;
    for (int i=0; i<scene->mNumCameras; i++) {
        shared_ptr<Camera> camera(AIAdapter::typeCast(scene->mCameras[i]));
        s->mCameras.push_back(camera);
    }
    for (int i=0; i<scene->mNumLights; i++) {
        shared_ptr<Light> light(AIAdapter::typeCast(scene->mLights[i]));
        s->mLights.push_back(light);
    }
    for (int i=0; i<scene->mNumTextures; i++) {
        shared_ptr<Texture> texture(AIAdapter::typeCast(scene->mTextures[i]));
        s->mTextures.push_back(texture);
    }
    for (int i=0; i<scene->mNumAnimations; i++) {
        shared_ptr<Animation> animation(AIAdapter::typeCast(scene->mAnimations[i]));
        s->mAnimations.push_back(animation);
    }
    for (int i=0; i<scene->mNumMaterials; i++) {
        shared_ptr<Material> material(AIAdapter::typeCast(scene->mMaterials[i]));
        s->mMaterials.push_back(material);
    }
    for (int i=0; i<scene->mNumMeshes; i++) {
        shared_ptr<Mesh> mesh(AIAdapter::typeCast(scene->mMeshes[i]));
        s->mMeshes.push_back(mesh);
    }

    AIAdapter::buildSceneGraph(s, scene->mRootNode);
    AIAdapter::postProcess(s);

    long long cvtTime = cvtDuration.getMicroSeconds();

    ALOGD("assimp load: %lld ms, conversion: %lld us", importTime, cvtTime);
    return s;
}

} // namespace dzy
