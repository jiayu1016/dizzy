#include "animation.h"

using namespace std;

namespace dzy {

VectorState::VectorState() {
}

VectorState::VectorState(double time, const glm::vec3& value)
    : mTime(time)
    , mValue(value) {
}

bool VectorState::operator == (const VectorState& o) const {
    return o.mValue == this->mValue;
}
bool VectorState::operator != (const VectorState& o) const {
    return o.mValue != this->mValue;
}

bool VectorState::operator < (const VectorState& o) const {
    return mTime < o.mTime;
}
bool VectorState::operator > (const VectorState& o) const {
    return mTime > o.mTime;
}

double VectorState::getTime() const {
    return mTime;
}

void VectorState::setTime(double time) {
    mTime = time;
}

glm::vec3 VectorState::getValue() const {
    return mValue;
}

void VectorState::setValue(const glm::vec3& value) {
    mValue = value;
}

RotationState::RotationState() {
}

RotationState::RotationState(double time, const glm::quat& value)
    : mTime(time)
    , mValue(value) {
}

bool RotationState::operator == (const RotationState& o) const {
    return o.mValue == this->mValue;
}
bool RotationState::operator != (const RotationState& o) const {
    return o.mValue != this->mValue;
}

bool RotationState::operator < (const RotationState& o) const {
    return mTime < o.mTime;
}
bool RotationState::operator > (const RotationState& o) const {
    return mTime > o.mTime;
}

double RotationState::getTime() const {
    return mTime;
}

void RotationState::setTime(double time) {
    mTime = time;
}

glm::quat RotationState::getValue() const {
    return mValue;
}

void RotationState::setValue(const glm::quat& value) {
    mValue = value;
}

MeshState::MeshState() {
}

MeshState::MeshState(double time, unsigned int value)
    : mTime(time)
    , mValue(value) {
}

bool MeshState::operator == (const MeshState& o) const {
    return o.mValue == this->mValue;
}

bool MeshState::operator != (const MeshState& o) const {
    return o.mValue != this->mValue;
}

bool MeshState::operator < (const MeshState& o) const {
    return mTime < o.mTime;
}

bool MeshState::operator > (const MeshState& o) const {
    return mTime > o.mTime;
}

double MeshState::getTime() const {
    return mTime;
}

void MeshState::setTime(double time) {
    mTime = time;
}

unsigned int MeshState::getValue() const {
    return mValue;
}

void MeshState::setValue(unsigned int value) {
    mValue = value;
}

NodeAnim::NodeAnim(const char* name)
    : NameObj(name), mType(0) {
    TRACE("");
}

NodeAnim::~NodeAnim() {
    TRACE("");
}

glm::vec3 NodeAnim::getTranslation(double timeStamp) {
    glm::vec3 ret(0.f, 0.f, 0.f);

    if (mTranslationState.empty()) {
        ALOGW("NodeAnim has no traslation state");
        return ret;
    }

    double duration = getDuration();
    int num = (int)(timeStamp / duration);
    timeStamp -= num * duration;
    int i = 0;
    for (; i<mTranslationState.size(); i++) {
        double time = mTranslationState[i].getTime();
        if (time >= timeStamp) break;
    }
    if (i == mTranslationState.size()) {
        ret = mTranslationState[i-1].getValue();
    } else if (i == 0) {
        ret = mTranslationState[0].getValue();
    } else {
        VectorState s1 = mTranslationState[i - 1];
        VectorState s2 = mTranslationState[i];
        double t1 = s1.getTime();
        double t2 = s2.getTime();
        float f = (timeStamp - t1) / (t2 - t1);
        Interpolator<glm::vec3>()(ret, s1.getValue(), s2.getValue(), f);
    }
    return ret;
}

glm::quat NodeAnim::getRotation(double timeStamp) {
    glm::quat ret(1.f, 0.f, 0.f, 0.f);
    if (mRotationState.empty()) {
        ALOGW("NodeAnim has no rotation state");
        return ret;
    }

    double duration = getDuration();
    int num = (int)(timeStamp / duration);
    timeStamp -= num * duration;
    int i = 0;
    for (; i<mRotationState.size(); i++) {
        double time = mRotationState[i].getTime();
        if (time >= timeStamp) break;
    }
    if (i == mRotationState.size()) {
        // currentTime is after the last state, use the last one
        ret = mRotationState[i-1].getValue();
    } else if (i == 0) {
        // currentTime is before the first state, use the first one
        ret = mRotationState[0].getValue();
    } else {
        RotationState s1 = mRotationState[i - 1];
        RotationState s2 = mRotationState[i];
        double t1 = s1.getTime();
        double t2 = s2.getTime();
        float f = (timeStamp - t1) / (t2 - t1);
        //DUMP(Log::F_ANIMATION, "f: %f", f);
        Interpolator<RotationState>()(ret, s1, s2, f);
    }
    return ret;
}

glm::vec3 NodeAnim::getScale(double timeStamp) {
    glm::vec3 ret(1.f, 1.f, 1.f);

    if (mScaleState.empty()) {
        ALOGW("NodeAnim has no scale state");
        return ret;
    }

    double duration = getDuration();
    int num = (int)(timeStamp / duration);
    timeStamp -= num * duration;
    int i = 0;
    for (; i<mScaleState.size(); i++) {
        double time = mScaleState[i].getTime();
        if (time >= timeStamp) break;
    }
    if (i == mScaleState.size()) {
        // currentTime is after the last state, use the last one
        ret = mScaleState[i-1].getValue();
    } else if (i == 0) {
        // currentTime is before the first state, use the first one
        ret = mScaleState[0].getValue();
    } else {
        VectorState s1 = mScaleState[i - 1];
        VectorState s2 = mScaleState[i];
        double t1 = s1.getTime();
        double t2 = s2.getTime();
        float f = (timeStamp - t1) / (t2 - t1);
        Interpolator<glm::vec3>()(ret, s1.getValue(), s2.getValue(), f);
    }
    return ret;
}

double NodeAnim::getDuration() {
    shared_ptr<Animation> animation(mAnimation.lock());
    if (animation) {
        return animation->getDuration();
    }
    ALOGE("Animation instance hosting this NodeAnim is released");
    return 1.0;
}

double NodeAnim::getTicksPerSecond() {
    shared_ptr<Animation> animation(mAnimation.lock());
    if (animation) {
        return animation->getTicksPerSecond();
    }
    ALOGE("Animation instance hosting this NodeAnim is released");
    return 1.0;
}

MeshAnim::MeshAnim() {
    TRACE("");
}

MeshAnim::~MeshAnim() {
    TRACE("");
}

Animation::Animation()
    : mDuration(-1.)
    , mTicksPerSecond() {
    TRACE("");
}

Animation::Animation(double duration, double ticksPerSecond)
    : mDuration(duration)
    , mTicksPerSecond(ticksPerSecond) {
    TRACE("");
}

Animation::~Animation() {
}

void Animation::setDuration(double duration) {
    mDuration = duration;
}

double Animation::getDuration() {
    return mDuration;
}

void Animation::setTicksPerSecond(double ticksPerSecond) {
    mTicksPerSecond = ticksPerSecond;
}

double Animation::getTicksPerSecond() {
    return mTicksPerSecond;
}

unsigned int Animation::getNumNodeAnims() {
    return mNodeAnims.size();
}

unsigned int Animation::getNumMeshAnims() {
    return mMeshAnims.size();
}

shared_ptr<NodeAnim> Animation::getNodeAnim(int idx) {
    if (idx >=0 && idx < mNodeAnims.size())
        return mNodeAnims[idx];
    return nullptr;
}

shared_ptr<MeshAnim> Animation::getMeshAnim(int idx) {
    if (idx >=0 && idx < mMeshAnims.size())
        return mMeshAnims[idx];
    return nullptr;
}

}
