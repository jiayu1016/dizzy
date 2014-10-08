// designed to hide data structures of assimp,  type conversion
// is implemented in overloaded functions in other files
#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <functional>
#include <algorithm>
#include <vecmath.h>
#include "utils.h"

namespace dzy {

class AIAdapter;

class Camera {
public:
    Camera();
    Camera(
        ndk_helper::Vec3    position,
        ndk_helper::Vec3    up,
        ndk_helper::Vec3    lookAt,
        float               horizontalFOV,
        float               clipPlaneNear,
        float               clipPlaneFar,
        float               aspect);
    ndk_helper::Mat4 getMatrix();

    friend class AIAdapter;
private:
    std::string         mName;
    ndk_helper::Vec3    mPosition;
    ndk_helper::Vec3    mUp;
    ndk_helper::Vec3    mLookAt;
    float               mHorizontalFOV;
    float               mClipPlaneNear;
    float               mClipPlaneFar;
    float               mAspect;

};

class Light {
public:
    enum LightSourceType {
        LIGHT_SOURCE_UNDEFINED      = 0x0,
        LIGHT_SOURCE_DIRECTIONAL    = 0x1,
        LIGHT_SOURCE_POINT          = 0x2,
        LIGHT_SOURCE_SPOT           = 0x3,
    };
    Light();
    Light(
        LightSourceType type,
        float           attenuationConstant,
        float           attenuationLinear,
        float           attenuationQuadratic,
        float           angleInnerCone,
        float           angleOuterCone);

    friend class AIAdapter;
private:
    std::string         mName;
    LightSourceType     mType;
    ndk_helper::Vec3    mPosition;
    ndk_helper::Vec3    mDirection;
    float               mAttenuationConstant;
    float               mAttenuationLinear;
    float               mAttenuationQuadratic;
    ndk_helper::Vec3    mColorDiffuse;
    ndk_helper::Vec3    mColorSpecular;
    ndk_helper::Vec3    mColorAmbient;
    float               mAngleInnerCone;
    float               mAngleOuterCone;

};
class Texture {
};
class Material {
};
class Animation {
};

/*
 * Used to hold raw buffer data of Mesh
 */
class MeshData {
public:
    enum MeshDataType {
        MESH_DATA_TYPE_NONE,
        MESH_DATA_TYPE_FLOAT,
        MESH_DATA_TYPE_INT,
        MESH_DATA_TYPE_UNSIGNED_SHORT,
        MESH_DATA_TYPE_FIXED16_16,
        MESH_DATA_TYPE_UNSIGNED_BYTE,
        MESH_DATA_TYPE_SHORT,
        MESH_DATA_TYPE_SHORT_NORM,
        MESH_DATA_TYPE_BYTE,
        MESH_DATA_TYPE_BYTE_NORM,
        MESH_DATA_TYPE_UNSIGNED_BYTE_NORM,
        MESH_DATA_TYPE_UNSIGNED_SHORT_NORM,
        MESH_DATA_TYPE_UNSIGNED_INT,
        MESH_DATA_TYPE_RGBA,
        MESH_DATA_TYPE_ARGB,
        MESH_DATA_TYPE_ABGR,
    };

    MeshData();
    inline bool empty() const { return mBuffer.get() == NULL; }
    void reset();
    void set(MeshDataType type, unsigned int numComponents,
        unsigned int stride, unsigned int numVertices,
        unsigned char *rawBuffer);

private:
    MeshDataType                    mType;
    unsigned int                    mNumComponents;
    unsigned int                    mStride;
    // c++11 shared_ptr doesn't support array, must explicitly set deleter
    std::shared_ptr<unsigned char>  mBuffer;
};

typedef std::vector<MeshData>       MeshDataContainer;
class Mesh {
public:
    enum {
        MAX_COLOR_SETS          = 0x8,
        MAX_TEXTURECOORDS       = 0x8,
    };

    enum PrimitiveType {
        PRIMITIVE_TYPE_TRIANGLES = 0,
        PRIMITIVE_TYPE_NUM
    };

    Mesh();

    bool hasPositions() const;
    bool hasFaces() const;
    bool hasNormals() const;
    bool hasTangentsAndBitangents() const;
    bool hasVertexColors(unsigned int index) const;
    bool hasTextureCoords(unsigned int index) const;
    unsigned int getNumUVChannels() const;
    unsigned int getNumColorChannels() const;
    
    friend class AIAdapter;
private:
    std::string             mName;
    PrimitiveType           mPrimitiveType;

    unsigned int            mNumVertices;
    unsigned int            mNumFaces;
    unsigned int            mNumUVComponents[MAX_TEXTURECOORDS];

    MeshData                mVertices;
    MeshData                mNormals;
    MeshData                mTangents;
    MeshData                mBitangents;
    MeshDataContainer       mTriangleFaces;

    MeshData                mColors         [MAX_COLOR_SETS];
    MeshData                mTextureCoords  [MAX_TEXTURECOORDS];

    // A mesh use only ONE material, otherwise it is splitted to multiple meshes
    unsigned int            mMaterialIndex;
};

class NodeTree;
class Node : public std::enable_shared_from_this<Node> {
public:
    Node() { }
    Node(const std::string& name) : mName(name) { }

    void addChild(std::shared_ptr<Node> node);
    //void setParent(std::shared_ptr<Node> parent);
    std::shared_ptr<Node> findNode(const std::string &name);

    friend class NodeTree;
    friend class AIAdapter;
private:

    std::string                             mName;
    ndk_helper::Mat4                        mTransformation;
    std::weak_ptr<Node>                     mParent;
    std::vector<std::shared_ptr<Node> >     mChildren;
    std::vector<int>                        mMeshes;

};

class NodeTree {
public:
    inline void setRoot(std::shared_ptr<Node> root) {
        mRoot = root;
    }
    void dfsTraversal(std::function<void(std::shared_ptr<Node>)> visit);

    friend class AIAdapter;
private:
    void dfsTraversal(std::shared_ptr<Node> node,
        std::function<void(std::shared_ptr<Node>)> visit);

    std::shared_ptr<Node> mRoot;
};

class SceneManager;
class AppContext;
class Scene {
public:
    Scene();
    virtual ~Scene();

    virtual bool loadColladaAsset(std::shared_ptr<AppContext> appContext,
        const std::string &assetFile) = 0;
    virtual bool load(std::shared_ptr<AppContext> appContext,
        const std::string &file) = 0;
    virtual bool listAssetFiles(std::shared_ptr<AppContext> appContext,
        const std::string &dir) = 0;

protected:

    void * mSceneData;
    std::size_t mSceneSize;
};

typedef std::vector<std::shared_ptr<Camera> >      CameraContainer;
typedef std::vector<std::shared_ptr<Light> >       LightContainer;
typedef std::vector<std::shared_ptr<Animation> >   AnimationContainer;
typedef std::vector<std::shared_ptr<Texture> >     TextureContainer;
typedef std::vector<std::shared_ptr<Material> >    MaterialContainer;
typedef std::vector<std::shared_ptr<Mesh> >        MeshContainer;
class FlatScene : public Scene {
public:
    explicit FlatScene();
    virtual ~FlatScene();

    virtual bool loadColladaAsset(std::shared_ptr<AppContext> appContext, const std::string &asset);
    virtual bool load(std::shared_ptr<AppContext> appContext, const std::string &file);
    virtual bool listAssetFiles(std::shared_ptr<AppContext> appContext,
        const std::string &dir);

    friend class SceneManager;

private:
    unsigned int        mFlags;
    CameraContainer     mCameras;
    LightContainer      mLights;
    TextureContainer    mTextures;
    AnimationContainer  mAnimations;
    MaterialContainer   mMaterials;
    MeshContainer       mMeshes;
    NodeTree            mNodeTree;
    std::size_t         mNumNode;
};

class SceneManager : Singleton<SceneManager> {
public:
    enum SceneType {
        SCENE_TYPE_FLAT,
    };
    ~SceneManager();
    static std::shared_ptr<Scene> createScene(SceneType);

private:    
    explicit SceneManager();
};

} // namespace dzy
#endif
