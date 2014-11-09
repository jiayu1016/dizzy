#ifndef MATERIAL_H
#define MATERIAL_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "shader_generator.h"

namespace dzy {

class Material {
public:
    Material(const std::string& name = "");
    Material(const Material& rhs);
    Material& operator =(const Material& rhs);
    ~Material();

    static std::shared_ptr<Material> getDefault();

    bool hasAmbient() const;
    glm::vec3 getAmbient() const;
    void setAmbient(const glm::vec3& color);
    void clearAmbient();

    bool hasDiffuse() const;
    glm::vec3 getDiffuse() const;
    void setDiffuse(const glm::vec3& color);
    void clearDiffuse();

    bool hasSpecular() const;
    glm::vec3 getSpecular() const;
    void setSpecular(const glm::vec3& color);
    void clearSpecular();

    bool hasEmission() const;
    glm::vec3 getEmission() const;
    void setEmission(const glm::vec3& color);
    void clearEmission();

    float getShininess() const;
    void setShininess(float shininess);

    std::string getName();
    void setName(const std::string& name);

private:
    enum Flags {
        F_AMBIENT     = 0x001,
        F_DIFFUSE     = 0x002,
        F_SPECULAR    = 0x004,
        F_EMISSION    = 0x008,
    };

    std::string mName;
    glm::vec3   mDiffuse;
    glm::vec3   mSpecular;
    glm::vec3   mAmbient;
    glm::vec3   mEmission;
    float       mShininess;
    int         mFlags;
};

} // namespace

#endif
