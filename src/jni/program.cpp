#include <android/asset_manager.h>
#include <shader.h>
#include "utils.h"
#include "scene.h"
#include "program.h"

using namespace std;

namespace dzy {

Shader::Shader(GLenum type)
    : mShaderType(type)
    , mShaderId(0)
    , mInitialized(false) {
}

Shader::~Shader() {
    if (mInitialized && mShaderId)
        glDeleteShader(mShaderId);
}

GLenum Shader::getShaderType() {
    return mShaderType;
}

GLuint Shader::getShaderID() {
    return mShaderId;
}

bool Shader::compileFromMemory(std::vector<uint8_t>& data) {
    bool success = ndk_helper::shader::CompileShader(&mShaderId, mShaderType, data);
    if (success) mInitialized = true;
    return success;
}

bool Shader::compileFromMemory(const GLchar *source, const int32_t size) {
    bool success = ndk_helper::shader::CompileShader(&mShaderId, mShaderType, source, size);
    if (success) mInitialized = true;
    return success;
}

bool Shader::compileFromFile(const char *strFileName) {
    bool success = ndk_helper::shader::CompileShader(&mShaderId, mShaderType, strFileName);
    if (success) mInitialized = true;
    return success;
}

bool Shader::compileFromAsset(
    AAssetManager *assetManager, const string &assetFile) {
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

    return compileFromMemory(buffer.get(), sz);
}

Program::Program()
    : mLinked(false)
    , mProgramId(0) {
    ALOGD("Program::Program()");
    mProgramId = glCreateProgram();
    if (!mProgramId) {
        ALOGE("glCreateProgram error");
        return;
    }
}

Program::~Program() {
    ALOGD("Program::~Program()");
    if (mProgramId) glDeleteProgram(mProgramId);
    if (!mVertexBOs.empty())
        glDeleteBuffers(mVertexBOs.size(), &mVertexBOs[0]);
    if (!mIndexBOs.empty())
        glDeleteBuffers(mIndexBOs.size(), &mIndexBOs[0]);
}

void Program::use() {
    glUseProgram(mProgramId);
}

void Program::bindBufferObjects(int meshIdx) {
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBOs[meshIdx]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBOs[meshIdx]);
}

bool Program::link(std::shared_ptr<Shader> vtxShader, std::shared_ptr<Shader> fragShader) {
    if (isValid()) {
        ALOGW("Program already linked");
        return false;
    }

    // vertex always slot 0, fragment always slot 1
    mShaders.push_back(vtxShader);
    mShaders.push_back(fragShader);

    // in order to support other shaders in future
    for (auto iter = mShaders.begin(); iter != mShaders.end(); iter++) {
        if (*iter) glAttachShader(mProgramId, (*iter)->getShaderID());
    }

    if (!ndk_helper::shader::LinkProgram(mProgramId)) {
        ALOGE("error link program");
        return false;
    }

    if (!ndk_helper::shader::ValidateProgram(mProgramId)) {
        ALOGE("program validation fail");
        return false;
    }

    mLinked = true;

    return true;
}

bool Program::load(std::shared_ptr<Scene> scene) {
    if (scene->atLeastOneMeshHasVertexPosition()) {
        GLint attribLoc = glGetAttribLocation(mProgramId, "dzyVertexPosition");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex position");
            return false;
        }
        mLocations["dzyVertexPosition"] = attribLoc;
    }
    if (scene->atLeastOneMeshHasVertexColor()) {
        GLint attribLoc = glGetAttribLocation(mProgramId, "dzyVertexColor");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex color");
            return false;
        }
        mLocations["dzyVertexColor"] = attribLoc;
    }
    if (scene->atLeastOneMeshHasNormal()) {
        GLint attribLoc = glGetAttribLocation(mProgramId, "dzyVertexNormal");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex normal");
            return false;
        }
        mLocations["dzyVertexNormal"] = attribLoc;
    }

    GLint mvpLoc = glGetUniformLocation(mProgramId, "dzyMVPMatrix");
    if (mvpLoc == -1) {
        ALOGE("shader not compatible with scene: mvp matrix");
        return false;
    }
    mLocations["dzyMVPMatrix"] = mvpLoc;

    GLint mvLoc = glGetUniformLocation(mProgramId, "dzyMVMatrix");
    if (mvLoc != -1)
        mLocations["dzyMVMatrix"] = mvLoc;

    GLint normalMatrixLoc = glGetUniformLocation(mProgramId, "dzyNormalMatrix");
    if (normalMatrixLoc != -1)
        mLocations["dzyNormalMatrix"] = normalMatrixLoc;

    if (scene->getNumMaterials() > 0) {
        GLint loc = -1;
        loc = glGetUniformLocation(mProgramId, "dzyMaterial.diffuse");
        if (loc != -1) mLocations["dzyMaterial.diffuse"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyMaterial.specular");
        if (loc != -1) mLocations["dzyMaterial.specular"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyMaterial.ambient");
        if (loc != -1) mLocations["dzyMaterial.ambient"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyMaterial.emission");
        if (loc != -1) mLocations["dzyMaterial.emission"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyMaterial.shininess");
        if (loc != -1) mLocations["dzyMaterial.shininess"] = loc;
    }

    if (scene->getNumLights() > 0) {
        GLint loc = -1;
        loc = glGetUniformLocation(mProgramId, "dzyLight.color");
        if (loc != -1) mLocations["dzyLight.color"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.ambient");
        if (loc != -1) mLocations["dzyLight.ambient"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.position");
        if (loc != -1) mLocations["dzyLight.position"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.attenuationConstant");
        if (loc != -1) mLocations["dzyLight.attenuationConstant"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.attenuationLinear");
        if (loc != -1) mLocations["dzyLight.attenuationLinear"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.attenuationQuadratic");
        if (loc != -1) mLocations["dzyLight.attenuationQuadratic"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.strength");
        if (loc != -1) mLocations["dzyLight.strength"] = loc;
    }

    size_t numMeshes = scene->getNumMeshes();
    mVertexBOs.resize(numMeshes);
    mIndexBOs.resize(numMeshes);
    for(size_t i = 0; i < numMeshes; ++i) {
        // Load vertex and index data into buffer object
        shared_ptr<Mesh> mesh(scene->mMeshes[i]);
        glGenBuffers(1, &mVertexBOs[i]);
        glGenBuffers(1, &mIndexBOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, mesh->getVertexBufSize(),
            mesh->getVertexBuf(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->getIndexBufSize(),
            mesh->getIndexBuf(), GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return true;
}

GLint Program::getLocation(const char* name) {
    if (!isValid()) {
        ALOGE("Program not valid");
        return -1;
    }
    if (mLocations.find(name) == mLocations.end()) {
        ALOGE("%s not found in shader", name);
        return -1;
    }
    return mLocations[name];
}

} //namespace dzy
