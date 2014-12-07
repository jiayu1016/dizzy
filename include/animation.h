#ifndef ANIMATION_H
#define ANIMATION_H

#include <string>
#include <vector>
#include <memory>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "log.h"
#include "nameobj.h"

namespace dzy {

class VectorState {
public:
    VectorState();
    VectorState(double time, const glm::vec3& value);
    bool operator == (const VectorState& o) const;
    bool operator != (const VectorState& o) const;
    bool operator < (const VectorState& o) const;
    bool operator > (const VectorState& o) const;

    double      getTime() const;
    void        setTime(double time);
    glm::vec3   getValue() const;
    void        setValue(const glm::vec3& value);

private:
    double      mTime;
    glm::vec3   mValue;
};

class RotationState {
public:
    RotationState();
    RotationState(double time, const glm::quat& value);
    bool operator == (const RotationState& o) const;
    bool operator != (const RotationState& o) const;
    bool operator < (const RotationState& o) const;
    bool operator > (const RotationState& o) const;

    double      getTime() const;
    void        setTime(double time);
    glm::quat   getValue() const;
    void        setValue(const glm::quat& value);

private:
    double      mTime;
    glm::quat   mValue;
};

class MeshState  {
public:
    MeshState();
    MeshState(double time, unsigned int value);
    bool operator == (const MeshState& o) const;
    bool operator != (const MeshState& o) const;
    bool operator < (const MeshState& o) const;
    bool operator > (const MeshState& o) const;

    double          getTime() const;
    void            setTime(double time);
    unsigned int    getValue() const;
    void            setValue(unsigned int value);

private:
    double          mTime;
    unsigned int    mValue;
};

class Animation;
/// NodeAnim must have a name to index a unique node in scene graph
class NodeAnim : public NameObj {
public:
    enum Type {
        TYPE_DEFAULT  = 0,
        TYPE_CONSTANT = 1,
        TYPE_LINEAR   = 2,
        TYPE_REPEAT   = 3,
    };
    NodeAnim(const char* name);
    ~NodeAnim();

    glm::vec3   getTranslation(double timeStamp);
    glm::quat   getRotation(double timeStamp);
    glm::vec3   getScale(double timeStamp);
    double      getDuration();
    double      getTicksPerSecond();

    friend class AIAdapter;
private:
    std::weak_ptr<Animation>    mAnimation;
    std::vector<VectorState>    mTranslationState;
    std::vector<RotationState>  mRotationState;
    std::vector<VectorState>    mScaleState;
    int                         mType;
};

/// MeshAnim is used to animate mesh data
class MeshAnim : public NameObj {
public:
    MeshAnim();
    ~MeshAnim();

    friend class AIAdapter;
private:
    std::weak_ptr<Animation>    mAnimation;
    std::vector<MeshState>      mMeshState;
};

class Animation : public NameObj {
public:
    Animation();
    Animation(double duration, double ticksPerSecond);
    ~Animation();

    void    setDuration(double duration);
    double  getDuration();
    void    setTicksPerSecond(double ticksPerSecond);
    double  getTicksPerSecond();
    unsigned int getNumNodeAnims();
    unsigned int getNumMeshAnims();
    std::shared_ptr<NodeAnim> getNodeAnim(int idx);
    std::shared_ptr<MeshAnim> getMeshAnim(int idx);

    friend class AIAdapter;
private:
    double mDuration;
    double mTicksPerSecond;
    std::vector<std::shared_ptr<NodeAnim> > mNodeAnims;
    std::vector<std::shared_ptr<MeshAnim> > mMeshAnims;
};

// copied from assimp
template <typename T>
struct Interpolator {
    // linear
    void operator () (T& out,const T& a, const T& b, float f) const {
        out = a + (b-a)*f;
    }
};

// template specialization
template <>
struct Interpolator <glm::quat> {
    // SLERP
    void operator () (glm::quat& out,const glm::quat& a,
        const glm::quat& b, float f) const {
        out = glm::mix(a, b, f);
    }
};

template <>
struct Interpolator <unsigned int> {
    // nearest
    void operator () (unsigned int& out,unsigned int a,
        unsigned int b, float f) const {
        out = f>0.5f ? b : a;
    }
};

template <>
struct Interpolator  <VectorState> {
    void operator () (glm::vec3& out,const VectorState& a,
        const VectorState& b, float f) const {
        Interpolator<glm::vec3> ipl;
        ipl(out, a.getValue(), b.getValue(), f);
    }
};

template <>
struct Interpolator <RotationState> {
    void operator () (glm::quat& out, const RotationState& a,
        const RotationState& b, float f) const {
        Interpolator<glm::quat> ipl;
        ipl(out, a.getValue(), b.getValue(), f);
    }
};

template <>
struct Interpolator <MeshState> {
    void operator () (unsigned int& out, const MeshState& a,
        const MeshState& b, float f) const {
        Interpolator<unsigned int> ipl;
        ipl(out, a.getValue(), b.getValue(), f);
    }
};

}

#endif
