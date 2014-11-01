#include <memory>
#include "scene.h"
#include "log.h"
#include "scene_graph.h"
#include "mesh.h"
#include "material.h"
#include "camera.h"
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

shared_ptr<Camera> AIAdapter::typeCast(aiCamera *camera) {
    shared_ptr<Camera> ret(new Camera(
        typeCast(camera->mPosition),
        typeCast(camera->mUp),
        typeCast(camera->mLookAt),
        camera->mHorizontalFOV,
        camera->mClipPlaneNear,
        camera->mClipPlaneFar,
        camera->mAspect));
    ret->mName = typeCast(camera->mName);

    return ret;
}

shared_ptr<Light> AIAdapter::typeCast(aiLight *light) {
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
    lt->mName          = typeCast(light->mName);
    lt->mPosition      = typeCast(light->mPosition);
    lt->mDirection     = typeCast(light->mDirection);
    lt->mColorDiffuse  = typeCast(light->mColorDiffuse);
    lt->mColorSpecular = typeCast(light->mColorSpecular);
    lt->mColorAmbient  = typeCast(light->mColorAmbient);

    return lt;
}

shared_ptr<Texture>AIAdapter::typeCast(aiTexture *texture) {
    shared_ptr<Texture> tex(new Texture);
    return tex;
}

shared_ptr<Animation> AIAdapter::typeCast(aiAnimation *animation) {
    shared_ptr<Animation> anim(new Animation);
    return anim;
}

shared_ptr<Material> AIAdapter::typeCast(aiMaterial *material) {
    shared_ptr<Material> ma(new Material);
    aiColor3D diffuse, specular, ambient, emission;
    float shininess;
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
    material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emission);
    material->Get(AI_MATKEY_SHININESS, shininess);
    ma->mDiffuse = typeCast(diffuse);
    ma->mSpecular = typeCast(specular);
    ma->mAmbient = typeCast(ambient);
    ma->mEmission = typeCast(emission);
    ma->mShininess = shininess;
    return ma;
}

shared_ptr<Mesh> AIAdapter::typeCast(aiMesh *mesh) {
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
    me->mName = typeCast(mesh->mName);
    // TODO: support point and line pritmive types
    me->mPrimitiveType = Mesh::PRIMITIVE_TYPE_TRIANGLES;
    me->mNumVertices = mesh->mNumVertices;
    me->mNumFaces = mesh->mNumFaces;

    int estimatedSize = 0;
    if (mesh->HasPositions())
        estimatedSize += me->mNumVertices * 3 *sizeof(float);
    estimatedSize += me->mNumVertices * mesh->GetNumColorChannels() * 4 * sizeof(float);
    estimatedSize += me->mNumVertices * mesh->GetNumUVChannels() * 3 * sizeof(float);
    if (mesh->HasNormals())
        estimatedSize += me->mNumVertices * 3 *sizeof(float);
    if (mesh->HasTangentsAndBitangents())
        estimatedSize += me->mNumVertices * 3 *sizeof(float) * 2;

    me->reserveDataStorage(estimatedSize);

    // Attention: rely on continuous memory layout of vertices in assimp
    if (mesh->HasPositions()) {
        me->appendVertexPositions(reinterpret_cast<unsigned char*>(mesh->mVertices),
            me->mNumVertices, 3, sizeof(float));
    }

    for (int i=0; i< mesh->GetNumColorChannels(); i++) {
        me->appendVertexColors(reinterpret_cast<unsigned char*>(mesh->mColors[i]),
            me->mNumVertices, 4, sizeof(float), i);
    }

    for (int i=0; i< mesh->GetNumUVChannels(); i++) {
        me->appendVertexTextureCoords(reinterpret_cast<unsigned char*>(mesh->mTextureCoords[i]),
            me->mNumVertices, mesh->mNumUVComponents[i], sizeof(float), i);
    }

    if (mesh->HasNormals()) {
        me->appendVertexNormals(reinterpret_cast<unsigned char*>(mesh->mNormals),
            me->mNumVertices, 3, sizeof(float));
    }

    if (mesh->HasTangentsAndBitangents()) {
        me->appendVertexTangents(reinterpret_cast<unsigned char*>(mesh->mTangents),
            me->mNumVertices, 3, sizeof(float));
        me->appendVertexBitangents(reinterpret_cast<unsigned char*>(mesh->mBitangents),
            me->mNumVertices, 3, sizeof(float));
    }
    if (mesh->HasFaces()) {
        ALOGD("mNumFaces: %u", me->mNumFaces);
        me->mTriangleFaces.resize(me->mNumFaces);
        // TODO: consider optimizing, avoid looping
        for (int i=0; i<me->mNumFaces; i++) {
            // Attention: support triange faces only
            assert(mesh->mFaces[i].mNumIndices == 3);
            me->mTriangleFaces[i].mIndices[0] = mesh->mFaces[i].mIndices[0];
            me->mTriangleFaces[i].mIndices[1] = mesh->mFaces[i].mIndices[1];
            me->mTriangleFaces[i].mIndices[2] = mesh->mFaces[i].mIndices[2];
        }
    }

    me->mMaterialIndex = mesh->mMaterialIndex;

    return me;
}

shared_ptr<Node> AIAdapter::typeCast(aiNode *node) {
    shared_ptr<Node> n(new Node);
    n->mName = typeCast(node->mName);
    n->mTransformation = typeCast(node->mTransformation);
    return n;
}

glm::vec3 AIAdapter::typeCast(const aiVector3D &vec3d) {
    return glm::vec3(vec3d.x, vec3d.y, vec3d.z);
}

glm::vec3 AIAdapter::typeCast(const aiColor3D &color3d) {
    return glm::vec3(color3d.r, color3d.g, color3d.b);
}

glm::vec4 AIAdapter::typeCast(const aiColor4D &color4d) {
    return glm::vec4(color4d.r, color4d.g, color4d.b, color4d.a);
}

// assimp matrix has different memory layout with glm matrix
glm::mat4 AIAdapter::typeCast(const aiMatrix4x4 &mat4) {
    glm::mat4 to;
    to[0][0] = mat4.a1; to[1][0] = mat4.a2; to[2][0] = mat4.a3; to[3][0] = mat4.a4;
    to[0][1] = mat4.b1; to[1][1] = mat4.b2; to[2][1] = mat4.b3; to[3][1] = mat4.b4;
    to[0][2] = mat4.c1; to[1][2] = mat4.c2; to[2][2] = mat4.c3; to[3][2] = mat4.c4;
    to[0][3] = mat4.d1; to[1][3] = mat4.d2; to[2][3] = mat4.d3; to[3][3] = mat4.d4;
    return to;
}

string AIAdapter::typeCast(const aiString &str) {
    return string(str.C_Str());
}

void AIAdapter::buildSceneGraph(shared_ptr<Scene> scene, aiNode *aroot) {
    shared_ptr<Node> root = typeCast(aroot);
    linkNode(scene, root, aroot);
    scene->mRootNode = root;
}

void AIAdapter::linkNode(shared_ptr<Scene> scene,
    shared_ptr<Node> node, aiNode *anode) {
    for (unsigned int i = 0; i < anode->mNumChildren; i++) {
        shared_ptr<Node> c = typeCast(anode->mChildren[i]);
        node->attachChild(c);
        linkNode(scene, c, anode->mChildren[i]);
    }
    // build one GeoNode to reference one Mesh
    for (unsigned int i = 0; i < anode->mNumMeshes; i++) {
        unsigned int meshIdx = anode->mMeshes[i];
        shared_ptr<GeoNode> c(new GeoNode(scene->mMeshes[meshIdx]));
        node->attachChild(c);
    }
}

} // namespace dzy
