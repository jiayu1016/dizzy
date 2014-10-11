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
        GLint attribLoc = glGetAttribLocation(mProgramId, "aPos");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex position");
            return false;
        }
        mLocations["aPos"] = attribLoc;
    }
    if (scene->atLeastOneMeshHasVertexColor()) {
        GLint attribLoc = glGetAttribLocation(mProgramId, "aColor");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex color");
            return false;
        }
        mLocations["aColor"] = attribLoc;
    }

    GLint mvpLoc = glGetUniformLocation(mProgramId, "uMVP");
    if (mvpLoc == -1) {
        ALOGE("shader not compatible with scene: mvp matrix");
        return false;
    }
    mLocations["aColor"] = mvpLoc;

    // TODO: check other attributes

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

    glClearColor(0.6f, 0.7f, 1.0f, 1.0f);

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
        shared_ptr<Mesh> mesh(scene->mMeshes[meshIdx]);
        drawMesh(mesh, i);
    }
}

void Render::drawMesh(shared_ptr<Mesh> mesh, int meshIdx) {
    ALOGD("meshIdx: %d, mesh name: %s", meshIdx, mesh->mName.c_str());
    glClear(GL_COLOR_BUFFER_BIT);
    mProgram->use();
    mProgram->bindBufferObjects(meshIdx);
    if (mesh->hasPositions()) {
        GLint posLoc = mProgram->getLocation("aPos");
        ALOGD("aPos attrib loc: %d", posLoc);
        glEnableVertexAttribArray(posLoc);
        ALOGD("num vertices: %d, num components: %d, stride: %d",
            mesh->getNumVertices(),
            mesh->getVertexNumComponent(),
            mesh->getVertexBufStride());
        mesh->dumpVertexBuf();
        glVertexAttribPointer(
            posLoc,
            mesh->getVertexNumComponent(),  // size 
            GL_FLOAT,                       // type 
            GL_FALSE,                       // normalized? 
            mesh->getVertexBufStride(),     // stride 
            (void*)0                        // array buffer offset 
        );
    }
    if (mesh->hasVertexColors()) {
        glEnableVertexAttribArray(mProgram->getLocation("aColor"));
    }
    // TODO: check other attributes in mesh

    ALOGD("num indices: %d", mesh->getNumIndices());
    mesh->dumpIndexBuf();
    // support only GL_UNSIGNED_INT right now
    glDrawElements(GL_TRIANGLES,            // mode
        mesh->getNumIndices(),              // indices count
        GL_UNSIGNED_INT,                    // type
        mesh->getIndexBuf());

    // restore
    if (mesh->hasPositions())
        glDisableVertexAttribArray(mProgram->getLocation("aPos"));

    if (mesh->hasVertexColors()) {
        glDisableVertexAttribArray(mProgram->getLocation("aColor"));
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

} // namespace dzy
