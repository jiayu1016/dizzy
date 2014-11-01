#ifndef MATERIAL_H
#define MATERIAL_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace dzy {

class Material {
public:
    enum MaterialType {
        COLOR_DIFFUSE,
        COLOR_SPECULAR,
        COLOR_AMBIENT,
        COLOR_EMISSION,
        SHININESS,
    };
    bool get(MaterialType type, glm::vec3& color);
    bool get(MaterialType type, float& value);

    friend class AIAdapter;
private:
    glm::vec3   mDiffuse;
    glm::vec3   mSpecular;
    glm::vec3   mAmbient;
    glm::vec3   mEmission;
    float       mShininess;
};

} // namespace

#endif
