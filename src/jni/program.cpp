#include <android/asset_manager.h>
#include <shader.h>
#include "utils.h"
#include "scene.h"
#include "engine_context.h"
#include "program.h"

using namespace std;

namespace dzy {

Shader::Shader(ShaderType type)
    : mShaderId(0)
    , mInitialized(false) {
    switch (type) {
    case Vertex:
        mShaderType = GL_VERTEX_SHADER;
        break;
    case Fragment:
        mShaderType = GL_FRAGMENT_SHADER;
        break;
    case Geometry:
        // TODO:
        break;
    default:
        break;
    }
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
}

void Program::use() {
    glUseProgram(mProgramId);
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

bool Program::storeLocation() {

#define STORE_CHECK_ATTRIB_LOC(ATTRIB) do {                             \
        GLint attribLoc = glGetAttribLocation(mProgramId, ATTRIB);      \
        if (attribLoc == -1) {                                          \
            break;                                                      \
        }                                                               \
        mLocations[ATTRIB] = attribLoc;                                 \
    } while(0)

#define STORE_CHECK_UNIFORM_LOC(UNIFORM) do {                           \
        GLint uniformLoc = glGetUniformLocation(mProgramId, UNIFORM);   \
        if (uniformLoc == -1) {                                         \
            break;                                                      \
        }                                                               \
        mLocations[UNIFORM] = uniformLoc;                               \
    } while(0)


    STORE_CHECK_ATTRIB_LOC("dzyVertexPosition");
    STORE_CHECK_ATTRIB_LOC("dzyVertexNormal");
    STORE_CHECK_UNIFORM_LOC("dzyMVPMatrix");
    STORE_CHECK_UNIFORM_LOC("dzyMVMatrix");
    STORE_CHECK_UNIFORM_LOC("dzyNormalMatrix");
    STORE_CHECK_UNIFORM_LOC("dzyMaterial.diffuse");
    STORE_CHECK_UNIFORM_LOC("dzyMaterial.specular");
    STORE_CHECK_UNIFORM_LOC("dzyMaterial.ambient");
    STORE_CHECK_UNIFORM_LOC("dzyMaterial.emission");
    STORE_CHECK_UNIFORM_LOC("dzyMaterial.shininess");
    STORE_CHECK_UNIFORM_LOC("dzyLight.color");
    STORE_CHECK_UNIFORM_LOC("dzyLight.ambient");
    STORE_CHECK_UNIFORM_LOC("dzyLight.position");
    STORE_CHECK_UNIFORM_LOC("dzyLight.attenuationConstant");
    STORE_CHECK_UNIFORM_LOC("dzyLight.attenuationLinear");
    STORE_CHECK_UNIFORM_LOC("dzyLight.attenuationQuadratic");
    STORE_CHECK_UNIFORM_LOC("dzyLight.strength");

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

bool ProgramManager::preCompile(shared_ptr<EngineContext> engineContext) {
    shared_ptr<Shader> vtxShader(new Shader(Shader::Vertex));
    if (!vtxShader->compileFromAsset(engineContext->getAssetManager(), "vertex.shader")) {
        ALOGE("error compile vertex shader");
        return false;
    }
    shared_ptr<Shader> fragShader(new Shader(Shader::Fragment));
    if (!fragShader->compileFromAsset(engineContext->getAssetManager(), "fragment.shader")) {
        ALOGE("error compile fragment shader");
        return false;
    }
    shared_ptr<Program> program(new Program);
    if (!program->link(vtxShader, fragShader)) {
        ALOGE("error link program");
        return false;
    }

    program->use();
    program->storeLocation();

    mPrograms.push_back(program);

    return true;
}

shared_ptr<Program> ProgramManager::getDefaultProgram() {
    // TODO:
    return mPrograms[0];
}

} //namespace dzy
