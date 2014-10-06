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

    shared_ptr<Light> lt(new Light(
        type,
        light->mAttenuationConstant,
        light->mAttenuationLinear,
        light->mAttenuationQuadratic,
        light->mAngleInnerCone,
        light->mAngleOuterCone));
    lt->mName          = assimpTypeCast(light->mName);
    lt->mPosition      = assimpTypeCast(light->mPosition);
    lt->mDirection     = assimpTypeCast(light->mDirection);
    lt->mColorDiffuse  = assimpTypeCast(light->mColorDiffuse);
    lt->mColorSpecular = assimpTypeCast(light->mColorSpecular);
    lt->mColorAmbient  = assimpTypeCast(light->mColorAmbient);

    return lt;
}

shared_ptr<Texture> assimpTypeCast(aiTexture *texture) {
    shared_ptr<Texture> tex(new Texture);
    return tex;
}

shared_ptr<Animation> assimpTypeCast(aiAnimation *animation) {
    shared_ptr<Animation> anim(new Animation);
    return anim;
}

shared_ptr<Material> assimpTypeCast(aiMaterial *material) {
    shared_ptr<Material> ma(new Material);
    return ma;
}

shared_ptr<Mesh> assimpTypeCast(aiMesh *mesh) {
    bool hasPoint = false, hasLine = false, hasTriangle = false;
    unsigned int type(mesh->mPrimitiveTypes);
    if (type & aiPrimitiveType_POLYGON) {
        ALOGE("polygon primivite type not supported");
        return NULL;
    }
    if (type & aiPrimitiveType_POINT)       hasPoint    = true;
    if (type & aiPrimitiveType_LINE)        hasLine     = true;
    if (type & aiPrimitiveType_TRIANGLE)    hasTriangle = true;

    if (!(hasPoint || hasLine || hasTriangle)) {
        ALOGE("no pritimive type found");
        return NULL;
    }

    // assimp document says "SortByPrimitiveType" can be used to
    // make sure output meshes contain only one primitive type each
    if ((hasPoint && hasLine) ||
        (hasPoint && hasTriangle) ||
        (hasLine && hasTriangle)) {
        ALOGE("mixed pritimive type not allowed, consider using assimp with different options");
        return NULL;
    }

    shared_ptr<Mesh> me(new Mesh);
    me->mName = assimpTypeCast(mesh->mName);
    // TODO: support point and line pritmive types
    me->mPrimitiveType = Mesh::PRIMITIVE_TYPE_TRIANGLES;
    me->mNumVertices = mesh->mNumVertices;
    me->mNumFaces = mesh->mNumFaces;

    for (int i=0; i< mesh->GetNumUVChannels(); i++) {
        me->mNumUVComponents[i] = mesh->mNumUVComponents[i];
        ALOGD("mesh->mNumUVComponents[%d]: %u", i, mesh->mNumUVComponents[i]);
        // TODO: consider optimizing
        for (int j=0; j < mesh->mNumVertices; j++) {
            me->mTextureCoords[i].push_back(ndk_helper::Vec3(
                mesh->mTextureCoords[i][j].x,
                mesh->mTextureCoords[i][j].y,
                mesh->mTextureCoords[i][j].z));
        }
    }

    for (int i=0; i< mesh->GetNumColorChannels(); i++) {
        for (int j=0; j < mesh->mNumVertices; j++) {
            me->mColors[i].push_back(ndk_helper::Vec4(
                mesh->mColors[i][j].r,
                mesh->mColors[i][j].g,
                mesh->mColors[i][j].b,
                mesh->mColors[i][j].a));
        }
    }

    // Attention: rely on continuous memory layout of vertices in assimp
    if (mesh->HasPositions()) {
        me->mVertices.set(MeshData::MESH_DATA_TYPE_FLOAT, 3, 3 * sizeof(float),
            me->mNumVertices, reinterpret_cast<unsigned char*>(mesh->mVertices));
    }
    if (mesh->HasNormals()) {
        me->mNormals.set(MeshData::MESH_DATA_TYPE_FLOAT,
            3, 3 * sizeof(float), me->mNumVertices,
            reinterpret_cast<unsigned char*>(mesh->mNormals));
    }
    if (mesh->HasTangentsAndBitangents()) {
        me->mTangents.set(MeshData::MESH_DATA_TYPE_FLOAT,
            3, 3 * sizeof(float), me->mNumVertices,
            reinterpret_cast<unsigned char*>(mesh->mTangents));
        me->mBitangents.set(MeshData::MESH_DATA_TYPE_FLOAT,
            3, 3 * sizeof(float), me->mNumVertices,
            reinterpret_cast<unsigned char*>(mesh->mBitangents));
    }
    // Attention: support triange faces only
    if (mesh->HasFaces()) {
        // TODO: track mNumFaces, consider optimizing
        for (int i=0; i< me->mNumFaces; i++) {
            MeshData *meshData = new MeshData;
            meshData->set(MeshData::MESH_DATA_TYPE_INT,
                1, 1 * sizeof(int), mesh->mFaces[i].mNumIndices,
                reinterpret_cast<unsigned char*>(mesh->mFaces[i].mIndices));
            me->mTriangleFaces.push_back(shared_ptr<MeshData>(meshData));
        }
    }

    // TODO: rethink the material index
    me->mMaterialIndex = mesh->mMaterialIndex;

    return me;
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
