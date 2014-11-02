#ifndef RENDER_H
#define RENDER_H

#include <memory>
#include <GLES3/gl3.h>

namespace dzy {

class EngineContext;
class Scene;
class NodeObj;
class Mesh;
class Program;
class Render {
public:
    Render();
    ~Render();
    bool init();
    bool release();

    /// draw the whole scene
    ///
    ///     this is place where the camera and light model transform is built
    ///     @param scene the scene
    ///     @return true is success, false otherwise
    bool drawScene(std::shared_ptr<Scene> scene);

    /// draw a single node
    ///
    ///     @param scene the scene that hosts the scene graph
    ///     @param the node current being drawn
    void drawNode(std::shared_ptr<Scene> scene, std::shared_ptr<NodeObj> node);

    /// draw a mesh
    ///
    ///     one thing that is worth mentioning is the buffer object, I prefer to
    ///     have Mesh class not dependent on anything gl, so buffer object is
    ///     not contained in Mesh class, but in Node class. Buffer object handle
    ///     must be passed as parameters.
    ///
    ///     @param scene the scene that hosts the scene graph
    ///     @param mesh the mesh holding geometry data is being drawn
    ///     @param vbo the vertex buffer object to hold mesh geometry data
    ///     @param ibo the index buffer object
    void drawMesh(std::shared_ptr<Scene> scene, std::shared_ptr<Mesh> mesh,
        std::shared_ptr<Program> program, GLuint vbo, GLuint ibo);

    std::shared_ptr<EngineContext> getEngineContext();
    static const char* glStatusStr();

    friend class EngineContext;
private:
    void setEngineContext(std::shared_ptr<EngineContext> engineContext);

    std::weak_ptr<EngineContext>    mEngineContext;
};

} // namespace dzy 

#endif
