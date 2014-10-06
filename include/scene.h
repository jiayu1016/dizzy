// this file is designed to hide everything about assimp
#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <vecmath.h>
#include "utils.h"

namespace dzy {

// definitions of scene elements  copied from assimp

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

public:
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
class Mesh {
};
class Node {
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
typedef std::list<std::shared_ptr<Node> >          NodeContainer;

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
    NodeContainer       mNodes;
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
