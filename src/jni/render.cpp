#include <memory>
#include <functional>
#include "log.h"
#include "scene.h"
#include "render.h"

using namespace std;

namespace dzy {

static const char VERTEX_SHADER[] =
    "#version 300 es\n"
    "in vec4 pos;\n"
    "in vec4 color;\n"
    "uniform mat4 uMVP;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    gl_Position = uMVP * pos;\n"
    "    vColor = color;\n"
    "}\n";

static const char FRAGMENT_SHADER[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec4 vColor;\n"
    "out vec4 outColor;\n"
    "void main() {\n"
    "    outColor = vColor;\n"
    "}\n";

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

Program::Program()
    : mInitialized(false)
    , mProgramId(0) {
    mProgramId = glCreateProgram();
    if (!mProgramId) {
        ALOGE("glCreateProgram error");
        return;
    }
}

Program::~Program() {
    if (mProgramId)
        glDeleteProgram(mProgramId);
}

void Program::use() {
    glUseProgram(mProgramId);
}

bool Program::link(std::shared_ptr<Shader> vtxShader, std::shared_ptr<Shader> fragShader) {
    if (mInitialized) {
        ALOGW("Program already initialized");
        return false;
    }

    // vertex always slot 0, fragment always slot 1
    mShaders[0] = vtxShader;
    mShaders[1] = fragShader;

    // in order to support other shaders in future
    for (auto iter = mShaders.begin(); iter != mShaders.end(); iter++) {
        if (*iter)
            glAttachShader(mProgramId, (*iter)->getShaderID());
    }

    if (!ndk_helper::shader::LinkProgram(mProgramId)) {
        ALOGE("error link program");
        return false;
    }

    if (!ndk_helper::shader::ValidateProgram(mProgramId)) {
        ALOGE("program validation fail");
        return false;
    }

    mInitialized = true;

    return true;
}

bool Program::load(std::shared_ptr<Scene> scene) {

    return true;
}

GLuint Program::getAttrib(const char* name) const {
    return 0;
}

GLint Program::getUniform(const char* name) const {
    return 0;
}

bool Render::init(shared_ptr<Scene> scene) {
    shared_ptr<Shader> vtxShader(new Shader(GL_VERTEX_SHADER));
    if (!vtxShader->compileFromMemory(VERTEX_SHADER,
            sizeof(VERTEX_SHADER))) {
        ALOGE("error compile vertex shader");
        return false;
    }
    shared_ptr<Shader> fragShader(new Shader(GL_FRAGMENT_SHADER));
    if (!fragShader->compileFromMemory(FRAGMENT_SHADER,
        sizeof(FRAGMENT_SHADER))) {
        ALOGE("error compile fragment shader");
        return false;
    }
    shared_ptr<Program> program(new Program());
    if (!mProgram->link(vtxShader, fragShader)) {
        ALOGE("error link program");
        return false;
    }
    if (!mProgram->load(scene)) {
        ALOGE("error load data");
        return false;
    }
}

bool Render::release() {
    return true;
}

bool Render::drawScene(shared_ptr<Scene> scene) {
    NodeTree &tree = scene->getNodeTree();
    using namespace std::placeholders;
    tree.dfsTraversal(scene, bind(&Render::drawNode, this, _1, _2));
    return true;
}

void Render::drawNode(shared_ptr<Scene> scene, shared_ptr<Node> node) {
    ALOGD("Node %s has %d meshes", node->mName.c_str(), node->mMeshes.size());
    for (size_t i=0; i<node->mMeshes.size(); i++) {
        int meshIdx = node->mMeshes[i];
        ALOGD("meshIdx: %d", meshIdx);
        shared_ptr<Mesh> mesh(scene->mMeshes[meshIdx]);
        ALOGD("mesh name: %s", mesh->mName.c_str());
        mesh->draw(*this);
    }
}

} // namespace dzy
