#ifndef RENDER_H
#define RENDER_H

#include <memory>

namespace dzy {

extern const char* glStatusStr();
class EngineContext;
class Scene;
class Node;
class Program;
class Render {
public:
    Render();
    ~Render();
    bool init(std::shared_ptr<Scene> scene);
    bool release();
    bool drawScene(std::shared_ptr<Scene> scene);
    void drawNode(std::shared_ptr<Scene> scene, std::shared_ptr<Node> node);    
    void drawMesh(std::shared_ptr<Scene> scene, int meshIdx);
    std::shared_ptr<EngineContext> getEngineContext();
    static const char* glStatusStr();

    friend class EngineContext;
private:
    void setEngineContext(std::shared_ptr<EngineContext> engineContext);

    std::shared_ptr<Program>        mProgram;
    std::weak_ptr<EngineContext>    mEngineContext;
};

} // namespace dzy 

#endif
