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
#include "render.h"
#include "scene.h"

using namespace std;

namespace dzy {

MeshData::MeshData()
    : mType(MESH_DATA_TYPE_FLOAT)
    , mNumComponents(0)
    , mStride(0)
    , mBufSize(0)
    , mBuffer(NULL, std::default_delete<unsigned char[]>()) {
}

void MeshData::reset() {
    mType = MESH_DATA_TYPE_FLOAT;
    mNumComponents = 0;
    mStride = 0;
    mBufSize = 0;
    mBuffer.reset();
}

void MeshData::set(MeshDataType type, unsigned int numComponents,
    unsigned int stride, unsigned int numVertices,
    unsigned char *rawBuffer) {
    mBufSize = numVertices * stride;
    unsigned char *newBuf = new unsigned char[mBufSize];
    memcpy(newBuf, rawBuffer, mBufSize);
    mType = type;
    mNumComponents = numComponents;
    mStride = stride;
    mBuffer.reset(newBuf, std::default_delete<unsigned char[]>());
}

void * MeshData::getBuf() {
    return static_cast<void *>(mBuffer.get());
}

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

Mesh::Mesh()
    : mPrimitiveType    (PRIMITIVE_TYPE_TRIANGLES)
    , mNumVertices      (0)
    , mNumFaces         (0)
    , mMaterialIndex    (0) {
    memset(mNumUVComponents, 0, MAX_TEXTURECOORDS * sizeof(unsigned int));
}

bool Mesh::hasPositions() const {
    return !mVertices.empty() && mNumVertices > 0;
}

bool Mesh::hasFaces() const {
    return !mTriangleFaces.empty() && mNumFaces > 0;
}

bool Mesh::hasNormals() const {
    return !mNormals.empty() && mNumVertices > 0;
}

bool Mesh::hasTangentsAndBitangents() const {
    return !mTangents.empty() && !mBitangents.empty() && mNumVertices > 0;
}

bool Mesh::hasVertexColors(unsigned int index) const {
    if( index >= MAX_COLOR_SETS)
        return false;
    else
        return !mColors[index].empty() && mNumVertices > 0;
}

bool Mesh::hasVertexColors() const {
    for (unsigned int i=0; i<MAX_COLOR_SETS; i++) {
        if (hasVertexColors(i)) return true;
    }
    return false;
}

bool Mesh::hasTextureCoords(unsigned int index) const {
    if( index >= MAX_TEXTURECOORDS)
        return false;
    else
        return !mTextureCoords[index].empty() && mNumVertices > 0;
}

unsigned int Mesh::getNumUVChannels() const {
    unsigned int n = 0;
    while (n < MAX_TEXTURECOORDS && !mTextureCoords[n].empty()) ++n;
    return n;
}

unsigned int Mesh::getNumColorChannels() const {
    unsigned int n = 0;
    while (n < MAX_COLOR_SETS && !mColors[n].empty()) ++n;
    return n;
}
unsigned int Mesh::getNumVertices() const {
    return mNumVertices;
}

unsigned int Mesh::getNumFaces() const {
    return mNumFaces;
}

unsigned int Mesh::getVertexNumComponent() {
    return mVertices.getNumComponents();
}

unsigned int Mesh::getVertexBufSize() {
    return mVertices.getBufSize();
}
unsigned int Mesh::getVertexBufStride() {
    return mVertices.getBufStride();
}

void * Mesh::getVertexBuf() {
    return mVertices.getBuf();
}

unsigned int Mesh::getNumIndices() {
    return getNumFaces() * 3;
}

unsigned int Mesh::getIndexBufSize() {
    return getNumIndices() * sizeof(float);
}

void * Mesh::getIndexBuf() {
    if (mTriangleFaces.empty()) return NULL;
    // Attention: 
    return &mTriangleFaces[0];
}

void Mesh::dumpVertexBuf(int groupSize) {
    unsigned int bufSize = getVertexBufSize();
    float *buf = (float *)getVertexBuf();
    int num = bufSize/sizeof(float);
    char format[1024];

    if (num)
        PRINT("************ start Mesh::dumpVertexBuf **********");
    for (int i=0; i<num; i+=groupSize) {
        int n = sprintf(format, "%8p:", buf + i);
        int left = (i+groupSize <= num) ? groupSize : num - i;
        for (int k=0; k<left; k++) {
            n += sprintf(format + n, " %+08.6f", buf[i+k]);
        }
        PRINT("%s", format);
    }
    if (num)
        PRINT("************ end Mesh::dumpVertexBuf ************");
}

void Mesh::dumpIndexBuf(int groupSize) {
    unsigned int bufSize = getIndexBufSize();
    int *buf = (int *)getIndexBuf();
    // Attention: supports only integer type indices
    int num = bufSize/sizeof(int);
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

void Node::addChild(shared_ptr<Node> node) {
    // no duplicate Node in children list
    bool exist = false;
    for (auto iter = mChildren.begin(); iter != mChildren.end(); iter++) {
        if (*iter == node) {
            exist = true;
            ALOGW("avoid inserting duplicate Node");
            break;
        }
    }

    if (!exist) {
        mChildren.push_back(node);
        node->mParent = shared_from_this();
        shared_ptr<Node> oldParent(node->mParent.lock());
        if (oldParent) {
            // remove node from oldParent
            remove(oldParent->mChildren.begin(), oldParent->mChildren.end(), node);
        }
    }
}

/*
void Node::setParent(shared_ptr<Node> parent) {
    shared_ptr<Node> oldParent(mParent.lock());
    if (oldParent) {
        if (oldParent != parent) {
            // remove this node from original parent's children list
            for (auto iter = oldParent->mChildren.begin();
                iter != oldParent->mChildren.end();
                iter++) {
                if ((*iter).get() == this) {
                    oldParent->mChildren.erase(iter);
                    break;
                }
            }
        } else {
            ALOGD("double set, ignore");
            return;
        }
    }

    mParent = parent;
    parent->insertChild(shared_from_this());
}
*/

shared_ptr<Node> Node::findNode(const string &name) {
    if (mName == name) return shared_from_this();
    for (size_t i = 0; i < mChildren.size(); i++) {
        shared_ptr<Node> node(mChildren[i]->findNode(name));
        if (node) return node;
    }
    return NULL;
}

void NodeTree::dfsTraversal(shared_ptr<Scene> scene, VisitFunc visit) {
    dfsTraversal(scene, mRoot, visit);
}

void NodeTree::dfsTraversal(shared_ptr<Scene> scene, shared_ptr<Node> node, VisitFunc visit) {
    visit(scene, node);
    std::for_each(node->mChildren.begin(), node->mChildren.end(), [&] (shared_ptr<Node> c) {
        dfsTraversal(scene, c, visit);
    });
}

bool Scene::loadColladaAsset(shared_ptr<AppContext> appContext,
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
    unique_ptr<char[]> buffer(new char[length]);
    size_t sz = AAsset_read(asset, buffer.get(), length);
    AAsset_close(asset);
    if (sz != length) {
        ALOGE("Partial read %d bytes", sz);
        return false;
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
        mCameras.push_back(camera);
    }
    for (int i=0; i<scene->mNumLights; i++) {
        shared_ptr<Light> light(AIAdapter::typeCast(scene->mLights[i]));
        mLights.push_back(light);
    }
    for (int i=0; i<scene->mNumTextures; i++) {
        shared_ptr<Texture> texture(AIAdapter::typeCast(scene->mTextures[i]));
        mTextures.push_back(texture);
    }
    for (int i=0; i<scene->mNumAnimations; i++) {
        shared_ptr<Animation> animation(AIAdapter::typeCast(scene->mAnimations[i]));
        mAnimations.push_back(animation);
    }
    for (int i=0; i<scene->mNumMaterials; i++) {
        shared_ptr<Material> material(AIAdapter::typeCast(scene->mMaterials[i]));
        mMaterials.push_back(material);
    }
    for (int i=0; i<scene->mNumMeshes; i++) {
        shared_ptr<Mesh> mesh(AIAdapter::typeCast(scene->mMeshes[i]));
        mMeshes.push_back(mesh);
    }

    AIAdapter::buildNodeTree(scene->mRootNode, mNodeTree);

    long long cvtTime = cvtDuration.getMicroSeconds();

    ALOGD("assimp load: %lld ms, conversion: %lld us", importTime, cvtTime);

    return true;
}

bool Scene::load(shared_ptr<AppContext> appContext,
    const string &file) {
    ifstream ifs(file.c_str(), ifstream::binary);
    if (!ifs) {
        ALOGE("ifstream ctor failed for %s", file.c_str());
        return false;
    }

    ifs.seekg(0, ifs.end);
    int length = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    unique_ptr<char[]> buffer(new char[length]);
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

bool Scene::listAssetFiles(shared_ptr<AppContext> appContext,
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

bool Scene::atLeastOneMeshHasVertexPosition() {
    for (size_t i=0; i<getNumMeshes(); i++) {
        if (mMeshes[i]->hasPositions()) return true;
    }
    return false;
}

bool Scene::atLeastOneMeshHasVertexColor() {
    for (size_t i=0; i<getNumMeshes(); i++) {
        if (mMeshes[i]->hasVertexColors()) return true;
    }
    return false;
}

SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
}

shared_ptr<Scene> SceneManager::createScene(SceneType sceneType) {
    (void)sceneType;
    shared_ptr<Scene> scene(new Scene);
    if (scene) {
        SceneManager::get()->addScene(scene);
        mCurrentScene = scene;
    }
    return scene;
}

void SceneManager::addScene(shared_ptr<Scene> scene) {
    mScenes.push_back(scene);
}

shared_ptr<Scene> SceneManager::getCurrentScene() {
    return mCurrentScene;
}


} // namespace dzy
