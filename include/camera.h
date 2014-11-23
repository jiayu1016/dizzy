#ifndef CAMERA_H
#define CAMERA_H

#include <string>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "nameobj.h"

namespace dzy {

class AIAdapter;
class Scene;
class Render;
class Camera : public NameObj {
public:
    Camera();
    Camera(
        glm::vec3           position,
        glm::vec3           up,
        glm::vec3           lookAt,
        float               horizontalFOV,
        float               clipPlaneNear,
        float               clipPlaneFar,
        float               aspect);

    void        setAspect(float aspect);
    glm::mat4   getViewMatrix();
    glm::mat4   getViewMatrix(glm::mat4 transform);
    glm::mat4   getProjMatrix();
    void        dump(glm::vec3 pos, glm::vec3 at, glm::vec3 up);
    void        dumpParameter();

    friend class Render;
    friend class AIAdapter;
private:
    glm::vec3           mPosition;
    glm::vec3           mUp;
    glm::vec3           mLookAt;
    float               mHorizontalFOV;
    float               mClipPlaneNear;
    float               mClipPlaneFar;
    float               mAspect;
};

} //namespace

#endif
