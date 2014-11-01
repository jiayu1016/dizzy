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
#include "scene.h"

using namespace std;

namespace dzy {

Camera::Camera()
    : mPosition         (0.f, 0.f, 0.f)
    , mUp               (0.f, 1.f, 0.f)
    , mLookAt           (0.f, 0.f, -1.f)
    , mHorizontalFOV    (0.25f * (float)M_PI)
    , mClipPlaneNear    (0.1f)
    , mClipPlaneFar     (1000.f)
    , mAspect           (0.f) {
}

Camera::Camera(
    glm::vec3           position,
    glm::vec3           up,
    glm::vec3           lookAt,
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

void Camera::setAspect(float aspect) {
    mAspect = aspect;
}

#if 0
glm::mat4 Camera::getCameraMatrix() {
    glm::vec3 zaxis = mLookAt;
    zaxis = glm::normalize(zaxis);
    glm::vec3 yaxis = mUp;
    yaxis = glm::normalize(yaxis);
    glm::vec3 xaxis = glm::cross(yaxis, zaxis);

    return glm::lookAt(...);
}
#endif

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(mPosition, mLookAt, mUp);
}

glm::mat4 Camera::getViewMatrix(glm::mat4 modelTransfrom) {
    //Utils::dump("camera model transform", modelTransfrom);
    glm::vec4 newPos = modelTransfrom * glm::vec4(mPosition, 1.0f);
    glm::vec4 newLook = modelTransfrom * glm::vec4(mLookAt, 1.0f);
    //glm::vec4 newUp = modelTransfrom * glm::vec4(mUp, 1.0f);

    glm::vec3 pos = glm::vec3(newPos.x, newPos.y, newPos.z);
    glm::vec3 look = glm::vec3(newLook.x, newLook.y, newLook.z);
    //glm::vec3 up = glm::vec3(newUp.x, newUp.y, newUp.z);

    // up vector doesn't change
    return glm::lookAt(pos, look, mUp);
}

glm::mat4 Camera::getProjMatrix() {
    return glm::perspective(mHorizontalFOV, mAspect, mClipPlaneNear, mClipPlaneFar);
}

void Camera::dump(glm::vec3 pos, glm::vec3 at, glm::vec3 up) {
    PRINT("position: (%+08.6f, %+08.6f, %+08.6f)",
        pos.x, pos.y, pos.z);
    PRINT("at: (%+08.6f, %+08.6f, %+08.6f)",
        at.x, at.y, at.z);
    PRINT("up: (%+08.6f, %+08.6f, %+08.6f)",
        up.x, up.y, up.z);
}

void Camera::dumpParameter() {
    PRINT("*********** Camera Parameters ***********");
    dump(mPosition, mLookAt, mUp);
    PRINT("(fov, aspect, near, far): (%+08.6f, %+08.6f, %+08.6f, %+08.6f)",
        mHorizontalFOV, mAspect, mClipPlaneNear, mClipPlaneFar);
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

void Light::dumpParameter() {
    ALOGD("light: %s\n"
        "position: (%f, %f, %f), direction: (%f, %f, %f)\n"
        "diffuse: (%f, %f, %f), specular: (%f, %f, %f), ambient: (%f, %f, %f)\n"
        "mAngleInnerCone: %f , mAngleOuterCone: %f\n"
        "mAttenuationConstant: %f, mAttenuationLinear: %f, mAttenuationQuadratic: %f",
        mName.c_str(),
        mPosition.x, mPosition.y, mPosition.z,
        mDirection.x, mDirection.y, mDirection.z,
        mColorDiffuse.x, mColorDiffuse.y, mColorDiffuse.z,
        mColorSpecular.x, mColorSpecular.y, mColorSpecular.z,
        mColorAmbient.x, mColorAmbient.y, mColorAmbient.z,
        mAngleInnerCone, mAngleOuterCone,
        mAttenuationConstant, mAttenuationLinear, mAttenuationQuadratic);
}

Scene::Scene() {
}

shared_ptr<Camera> Scene::getActiveCamera() {
    // TODO: support muliple cameras in a scene
    if (getNumCameras() > 0)
        return mCameras[0];
    return NULL;
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

SceneManager::SceneManager() {
}

void SceneManager::addScene(shared_ptr<Scene> scene) {
    mScenes.push_back(scene);
}

shared_ptr<Scene> SceneManager::getCurrentScene() {
    return mCurrentScene;
}

void SceneManager::setCurrentScene(shared_ptr<Scene> scene) {
    auto it = find(mScenes.begin(), mScenes.end(), scene);
    if (it != mScenes.end())
        mCurrentScene = *it;
    else {
        mScenes.push_back(scene);
        mCurrentScene = scene;
    }
}

shared_ptr<Scene> SceneManager::loadFile(shared_ptr<EngineContext> engineContext,
    const string &file) {
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
    ALOGD("%s: %s", file.c_str(), buffer.get());

    shared_ptr<Scene> s(new Scene);

    // TODO:

    return s;
}

shared_ptr<Scene> SceneManager::loadColladaAsset(shared_ptr<EngineContext> engineContext,
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

    MeasureDuration importDuration;
    Assimp::Importer importer;
    const aiScene *scene(importer.ReadFileFromMemory(
        buffer.get(), length,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType));
    if (!scene) {
        ALOGE("assimp failed to load %s: %s", assetFile.c_str(),
            importer.GetErrorString());
        return nullptr;
    }

    if (scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
        ALOGE("%s: incomplete scene data loaded", assetFile.c_str());
        return nullptr;
    }
    if (scene->mRootNode == NULL) {
        ALOGE("%s: root node null", assetFile.c_str());
        return nullptr;
    }

    shared_ptr<Scene> s(new Scene);
    long long importTime = importDuration.getMilliSeconds();

    ALOGD("%s: %u meshes found",     assetFile.c_str(), scene->mNumMeshes);
    ALOGD("%s: %u materials found",  assetFile.c_str(), scene->mNumMaterials);
    ALOGD("%s: %u animations found", assetFile.c_str(), scene->mNumAnimations);
    ALOGD("%s: %u textures found",   assetFile.c_str(), scene->mNumTextures);
    ALOGD("%s: %u lights found",     assetFile.c_str(), scene->mNumLights);
    ALOGD("%s: %u cameras found",    assetFile.c_str(), scene->mNumCameras);

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

    long long cvtTime = cvtDuration.getMicroSeconds();

    ALOGD("assimp load: %lld ms, conversion: %lld us", importTime, cvtTime);

    return s;
}

} // namespace dzy
