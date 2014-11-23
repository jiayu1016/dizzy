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

int Program::count = 0;
Program::Program()
    : mLinked(false)
    , mProgramId(0)
    , mRequirement(0) {
    count++;
    ALOGD("Program::Program(), count: %d", count);
    mProgramId = glCreateProgram();
    if (!mProgramId) {
        ALOGE("glCreateProgram error");
        return;
    }
}

Program::~Program() {
    --count;
    ALOGD("Program::~Program(), count: %d", count);
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

bool Program::uploadData(
    shared_ptr<Camera> camera,
    shared_ptr<Light> light,
    shared_ptr<Material> material,
    glm::mat4& world,
    glm::mat4& view,
    glm::mat4& proj) {
    ALOGE("subclass should implement uploadData");
    return false;
}

bool Program::updateMeshData(shared_ptr<Mesh> mesh, GLuint vbo) {
    ALOGE("subclass should implement updateMeshData");
    return false;
}

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

Program000::Program000() {
    setRequirement(false, false, false, false);
}

bool Program000::uploadData(
    shared_ptr<Camera> camera,
    shared_ptr<Light> light,
    shared_ptr<Material> material,
    glm::mat4& world,
    glm::mat4& view,
    glm::mat4& proj) {
    glm::mat4 mvp = proj * view * world;
    glUniformMatrix4fv(getLocation("dzyMVPMatrix"), 1, GL_FALSE, glm::value_ptr(mvp));
    // TODO: find a way to determine constant color
    glUniform3f(getLocation("dzyConstantColor"), 0.f, 0.f, 0.f);
    return true;
}

bool Program000::updateMeshData(shared_ptr<Mesh> mesh, GLuint vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    if (mesh->hasVertexPositions()) {
        GLint posLoc = getLocation("dzyVertexPosition");
        glEnableVertexAttribArray(posLoc);
        //ALOGD("num vertices: %d, num components: %d, stride: %d, offset: %d",
        //    mesh->getNumVertices(),
        //    mesh->getPositionNumComponent(),
        //    mesh->getPositionBufStride(),
        //    mesh->getPositionOffset());
        //mesh->dumpBuf((float *)mesh->getPositionBuf(), mesh->getPositionBufSize());
        glVertexAttribPointer(
            posLoc,
            mesh->getPositionNumComponent(),// size
            GL_FLOAT,                       // type
            GL_FALSE,                       // normalized
            mesh->getPositionBufStride(),   // stride, 0 means tightly packed
            (void*)mesh->getPositionOffset()// offset
        );
    }

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

Program010::Program010() {
    setRequirement(true, false, false, false);
}

bool Program010::uploadData(
    shared_ptr<Camera> camera,
    shared_ptr<Light> light,
    shared_ptr<Material> material,
    glm::mat4& world,
    glm::mat4& view,
    glm::mat4& proj) {
    glm::mat4 mvp = proj * view * world;
    glUniformMatrix4fv(getLocation("dzyMVPMatrix"), 1, GL_FALSE, glm::value_ptr(mvp));
    return true;
}

bool Program010::updateMeshData(shared_ptr<Mesh> mesh, GLuint vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    if (mesh->hasVertexPositions()) {
        GLint posLoc = getLocation("dzyVertexPosition");
        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(
            posLoc,
            mesh->getPositionNumComponent(),// size
            GL_FLOAT,                       // type
            GL_FALSE,                       // normalized
            mesh->getPositionBufStride(),   // stride, 0 means tightly packed
            (void*)mesh->getPositionOffset()// offset
        );
    }
    if (mesh->hasVertexColors()) {
        GLint colorLoc = getLocation("dzyVertexColor");
        glEnableVertexAttribArray(colorLoc);
        glVertexAttribPointer(
            colorLoc,
            mesh->getColorNumComponent(0),
            GL_FLOAT,
            GL_FALSE,
            mesh->getColorBufStride(0),
            (void*)mesh->getColorOffset(0)
        );
    }

    return true;
}

static const char VERTEX_simple_material[] =
"#version 300 es\n"
"uniform mat4 dzyMVPMatrix;\n"
"in vec3 dzyVertexPosition;\n"
"void main() {\n"
"    gl_Position = dzyMVPMatrix * vec4(dzyVertexPosition, 1.0);\n"
"}\n";

static const char FRAGMENT_simple_material[] =
"#version 300 es\n"
"precision mediump float;\n"
"struct Material {\n"
"    vec3 diffuse;\n"
"    vec3 ambient;\n"
"};\n"
"uniform Material dzyMaterial;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"    vec3 color = dzyMaterial.diffuse + dzyMaterial.ambient;\n"
"    fragColor = vec4(color, 1.0);\n"
"}\n";

Program020::Program020() {
    setRequirement(false, false, true, false);
}

bool Program020::uploadData(
    shared_ptr<Camera> camera,
    shared_ptr<Light> light,
    shared_ptr<Material> material,
    glm::mat4& world,
    glm::mat4& view,
    glm::mat4& proj) {
    glm::mat4 mvp = proj * view * world;
    glUniformMatrix4fv(getLocation("dzyMVPMatrix"), 1, GL_FALSE, glm::value_ptr(mvp));
    glm::vec3 diffuse = material->getDiffuse();
    glm::vec3 ambient = material->getAmbient();
    glUniform3fv(getLocation("dzyMaterial.diffuse"), 1, glm::value_ptr(diffuse));
    glUniform3fv(getLocation("dzyMaterial.ambient"), 1, glm::value_ptr(ambient));
    return true;
}

bool Program020::updateMeshData(shared_ptr<Mesh> mesh, GLuint vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    if (mesh->hasVertexPositions()) {
        GLint posLoc = getLocation("dzyVertexPosition");
        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(
            posLoc,
            mesh->getPositionNumComponent(),// size
            GL_FLOAT,                       // type
            GL_FALSE,                       // normalized
            mesh->getPositionBufStride(),   // stride, 0 means tightly packed
            (void*)mesh->getPositionOffset()// offset
        );
    }
/*
    if (mesh->hasVertexColors()) {
        GLint colorLoc = getLocation("dzyVertexColor");
        glEnableVertexAttribArray(colorLoc);
        glVertexAttribPointer(
            colorLoc,
            mesh->getColorNumComponent(0),
            GL_FLOAT,
            GL_FALSE,
            mesh->getColorBufStride(0),
            (void*)mesh->getColorOffset(0)
        );
    }

    if (mesh->hasVertexNormals()) {
        GLint normalLoc = getLocation("dzyVertexNormal");
        glEnableVertexAttribArray(normalLoc);
        glVertexAttribPointer(
            normalLoc,
            mesh->getNormalNumComponent(),  // size
            GL_FLOAT,                       // type
            GL_FALSE,                       // normalized
            mesh->getNormalBufStride(),     // stride, 0 means tightly packed
            (void *)mesh->getNormalOffset() // offset
        );
    }
*/
    return true;
}

static const char VERTEX_simple_vertex_color_light[] =
"#version 300 es\n"
"uniform mat4 dzyMVPMatrix;\n"
"uniform mat4 dzyMVMatrix;\n"
"uniform mat3 dzyNormalMatrix;\n"
"in vec3 dzyVertexPosition;\n"
"in vec3 dzyVertexColor;\n"
"in vec3 dzyVertexNormal;\n"
"out vec3 vVertexPositionEyeSpace;\n"
"out vec3 vVertexNormalEyeSpace;\n"
"void main() {\n"
"    gl_Position = dzyMVPMatrix * vec4(dzyVertexPosition, 1.0);\n"
"    vVertexPositionEyeSpace = vec3(dzyMVMatrix * vec4(dzyVertexPosition, 1.0));\n"
"    vVertexNormalEyeSpace = dzyNormalMatrix * dzyVertexNormal;\n"
"}\n";

static const char FRAGMENT_simple_vertex_color_light[] =
"#version 300 es\n"
"precision mediump float;\n"
"in vec3 vVertexPositionEyeSpace;\n"
"in vec3 vVertexNormalEyeSpace;\n"
"struct PointLight {\n"
"    vec3 color;\n"
"    vec3 ambient;\n"
"    vec3 position;\n"
"    float attenuationConstant;\n"
"    float attenuationLinear;\n"
"    float attenuationQuadratic;\n"
"    float strength;\n"
"};\n"
"uniform PointLight dzyLight;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"    fragColor = vec4(vVertexColor, 1.0);\n"
"}\n";

/// built-in shaders, mvp & vertex position are mandatory
ProgramManager::ProgramTable ProgramManager::builtInProgramTable[] = {
#define PROG_TBL_ENTRY_DEF(NAME) {                  \
        #NAME,                                      \
        false,                                      \
        VERTEX_ ##NAME,                             \
        sizeof(VERTEX_ ##NAME),                     \
        FRAGMENT_ ##NAME,                           \
        sizeof(FRAGMENT_ ##NAME),                   \
        NULL,                                       \
        0                                           \
    }

#define PROG_TBL_ENTRY_DEF2(NAME) {                 \
        #NAME,                                      \
        true,                                       \
        VERTEX_ ##NAME,                             \
        sizeof(VERTEX_ ##NAME),                     \
        FRAGMENT_ ##NAME,                           \
        sizeof(FRAGMENT_ ##NAME),                   \
        GEOMETRY_ ##NAME,                           \
        sizeof(GEOMETRY_ ##NAME)                    \
    }

#define PROG_TBL_ENTRY_DEF_END() {NULL, false, NULL, 0, NULL, 0, NULL, 0}

    //PROG_TBL_ENTRY_DEF("per_pixel_shading",                   false,  true,   true,   true),
    //PROG_TBL_ENTRY_DEF("normal_mapping",                      false,  true,   true,   true),
    //PROG_TBL_ENTRY_DEF("gloss_mapping",                       false,  true,   true,   true),
    //PROG_TBL_ENTRY_DEF("glow_mapping",                        false,  true,   true,   true),
    //PROG_TBL_ENTRY_DEF("high_dynamic_range_shading",          false,  true,   true,   true),
    //PROG_TBL_ENTRY_DEF("cartoon_shading",                     false,  true,   true,   true),
    //PROG_TBL_ENTRY_DEF("simple_material_light",       false,  false,  true,   true),
    PROG_TBL_ENTRY_DEF(simple_material),
    PROG_TBL_ENTRY_DEF(simple_vertex_color),
    PROG_TBL_ENTRY_DEF(simple_constant_color),
    PROG_TBL_ENTRY_DEF_END()

#undef PROG_TBL_ENTRY_DEF
#undef PROG_TBL_ENTRY_DEF2
#undef PROG_TBL_ENTRY_DEF_END
};

bool ProgramManager::preCompile(shared_ptr<EngineContext> engineContext) {
    for (int i=0; builtInProgramTable[i].technique; i++) {
        shared_ptr<Shader> vtxShader(new Shader(Shader::Vertex));
        if (!vtxShader->compileFromMemory(builtInProgramTable[i].vertexSrc, builtInProgramTable[i].vertexLen)) {
            ALOGE("error compile vertex shader");
            return false;
        }
        shared_ptr<Shader> fragShader(new Shader(Shader::Fragment));
        if (!fragShader->compileFromMemory(builtInProgramTable[i].fragmentSrc, builtInProgramTable[i].fragmentLen)) {
            ALOGE("error compile fragment shader");
            return false;
        }
        shared_ptr<Program> program(createProgram(builtInProgramTable[i].technique));
        if (!program->link(vtxShader, fragShader)) {
            ALOGE("error link program");
            return false;
        }

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

shared_ptr<Program> ProgramManager::createProgram(const string& name) {
    if (name == "simple_material")
        return shared_ptr<Program>(new Program020);
    if (name == "simple_vertex_color")
        return shared_ptr<Program>(new Program010);
    if (name == "simple_constant_color")
        return shared_ptr<Program>(new Program000);

    return nullptr;
}

bool ProgramManager::isCompatible(bool b1, bool b2) {
    return (b1 && b2) || (!b1 && !b2);
}

} //namespace dzy
