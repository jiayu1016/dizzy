#include <android/asset_manager.h>
#include <shader.h>
#include "utils.h"
#include "scene.h"
#include "engine_context.h"
#include "mesh.h"
#include "material.h"
#include "light.h"
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
    , mProgramId(0)
    , mRequirement(0) {
    mProgramId = glCreateProgram();
    if (!mProgramId) {
        ALOGE("glCreateProgram error");
        return;
    }
}

Program::~Program() {
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
    STORE_CHECK_ATTRIB_LOC("dzyVertexColor");
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

static const char VERTEX_simple_vertex_color[] =
"#version 300 es\n"
"uniform mat4 dzyMVPMatrix;\n"
"in vec3 dzyVertexPosition;\n"
"in vec3 dzyVertexColor;\n"
"out vec3 vVertexColor;\n"
"void main() {\n"
"    gl_Position = dzyMVPMatrix * vec4(dzyVertexPosition, 1.0);\n"
"    vVertexColor = dzyVertexColor;\n"
"}\n";

static const char FRAGMENT_simple_vertex_color[] =
"#version 300 es\n"
"precision mediump float;\n"
"in vec3 vVertexColor;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"    fragColor = vec4(vVertexColor, 1.0);\n"
"}\n";

static const char VERTEX_simple_constant_color[] =
"#version 300 es\n"
"uniform mat4 dzyMVPMatrix;\n"
"in vec3 dzyVertexPosition;\n"
"void main() {\n"
"    gl_Position = dzyMVPMatrix * vec4(dzyVertexPosition, 1.0);\n"
"}\n";

static const char FRAGMENT_simple_constant_color[] =
"#version 300 es\n"
"precision mediump float;\n"
"uniform vec3 dzyConstantColor;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"    fragColor = vec4(dzyConstantColor, 1.0);\n"
"}\n";

/// built-in shaders, mvp & vertex position are mandatory
static struct ShaderTable {
    const char *technique;
    bool        hasGeometry;
    bool        requireVertexColor;
    bool        requireVertexNormal;
    bool        requireMaterial;
    bool        requireLight;
    const char *vertexSrc;
    int         vertexLen;
    const char *fragmentSrc;
    int         fragmentLen;
    const char *gometrySrc;
    int         geometryLen;
} builtInShaderTable[] = {
#define SHADER_DEF(NAME, VC, VN, MT, LT) {  \
        #NAME,                              \
        false,                              \
        VC,                                 \
        VN,                                 \
        MT,                                 \
        LT,                                 \
        VERTEX_ ##NAME,                     \
        sizeof(VERTEX_ ##NAME),             \
        FRAGMENT_ ##NAME,                   \
        sizeof(FRAGMENT_ ##NAME),           \
        NULL,                               \
        0                                   \
    }

#define SHADER_DEF2(NAME, VC, VN, MT, LT) { \
        #NAME,                              \
        true,                               \
        VC,                                 \
        VN,                                 \
        MT,                                 \
        LT,                                 \
        VERTEX_ ##NAME,                     \
        sizeof(VERTEX_ ##NAME),             \
        FRAGMENT_ ##NAME,                   \
        sizeof(FRAGMENT_ ##NAME),           \
        GEOMETRY_ ##NAME,                   \
        sizeof(GEOMETRY_ ##NAME)            \
    }

#define SHADER_DEF_END() \
    {NULL, false, false, false, false, false, NULL, 0, NULL, 0, NULL, 0}

    //SHADER_DEF("per_pixel_shading",                   false,  true,   true,   true),
    //SHADER_DEF("normal_mapping",                      false,  true,   true,   true),
    //SHADER_DEF("gloss_mapping",                       false,  true,   true,   true),
    //SHADER_DEF("glow_mapping",                        false,  true,   true,   true),
    //SHADER_DEF("high_dynamic_range_shading",          false,  true,   true,   true),
    //SHADER_DEF("cartoon_shading",                     false,  true,   true,   true),
    //SHADER_DEF("simple_material_texture_light",       false,  false,  true,   true),
    //SHADER_DEF("simple_vertex_color_light",           true,   false,  false,  true),
    //SHADER_DEF("simple_material_texture",             false,  false,  true,   false),
    SHADER_DEF(simple_vertex_color,         true, false, false, false),
    SHADER_DEF(simple_constant_color,       false, false, false, false),
    SHADER_DEF_END()

#undef SHADER_DEF
#undef SHADER_DEF2
#undef SHADER_DEF_END
};

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

void Program::setRequirement(
            bool requireVertexColor,
            bool requireVertexNormal,
            bool requireMaterial,
            bool requireLight) {
    if (requireVertexColor)     mRequirement |= 1 << REQUIRE_VERTEX_COLOR;
    else                        mRequirement &= ~(1 << REQUIRE_VERTEX_COLOR);
    if (requireVertexNormal)    mRequirement |= 1 << REQUIRE_VERTEX_NORMAL;
    else                        mRequirement &= ~(1 << REQUIRE_VERTEX_NORMAL);
    if (requireMaterial)        mRequirement |= 1 << REQUIRE_MATERIAL;
    else                        mRequirement &= ~(1 << REQUIRE_MATERIAL);
    if (requireLight)           mRequirement |= 1 << REQUIRE_LIGHT;
    else                        mRequirement &= ~(1 << REQUIRE_LIGHT);
}

bool Program::hasRequirement(Requirement requirement) {
    return (mRequirement & (1 << requirement)) != 0;
}

bool ProgramManager::preCompile(shared_ptr<EngineContext> engineContext) {
    for (int i=0; builtInShaderTable[i].technique; i++) {
        shared_ptr<Shader> vtxShader(new Shader(Shader::Vertex));
        if (!vtxShader->compileFromMemory(builtInShaderTable[i].vertexSrc, builtInShaderTable[i].vertexLen)) {
            ALOGE("error compile vertex shader");
            return false;
        }
        shared_ptr<Shader> fragShader(new Shader(Shader::Fragment));
        if (!fragShader->compileFromMemory(builtInShaderTable[i].fragmentSrc, builtInShaderTable[i].fragmentLen)) {
            ALOGE("error compile fragment shader");
            return false;
        }
        shared_ptr<Program> program(new Program);
        if (!program->link(vtxShader, fragShader)) {
            ALOGE("error link program");
            return false;
        }
        program->setRequirement(
            builtInShaderTable[i].requireVertexColor,
            builtInShaderTable[i].requireVertexNormal,
            builtInShaderTable[i].requireMaterial,
            builtInShaderTable[i].requireLight);
        program->use();
        program->storeLocation();

        mPrograms.push_back(program);
    }

    return true;
}

shared_ptr<Program> ProgramManager::getCompatibleProgram(
    shared_ptr<Material> material, bool hasLight, shared_ptr<Mesh> mesh) {
    for (auto it = mPrograms.begin(); it != mPrograms.end(); it++) {
        shared_ptr<Program> program(*it);
        bool compatible = isCompatible(mesh->hasVertexColors(),
            program->hasRequirement(Program::REQUIRE_VERTEX_COLOR));
        if (!compatible) continue;
        compatible = compatible && isCompatible(mesh->hasVertexNormals(),
            program->hasRequirement(Program::REQUIRE_VERTEX_NORMAL));
        if (!compatible) continue;
        compatible = compatible && isCompatible(material != nullptr,
            program->hasRequirement(Program::REQUIRE_MATERIAL));
        if (!compatible) continue;
        // FIXME: deal with the case that program has no light,
        // but scene has light in it, simple ignore and pass
        // compatibility check for now.
        //compatible = compatible && isCompatible(hasLight,
        //    program->hasRequirement(Program::REQUIRE_LIGHT));
        if (compatible) return program;
    }
    return nullptr;
}

bool ProgramManager::isCompatible(bool b1, bool b2) {
    return (b1 && b2) || (!b1 && !b2);
}

} //namespace dzy
