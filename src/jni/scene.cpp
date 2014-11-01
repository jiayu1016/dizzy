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
#include "scene.h"

using namespace std;

namespace dzy {

unsigned int MeshData::append(int size, unsigned char *buf) {
    // TODO: consider optimizing
    int bufSize = mBuffer.size();
    if (mBuffer.capacity() < bufSize + size) {
        ALOGD("insufficent MeshData capacity, enlarge, %d => %d",
            mBuffer.capacity(), bufSize + size);
        mBuffer.reserve(bufSize + size);
        if (mBuffer.capacity() < bufSize + size) {
            ALOGE("failed to enlarge MeshData storage, data unchanged");
            return -1;
        }
    }

    mBuffer.resize(bufSize + size);
    memcpy(&mBuffer[bufSize], buf, size);
    return bufSize;
}

void * MeshData::getBuf(int offset) {
    return static_cast<void *>(&mBuffer[offset]);
}

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

bool Material::get(MaterialType type, glm::vec3& color) {
    switch(type) {
    case COLOR_DIFFUSE:
        color = mDiffuse;
        break;
    case COLOR_SPECULAR:
        color = mSpecular;
        break;
    case COLOR_AMBIENT:
        color = mAmbient;
        break;
    case COLOR_EMISSION:
        color = mEmission;
        break;
    default:
        return false;
    }
    return true;
}

bool Material::get(MaterialType type, float& value) {
    switch(type) {
    case SHININESS:
        value = mShininess;
        break;
    default:
        return false;
    }
    return true;
}

Mesh::Mesh()
    : mPrimitiveType            (PRIMITIVE_TYPE_TRIANGLES)
    , mNumVertices              (0)
    , mNumFaces                 (0)
    , mMaterialIndex            (0)
    , mPosOffset                (0)
    , mPosNumComponents         (0)
    , mPosBytesComponent        (0)
    , mHasPos                   (false)
    , mNumColorChannels         (0)
    , mNumTextureCoordChannels  (0)
    , mNormalOffset             (0)
    , mNormalNumComponents      (0)
    , mNormalBytesComponent     (0)
    , mHasNormal                (false)
    , mTangentOffset            (0)
    , mTangentNumComponents     (0)
    , mTangentBytesComponent    (0)
    , mHasTangent               (false)
    , mBitangentOffset          (0)
    , mBitangentNumComponents   (0)
    , mBitangentBytesComponent  (0)
    , mHasBitangent             (false) {
    memset(&mColorOffset[0], 0, MAX_COLOR_SETS * sizeof(unsigned int));
    memset(&mColorNumComponents[0], 0, MAX_COLOR_SETS * sizeof(unsigned int));
    memset(&mColorBytesComponent[0], 0, MAX_COLOR_SETS * sizeof(unsigned int));
    memset(&mTextureCoordOffset[0], 0, MAX_TEXTURECOORDS * sizeof(unsigned int));
    memset(&mTextureCoordNumComponents[0], 0, MAX_TEXTURECOORDS * sizeof(unsigned int));
    memset(&mTextureCoordBytesComponent[0], 0, MAX_TEXTURECOORDS * sizeof(unsigned int));
}

bool Mesh::hasVertexPositions() const {
    return mHasPos;
}

bool Mesh::hasVertexColors(unsigned int channel) const {
    if( channel >= MAX_COLOR_SETS)
        return false;
    else
        // these values are 0 by default
        return  mColorNumComponents[channel] &&
                mColorBytesComponent[channel] &&
                mNumVertices > 0;
}

bool Mesh::hasVertexColors() const {
    return mNumColorChannels > 0;
}

bool Mesh::hasVertexTextureCoords(unsigned int channel) const {
    if( channel >= MAX_TEXTURECOORDS)
        return false;
    else
        // these values are 0 by default
        return  mTextureCoordNumComponents[channel] &&
                mTextureCoordNumComponents[channel] &&
                mNumVertices > 0;
}

bool Mesh::hasVertexTextureCoords() const {
    return mNumTextureCoordChannels > 0;
}

bool Mesh::hasVertexNormals() const {
    return mHasNormal;
}

bool Mesh::hasVertexTangentsAndBitangents() const {
    return mHasTangent && mHasBitangent;
}

bool Mesh::hasFaces() const {
    return !mTriangleFaces.empty() && mNumFaces > 0;
}

unsigned int Mesh::getNumVertices() const {
    return mNumVertices;
}

unsigned int Mesh::getNumColorChannels() const {
    return mNumColorChannels;
}

unsigned int Mesh::getNumTextureCoordChannels() const {
    return mNumTextureCoordChannels;
}

unsigned int Mesh::getNumFaces() const {
    return mNumFaces;
}

unsigned int Mesh::getNumIndices() const {
    return mNumFaces * 3;
}

unsigned int Mesh::getPositionNumComponent() const {
    return mPosNumComponents;
}

unsigned int Mesh::getPositionBufStride() const {
    return mPosNumComponents * mPosBytesComponent;
}

unsigned int Mesh::getPositionBufSize() const {
    return getPositionBufStride() * mNumVertices;
}

unsigned int Mesh::getPositionOffset() const {
    return mPosOffset;
}

void * Mesh::getPositionBuf() {
    return mMeshData.getBuf(mPosOffset);
}

unsigned int Mesh::getColorBufSize(int channel) const {
    return mColorBytesComponent[channel] *
        mColorNumComponents[channel] * mNumVertices;
}

unsigned int Mesh::getColorBufSize() const {
    int totalSize = 0;
    for (unsigned int i=0; i<getNumColorChannels(); i++)
        totalSize += getColorBufSize(i);
    return totalSize;
}

unsigned int Mesh::getTextureCoordBufSize(int channel) const {
    return mTextureCoordBytesComponent[channel] *
        mTextureCoordNumComponents[channel] * mNumVertices;
}

unsigned int Mesh::getTextureCoordBufSize() const {
    int totalSize = 0;
    for (unsigned int i=0; i<getNumTextureCoordChannels(); i++)
        totalSize += getTextureCoordBufSize(i);
    return totalSize;
}

unsigned int Mesh::getNormalNumComponent() const {
    return mNormalNumComponents;
}

unsigned int Mesh::getNormalBufStride() const {
    return mNormalNumComponents * mNormalBytesComponent;
}

unsigned int Mesh::getNormalBufSize() const {
    return getNormalBufStride() * mNumVertices;
}

unsigned int Mesh::getNormalOffset() const {
    return mNormalOffset;
}

void * Mesh::getNormalBuf() {
    return mMeshData.getBuf(mNormalOffset);
}

unsigned int Mesh::getTangentNumComponent() const {
    return mTangentNumComponents;
}

unsigned int Mesh::getTangentBufStride() const {
    return mTangentNumComponents * mTangentBytesComponent;
}

unsigned int Mesh::getTangentBufSize() const {
    return getTangentBufStride() * mNumVertices;
}

unsigned int Mesh::getTangentOffset() const {
    return mTangentOffset;
}

void * Mesh::getTangentBuf() {
    return mMeshData.getBuf(mTangentOffset);
}

unsigned int Mesh::getBitangentNumComponent() const {
    return mBitangentNumComponents;
}

unsigned int Mesh::getBitangentBufStride() const {
    return mBitangentNumComponents * mBitangentBytesComponent;
}

unsigned int Mesh::getBitangentBufSize() const {
    return getBitangentBufStride() * mNumVertices;
}

unsigned int Mesh::getBitangentOffset() const {
    return mBitangentOffset;
}

void * Mesh::getBitangentBuf() {
    return mMeshData.getBuf(mBitangentOffset);
}

unsigned int Mesh::getVertexBufSize() const {
    int totalSize = 0;
    if (hasVertexPositions())       totalSize += getPositionBufSize();
    if (hasVertexColors())          totalSize += getColorBufSize();
    if (hasVertexTextureCoords())   totalSize += getTextureCoordBufSize();
    if (hasVertexNormals())         totalSize += getNormalBufSize();
    if (hasVertexTangentsAndBitangents()) {
        totalSize += getTangentBufSize();
        totalSize += getBitangentBufSize();
    }
    return totalSize;
}

void * Mesh::getVertexBuf() {
    return mMeshData.getBuf();
}

unsigned int Mesh::getIndexBufSize() const {
    return getNumIndices() * sizeof(unsigned int);
}

void * Mesh::getIndexBuf() {
    if (mTriangleFaces.empty()) return NULL;
    return &mTriangleFaces[0];
}

void Mesh::appendVertexPositions(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mPosOffset = mMeshData.append(totalSize, buf);
    mPosNumComponents = numComponents;
    mPosBytesComponent = bytesEachComponent;
    mHasPos = true;
}

void Mesh::appendVertexColors(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent, unsigned int channel) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mColorOffset[channel] = mMeshData.append(totalSize, buf);
    mColorNumComponents[channel] = numComponents;
    mColorBytesComponent[channel] = bytesEachComponent;
    mNumColorChannels++;
}

void Mesh::appendVertexTextureCoords(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent, unsigned int channel) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mTextureCoordOffset[channel] = mMeshData.append(totalSize, buf);
    mTextureCoordNumComponents[channel] = numComponents;
    mTextureCoordBytesComponent[channel] = bytesEachComponent;
    mNumTextureCoordChannels++;
}

void Mesh::appendVertexNormals(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mNormalOffset = mMeshData.append(totalSize, buf);
    mNormalNumComponents = numComponents;
    mNormalBytesComponent = bytesEachComponent;
    mHasNormal = true;
}

void Mesh::appendVertexTangents(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mTangentOffset = mMeshData.append(totalSize, buf);
    mTangentNumComponents = numComponents;
    mTangentBytesComponent = bytesEachComponent;
    mHasTangent++;
}

void Mesh::appendVertexBitangents(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mBitangentOffset = mMeshData.append(totalSize, buf);
    mBitangentNumComponents = numComponents;
    mBitangentBytesComponent = bytesEachComponent;
    mHasBitangent++;
}

void Mesh::reserveDataStorage(int size) {
    mMeshData.reserve(size);
}

void Mesh::dumpVertexPositionBuf(int groupSize) {
    unsigned int bufSize = getPositionBufSize();
    float *buf = (float *)getPositionBuf();
    int num = bufSize/sizeof(float);
    char format[1024];

    if (num)
        PRINT("************ start Mesh::dumpVertexPositionBuf **********");
    for (int i=0; i<num; i+=groupSize) {
        int n = sprintf(format, "%8p:", buf + i);
        int left = (i+groupSize <= num) ? groupSize : num - i;
        for (int k=0; k<left; k++) {
            n += sprintf(format + n, " %+08.6f", buf[i+k]);
        }
        PRINT("%s", format);
    }
    if (num)
        PRINT("************ end Mesh::dumpVertexPositionBuf ************");
}

void Mesh::dumpIndexBuf(int groupSize) {
    unsigned int bufSize = getIndexBufSize();
    unsigned short *buf = (unsigned short *)getIndexBuf();
    // Attention: supports only integer type indices
    int num = bufSize/sizeof(unsigned short);
    char format[1024];

    if (num)
        PRINT("************ start Mesh::dumpIndexBuf **********");
    for (int i=0; i<num; i+=groupSize) {
        int n = sprintf(format, "%8p:", buf + i);
        int left = (i+groupSize <= num) ? groupSize : num - i;
        for (int k=0; k<left; k++) {
            n += sprintf(format + n, " %8u", buf[i+k]);
        }
        PRINT("%s", format);
    }
    if (num)
        PRINT("************ end Mesh::dumpIndexBuf ************");
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
