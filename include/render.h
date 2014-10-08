#ifndef RENDER_H
#define RENDER_H

#include <memory>

namespace dzy {

class Render {

};

class RenderManager {
public:
    static std::shared_ptr<Render> createRender() {
        return std::shared_ptr<Render>(new Render);
    }
};

} // namespace dzy 

#endif
