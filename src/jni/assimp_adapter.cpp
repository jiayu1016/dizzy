#include <memory>
#include "scene.h"
#include "log.h"
#include "assimp_adapter.h"

using namespace std;

namespace dzy {

AssetIOStream::AssetIOStream() {
}

AssetIOStream::~AssetIOStream() {
}

std::size_t AssetIOStream::Read(void* buffer, std::size_t size, std::size_t count) {
    return 0;
}

std::size_t AssetIOStream::Write(const void* buffer, std::size_t size, std::size_t count) {
    return 0;
}

aiReturn AssetIOStream::Seek(std::size_t offset, aiOrigin origin) {
    return aiReturn_SUCCESS;
}

std::size_t AssetIOStream::Tell() const {
    return 0;
}

std::size_t AssetIOStream::FileSize() const {
    return 0;
}

void AssetIOStream::Flush() {
}

shared_ptr<Camera> assimpTypeCast(aiCamera *camera) {
    shared_ptr<Camera> ret(new Camera(
        assimpTypeCast(camera->mPosition),
        assimpTypeCast(camera->mUp),
        assimpTypeCast(camera->mLookAt),
        camera->mHorizontalFOV,
        camera->mClipPlaneNear,
        camera->mClipPlaneFar,
        camera->mAspect));
    ret->mName = assimpTypeCast(camera->mName);

    return ret;
}

shared_ptr<Light> assimpTypeCast(aiLight *light) {
    Light::LightSourceType type;
    switch(light->mType) {
    case aiLightSource_UNDEFINED:
        type = Light::LIGHT_SOURCE_UNDEFINED;
        break;
    case aiLightSource_DIRECTIONAL:
        type = Light::LIGHT_SOURCE_DIRECTIONAL;
        break;
    case aiLightSource_POINT:
        type = Light::LIGHT_SOURCE_POINT;
        break;
    case aiLightSource_SPOT:
        type = Light::LIGHT_SOURCE_SPOT;
        break;
    default:
        ALOGE("unknown light source type");
        return NULL;
    }

    shared_ptr<Light> ret(new Light(
        type,
        light->mAttenuationConstant,
        light->mAttenuationLinear,
        light->mAttenuationQuadratic,
        light->mAngleInnerCone,
        light->mAngleOuterCone));
    ret->mName          = assimpTypeCast(light->mName);
    ret->mPosition      = assimpTypeCast(light->mPosition);
    ret->mDirection     = assimpTypeCast(light->mDirection);
    ret->mColorDiffuse  = assimpTypeCast(light->mColorDiffuse);
    ret->mColorSpecular = assimpTypeCast(light->mColorSpecular);
    ret->mColorAmbient  = assimpTypeCast(light->mColorAmbient);

    return ret;
}

shared_ptr<Texture> assimpTypeCast(aiTexture *texture) {
    shared_ptr<Texture> ret(new Texture);
    return ret;
}

shared_ptr<Animation> assimpTypeCast(aiAnimation *animation) {
    shared_ptr<Animation> ret(new Animation);
    return ret;
}

shared_ptr<Material> assimpTypeCast(aiMaterial *material) {
    shared_ptr<Material> ret(new Material);
    return ret;
}

shared_ptr<Mesh> assimpTypeCast(aiMesh *mesh) {
    shared_ptr<Mesh> ret(new Mesh);
    return ret;
}

shared_ptr<Node> assimpTypeCast(aiNode *node) {
    shared_ptr<Node> ret(new Node);
    return ret;
}

ndk_helper::Vec3 assimpTypeCast(const aiVector3D &vec3d) {
    return ndk_helper::Vec3(vec3d.x, vec3d.y, vec3d.z);
}

ndk_helper::Vec3 assimpTypeCast(const aiColor3D &color3d) {
    return ndk_helper::Vec3(color3d.r, color3d.g, color3d.b);
}

string assimpTypeCast(const aiString &str) {
    return string(str.C_Str());
}


} // namespace dzy
