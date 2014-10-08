#ifndef RENDER_H
#define RENDER_H

#include <memory>
#include <vector>
#include "utils.h"

namespace dzy {

class Render {

};

class RenderManager : public Singleton<RenderManager> {
public:
    std::shared_ptr<Render> createDefaultRender() {
        std::shared_ptr<Render> r(new Render);
        mCurrentRender = r;
        return r;
    }

    std::shared_ptr<Render> getCurrentRender() {
        return mCurrentRender;
    }

private:
    std::vector<std::shared_ptr<Render> >   mRenders;
    std::shared_ptr<Render>                 mCurrentRender;
};

} // namespace dzy 

#endif
