#include <GLES3/gl3.h>
#include "log.h"
#include "engine_context.h"
#include "scene.h"
#include "program.h"
#include "utils.h"
#include "scene_graph.h"
#include "mesh.h"
#include "material.h"
#include "camera.h"
#include "light.h"
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
        shared_ptr<NodeObj> node(rootNode->getChild(activeCamera->mName));
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

        // Support lighting only when we have a camera
        // Use only one light right now
        if (scene->getNumLights() > 0) {
            shared_ptr<Light> light(scene->mLights[0]);
            if (light) {
                shared_ptr<NodeObj> node(rootNode->getChild(light->mName));
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
}

bool Render::drawGeometry(shared_ptr<Scene> scene, shared_ptr<Geometry> geometry) {
    if (!geometry || !scene) {
        ALOGE("Invalid argument");
        return false;
    }

    shared_ptr<Node> rootNode(scene->getRootNode());
    assert(rootNode);
    glm::mat4 world = glm::mat4(1.0f);
    shared_ptr<NodeObj> nd(geometry);
    while(nd != rootNode) {
        world = nd->mTransformation * world;
        nd = nd->getParent();
    }
    world = nd->mTransformation * world;
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);

    shared_ptr<Material> material(geometry->getMaterial());
    shared_ptr<Program> currentProgram(
        geometry->getProgram(material, scene->getNumLights() > 0, geometry->getMesh()));
    if (!currentProgram) {
        //ALOGE("No built-in program generated for material: %s, mesh: %s",
        //    material ? material->getName().c_str() : "NULL", geometry->getMesh()->getName().c_str());
        return false;
    }
    currentProgram->use();

    shared_ptr<Camera> activeCamera(scene->getActiveCamera());
    if (activeCamera) {
        //override the aspect read from model
        float surfaceWidth = getEngineContext()->getSurfaceWidth();
        float surfaceHeight = getEngineContext()->getSurfaceHeight();
        activeCamera->setAspect(surfaceWidth/surfaceHeight);
        view = activeCamera->getViewMatrix(scene->mCameraModelTransform);
        proj = activeCamera->getProjMatrix();
    }

    shared_ptr<Light> light(scene->getLight(0));
    light->setTransform(scene->mLightModelTransform);
    currentProgram->uploadData(activeCamera, light, material, world, view, proj);
    return true;
}

void Render::drawMesh(shared_ptr<Scene> scene, shared_ptr<Mesh> mesh,
    shared_ptr<Program> program, GLuint vbo, GLuint ibo) {
    program->updateMeshData(mesh, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    // support only GL_UNSIGNED_INT right now
    glDrawElements(GL_TRIANGLES,            // mode
        mesh->getNumIndices(),              // indices count
        GL_UNSIGNED_INT,                    // type
        (void *)0);                         // offset
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
