#ifndef CAMERA_H
#define CAMERA_H

#include <string>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "nameobj.h"
#include "transform.h"

namespace dzy {

#define DEFAULT_CAMERA_POS          glm::vec3(0.f, 0.f, 10.f)
#define DEFAULT_CAMERA_CENTER       glm::vec3(0.f, 0.f, 0.f)
#define DEFAULT_CAMERA_UP           glm::vec3(0.f, 1.f, 0.f)

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
    Camera&     pitch(float angle);
    Camera&     yaw(float angle);
    Camera&     roll(float angle);
    void        setAspect(float aspect);
    float       getAspect();
    void        setHorizontalFOV(float fov);
    float       getHorizontalFOV();
    void        setNearPlane(float near);
    float       getNearPlane();
    void        setFarPlane(float far);
    float       getFarPlane();
    void        setPostion(const glm::vec3& pos);
    glm::vec3   getPosition();
    void        setUp(const glm::vec3& up);
    glm::vec3   getUp();
    void        setLookAt(const glm::vec3& at);
    glm::vec3   getLookAt();
    glm::mat4   getViewMatrix();
    glm::mat4   getProjMatrix();

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
