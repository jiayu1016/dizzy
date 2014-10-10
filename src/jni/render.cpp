#include <memory>
#include <functional>
#include "log.h"
#include "scene.h"
#include "render.h"

using namespace std;

namespace dzy {

static const char VERTEX_SHADER[] =
    "#version 300 es\n"
    "in vec4 aPos;\n"
    "in vec4 aColor;\n"
    "uniform mat4 uMVP;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    gl_Position = uMVP * aPos;\n"
    "    vColor = aColor;\n"
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
    : mLinked(false)
    , mProgramId(0) {
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

bool Program::load(std::shared_ptr<Scene> scene) {
    if (scene->atLeastOneMeshHasVertexPosition()) {
        GLint attribLoc = getAttrib("aPos");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex position");
            return false;
        }
        mLocations["aPos"] = attribLoc;
    }
    if (scene->atLeastOneMeshHasVertexColor()) {
        GLint attribLoc = getAttrib("aColor");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex color");
            return false;
        }
        mLocations["aColor"] = attribLoc;
    }

    GLint uniformLoc = getUniform("uMVP");
    if (uniformLoc == -1) {
        ALOGE("shader not compatible with scene: mvp matrix");
        return false;
    }
    mLocations["aColor"] = uniformLoc;

    // TODO: check other attributes

    size_t numMeshes = scene->getNumMeshes();
    mVBOs.resize(numMeshes);
    for(size_t i = 0; i < numMeshes; ++i) {
        // Load vertex data into buffer object
        shared_ptr<Mesh> mesh(scene->mMeshes[i]);
        glGenBuffers(1, &mVBOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, mVBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, mesh->getVertexBufSize(),
            mesh->getVertexBuf(), GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

GLint Program::getAttrib(const char* name) const {
    if (!isValid()) return -1;
    return glGetAttribLocation(mProgramId, name);
}

GLint Program::getUniform(const char* name) const {
    if (!isValid()) return -1;
    return glGetUniformLocation(mProgramId, name);
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
    if (!program->link(vtxShader, fragShader)) {
        ALOGE("error link program");
        return false;
    }
    if (!program->load(scene)) {
        ALOGE("error load data");
        return false;
    }
    mProgram = program;
    return true;
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
    //ALOGD("Node %s has %d meshes", node->mName.c_str(), node->mMeshes.size());
    for (size_t i=0; i<node->mMeshes.size(); i++) {
        int meshIdx = node->mMeshes[i];
        //ALOGD("meshIdx: %d", meshIdx);
        shared_ptr<Mesh> mesh(scene->mMeshes[meshIdx]);
        //ALOGD("mesh name: %s", mesh->mName.c_str());
        drawMesh(mesh);
    }
}

void Render::drawMesh(shared_ptr<Mesh> mesh) {
}

} // namespace dzy
