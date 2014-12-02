#ifndef CAMERA_H
#define CAMERA_H

#include <string>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "nameobj.h"
#include "transform.h"

namespace dzy {

#define DEFAULT_CAMERA_POS          glm::vec3(7.f, 5.f, 6.f)

class AIAdapter;
class Scene;
class Render;
class Camera : public NameObj {
public:
    Camera(const std::string& name = "");
    Camera(
        glm::vec3           position,
        glm::vec3           up,
        glm::vec3           lookAt,
        float               horizontalFOV,
        float               clipPlaneNear,
        float               clipPlaneFar,
        float               aspect,
        const std::string&  name = "");

    Camera&     translate(float x, float y, float z);
    Camera&     translate(const glm::vec3& offset);
    Camera&     scale(float s);
    Camera&     scale(float x, float y, float z);
    Camera&     scale(const glm::vec3& s);
    Camera&     rotate(const glm::quat& rotation);
    Camera&     rotate(float axisX, float axisY, float axisZ);
    void        setAspect(float aspect);
    glm::mat4   getViewMatrix();
    glm::mat4   getViewMatrix(const glm::mat4& transform);
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
    Transform           mLocalTransform;
};

} //namespace

#endif
