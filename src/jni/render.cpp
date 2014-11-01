#include <GLES3/gl3.h>
#include "log.h"
#include "engine_context.h"
#include "scene.h"
#include "program.h"
#include "utils.h"
#include "scene_graph.h"
#include "mesh.h"
#include "render.h"

using namespace std;

namespace dzy {

Render::Render() {
    ALOGD("Render::Render()");
}

Render::~Render() {
    ALOGD("Render::~Render()");
}

bool Render::init() {
    shared_ptr<EngineContext> engineContext(getEngineContext());
    if (!engineContext) {
        ALOGE("EngineContext released while init Render");
        return false;
    }

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.6f, 0.7f, 1.0f, 1.0f);

    glViewport(0, 0,
        engineContext->getSurfaceWidth(),
        engineContext->getSurfaceHeight());

    return true;
}

bool Render::release() {
    return true;
}

bool Render::drawScene(shared_ptr<Scene> scene) {
    shared_ptr<EngineContext> engineContext(getEngineContext());
    if (!engineContext) {
        ALOGE("EngineContext released while rendering a scene");
        return false;
    }

    shared_ptr<Node> rootNode = scene->getRootNode();
    if (!rootNode) {
        ALOGW("Scene graph empty, nothing to render");
        return false;
    }

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    scene->mCameraModelTransform = glm::mat4(1.0f);

    shared_ptr<Camera> activeCamera(scene->getActiveCamera());
    if (activeCamera) {
        shared_ptr<Node> node(rootNode->findNode(activeCamera->mName));
        if (node) {
            glm::mat4 transform = glm::mat4(1.0f);
            while(node != rootNode) {
                transform = node->mTransformation * transform;
                node = node->getParent();
            }
            transform = node->mTransformation * transform;
            scene->mCameraModelTransform = transform;
        }

        scene->mLightModelTransform = glm::mat4(1.0f);

        // Support lighing only when we have a camera
        // Use only one light right now
        if (scene->getNumLights() > 0) {
            shared_ptr<Light> light(scene->mLights[0]);
            if (light) {
                shared_ptr<Node> node(rootNode->findNode(light->mName));
                if (node) {
                    glm::mat4 trans = glm::mat4(1.0f);
                    while(node != rootNode) {
                        trans = node->mTransformation * trans;
                        node = node->getParent();
                    }
                    trans = node->mTransformation * trans;
                    scene->mLightModelTransform = trans;
                }
            }
        }
    }

    rootNode->draw(*this, scene);
    eglSwapBuffers(engineContext->getEGLDisplay(), engineContext->getEGLSurface());

    return true;
}

void Render::drawNode(shared_ptr<Scene> scene, shared_ptr<Node> node) {
    if (!node) return;

    shared_ptr<Node> rootNode(scene->getRootNode());
    assert(rootNode);
    glm::mat4 world = glm::mat4(1.0f);
    shared_ptr<Node> nd(node);
    while(nd != rootNode) {
        world = nd->mTransformation * world;
        nd = nd->getParent();
    }
    world = nd->mTransformation * world;
    glm::mat4 mvp = world;

    node->getProgram()->use();
    //Utils::dump(node->mName.c_str(), node->mTransformation);
    shared_ptr<Camera> activeCamera(scene->getActiveCamera());
    if (activeCamera) {
        //override the aspect read from model
        float surfaceWidth = getEngineContext()->getSurfaceWidth();
        float surfaceHeight = getEngineContext()->getSurfaceHeight();
        activeCamera->setAspect(surfaceWidth/surfaceHeight);
        glm::mat4 view = activeCamera->getViewMatrix(scene->mCameraModelTransform);
        glm::mat4 proj = activeCamera->getProjMatrix();
        glm::mat4 mv = view * world;
        mvp = proj * mv;

        GLint mvLoc = node->getProgram()->getLocation("dzyMVMatrix");
        if (mvLoc != -1) {
            glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mv));
        }
        GLint normalMatrixLoc = node->getProgram()->getLocation("dzyNormalMatrix");
        if (normalMatrixLoc != -1) {
            glm::mat3 mvInvTransMatrix = glm::mat3(glm::transpose(glm::inverse(mv)));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(mvInvTransMatrix));
        }

        glUniformMatrix4fv(node->getProgram()->getLocation("dzyMVPMatrix"), 1, GL_FALSE, glm::value_ptr(mvp));

        if (scene->getNumLights() > 0) {
            shared_ptr<Light> light(scene->mLights[0]);
            if (light) {
                glUniform3fv(node->getProgram()->getLocation("dzyLight.color"),
                    1, glm::value_ptr(light->mColorDiffuse));
                glUniform3fv(node->getProgram()->getLocation("dzyLight.ambient"),
                    1, glm::value_ptr(light->mColorAmbient));
                glm::vec3 lightPosEyeSpace = glm::vec3(
                    view * scene->mLightModelTransform * glm::vec4(light->mPosition, 1.0f));
                glUniform3fv(node->getProgram()->getLocation("dzyLight.position"),
                    1, glm::value_ptr(lightPosEyeSpace));
                glUniform1f(node->getProgram()->getLocation("dzyLight.attenuationConstant"),
                    light->mAttenuationConstant);
                glUniform1f(node->getProgram()->getLocation("dzyLight.attenuationLinear"),
                    light->mAttenuationLinear);
                glUniform1f(node->getProgram()->getLocation("dzyLight.attenuationQuadratic"),
                    light->mAttenuationQuadratic);
                glUniform1f(node->getProgram()->getLocation("dzyLight.strength"), 1.0f);
            }
        }
    }
}

void Render::drawMesh(shared_ptr<Scene> scene, shared_ptr<Mesh> mesh,
    shared_ptr<Program> program, GLuint vbo, GLuint ibo) {
    // bind the BOs
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // TODO: move material to node, not mesh
    if (scene->getNumMaterials() > 0) {
        shared_ptr<Material> material(scene->mMaterials[mesh->mMaterialIndex]);
        glm::vec3 diffuse, specular, ambient, emission;
        float shininess;
        material->get(Material::COLOR_DIFFUSE, diffuse);
        material->get(Material::COLOR_SPECULAR, specular);
        material->get(Material::COLOR_AMBIENT, ambient);
        material->get(Material::COLOR_EMISSION, emission);
        material->get(Material::SHININESS, shininess);

        glUniform3fv(program->getLocation("dzyMaterial.diffuse"),
            1, glm::value_ptr(diffuse));
        glUniform3fv(program->getLocation("dzyMaterial.specular"),
            1, glm::value_ptr(specular));
        glUniform3fv(program->getLocation("dzyMaterial.ambient"),
            1, glm::value_ptr(ambient));
        glUniform3fv(program->getLocation("dzyMaterial.emission"),
            1, glm::value_ptr(emission));
        glUniform1f(program->getLocation("dzyMaterial.shininess"), shininess);

    }
    if (mesh->hasVertexPositions()) {
        GLint posLoc = program->getLocation("dzyVertexPosition");
        glEnableVertexAttribArray(posLoc);
        //ALOGD("num vertices: %d, num components: %d, stride: %d",
        //    mesh->getNumVertices(),
        //    mesh->getPositionNumComponent(),
        //    mesh->getPositionBufStride());
        //mesh->dumpVertexPositionBuf();
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
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        GLint colorLoc = program->getLocation("dzyVertexColor");
        glEnableVertexAttribArray(colorLoc);
    }
    if (mesh->hasVertexNormals()) {
        GLint normalLoc = program->getLocation("dzyVertexNormal");
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

    //ALOGD("num indices: %d, indices buf size: %d",
    //    mesh->getNumIndices(), mesh->getIndexBufSize());
    //mesh->dumpIndexBuf();
    // support only GL_UNSIGNED_INT right now
    glDrawElements(GL_TRIANGLES,            // mode
        mesh->getNumIndices(),              // indices count
        GL_UNSIGNED_INT,                    // type
        (void *)0);                         // offset

    // restore
    if (mesh->hasVertexPositions())
        glDisableVertexAttribArray(program->getLocation("dzyVertexPosition"));

    if (mesh->hasVertexColors()) {
        glDisableVertexAttribArray(program->getLocation("dzyVertexColor"));
    }
    if (mesh->hasVertexNormals()) {
        glDisableVertexAttribArray(program->getLocation("dzyVertexNormal"));
    }

    // unbind the BOs
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

shared_ptr<EngineContext> Render::getEngineContext() {
    return mEngineContext.lock();
}

const char* Render::glStatusStr() {
    GLenum error = glGetError();

    switch (error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        default: return "UNKNOWN_GL_ERROR";
    }
}

void Render::setEngineContext(shared_ptr<EngineContext> engineContext) {
    mEngineContext = engineContext;
}

} // namespace dzy
