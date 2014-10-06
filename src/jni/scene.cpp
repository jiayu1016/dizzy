#include <android/asset_manager.h>
#include <fstream>
#include <assert.h>
#include <math.h>
#include "app_context.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "log.h"
#include "assimp_adapter.h"
#include "utils.h"
#include "scene.h"

using namespace std;

namespace dzy {

Camera::Camera()
    : mUp               (0.f,1.f,0.f)
    , mLookAt           (0.f,0.f,1.f)
    , mHorizontalFOV    (0.25f * (float)M_PI)
    , mClipPlaneNear    (0.1f)
    , mClipPlaneFar     (1000.f)
    , mAspect           (0.f) {

}

Camera::Camera(
    ndk_helper::Vec3    position,
    ndk_helper::Vec3    up,
    ndk_helper::Vec3    lookAt,
    float               horizontalFOV,
    float               clipPlaneNear,
    float               clipPlaneFar,
    float               aspect)
    : mPosition         (position)
    , mUp               (up)
    , mLookAt           (lookAt)
    , mHorizontalFOV    (horizontalFOV)
    , mClipPlaneNear    (clipPlaneNear)
    , mClipPlaneFar     (clipPlaneFar)
    , mAspect           (aspect) {

}

Light::Light()
    : mType                 (LIGHT_SOURCE_UNDEFINED)
    , mAttenuationConstant  (0.f)
    , mAttenuationLinear    (1.f)
    , mAttenuationQuadratic (0.f)
    , mAngleInnerCone       ((float)M_PI)
    , mAngleOuterCone       ((float)M_PI) {
}

Light::Light(
    LightSourceType type,
    float attenuationConstant,
    float attenuationLinear,
    float attenuationQuadratic,
    float angleInnerCone,
    float angleOuterCone)
    : mType                 (type)
    , mAttenuationConstant  (attenuationConstant)
    , mAttenuationLinear    (attenuationLinear)
    , mAttenuationQuadratic (attenuationQuadratic)
    , mAngleInnerCone       (angleInnerCone)
    , mAngleOuterCone       (angleOuterCone) {
} 

ndk_helper::Mat4 getMatrix() {
    return ndk_helper::Mat4();
};

Scene::Scene()
    : mSceneData(NULL)
    , mSceneSize(0) {
}

Scene::~Scene() {
}

FlatScene::FlatScene()
    : mFlags(0)
    , mNumNode(0) {
}

FlatScene::~FlatScene() {
}

bool FlatScene::loadColladaAsset(shared_ptr<AppContext> appContext,
    const string &assetFile) {    
    AAssetManager *assetManager = appContext->getAssetManager();
    assert(assetManager != NULL);

    AAsset* asset = AAssetManager_open(assetManager,
        assetFile.c_str(), AASSET_MODE_BUFFER);

    if (!asset) {
        ALOGE("Failed to open asset: %s", assetFile.c_str());
        return false;
    }

    off_t length = AAsset_getLength(asset);
    unique_ptr<char, ArrayDeleter<char> > buffer(
        new char[length], ArrayDeleter<char>());
    size_t sz = AAsset_read(asset, buffer.get(), length);
    AAsset_close(asset);
    if (sz != length) {
        ALOGE("Partial read %d bytes", sz);
        return false;
    }

    Assimp::Importer importer;
    const aiScene *scene(importer.ReadFileFromMemory(
        buffer.get(), length,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType));
    if (!scene) {
        ALOGE("assimp failed to load %s: %s", assetFile.c_str(),
            importer.GetErrorString());
        return false;
    }

    // parse aiScene to FlatScene
    if (scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
        ALOGE("%s: incomplete scene data loaded", assetFile.c_str());
        return false;
    }
    if (scene->mRootNode == NULL) {
        ALOGE("%s: root node null", assetFile.c_str());
        return false;
    }
    ALOGD("%s: %u meshes found",     assetFile.c_str(), scene->mNumMeshes);
    ALOGD("%s: %u materials found",  assetFile.c_str(), scene->mNumMaterials);
    ALOGD("%s: %u animations found", assetFile.c_str(), scene->mNumAnimations);
    ALOGD("%s: %u textures found",   assetFile.c_str(), scene->mNumTextures);
    ALOGD("%s: %u lights found",     assetFile.c_str(), scene->mNumLights);
    ALOGD("%s: %u cameras found",    assetFile.c_str(), scene->mNumCameras);

    for (int i=0; i<scene->mNumCameras; i++) {
        shared_ptr<Camera> camera(assimpTypeCast(scene->mCameras[i]));
        mCameras.push_back(camera);
    }
    for (int i=0; i<scene->mNumLights; i++) {
        shared_ptr<Light> light(assimpTypeCast(scene->mLights[i]));
        mLights.push_back(light);
    }
    for (int i=0; i<scene->mNumTextures; i++) {
        shared_ptr<Texture> texture(assimpTypeCast(scene->mTextures[i]));
        mTextures.push_back(texture);
    }
    for (int i=0; i<scene->mNumAnimations; i++) {
        shared_ptr<Animation> animation(assimpTypeCast(scene->mAnimations[i]));
        mAnimations.push_back(animation);
    }
    for (int i=0; i<scene->mNumMaterials; i++) {
        shared_ptr<Material> material(assimpTypeCast(scene->mMaterials[i]));
        mMaterials.push_back(material);
    }
    for (int i=0; i<scene->mNumMeshes; i++) {
        shared_ptr<Mesh> mesh(assimpTypeCast(scene->mMeshes[i]));
        mMeshes.push_back(mesh);
    }

    

    return true;
}

bool FlatScene::load(shared_ptr<AppContext> appContext,
    const string &file) {
    ifstream ifs(file.c_str(), ifstream::binary);
    if (!ifs) {
        ALOGE("ifstream ctor failed for %s", file.c_str());
        return false;
    }

    ifs.seekg(0, ifs.end);
    int length = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    unique_ptr<char, ArrayDeleter<char> > buffer(
        new char[length], ArrayDeleter<char>());
    ifs.read(buffer.get(), length);
    ifs.close();
    if (!ifs) {
        ALOGE("Partial read %d bytes", ifs.gcount());
        return false;
    }

    // process the buffer
    buffer.get()[length - 1] = 0;
    ALOGD("%s: %s", file.c_str(), buffer.get());

    return true;
}

bool FlatScene::listAssetFiles(shared_ptr<AppContext> appContext,
    const string &dir) {
    AAssetManager *assetManager = appContext->getAssetManager();
    assert(assetManager != NULL);

    AAssetDir* assetDir = AAssetManager_openDir(assetManager, dir.c_str());
    if (!assetDir) {
        ALOGE("Failed to open asset root dir");
        return false;
    }

    const char * assetFile = NULL;
    while (assetFile = AAssetDir_getNextFileName(assetDir)) {
        ALOGD("%s", assetFile);
    }

    AAssetDir_close(assetDir);

    return true;
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
