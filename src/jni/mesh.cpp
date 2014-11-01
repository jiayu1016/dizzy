#include "log.h"
#include "mesh.h"

using namespace std;

namespace dzy {

unsigned int MeshData::append(int size, unsigned char *buf) {
    // TODO: consider optimizing
    int bufSize = mBuffer.size();
    if (mBuffer.capacity() < bufSize + size) {
        ALOGD("insufficent MeshData capacity, enlarge, %d => %d",
            mBuffer.capacity(), bufSize + size);
        mBuffer.reserve(bufSize + size);
        if (mBuffer.capacity() < bufSize + size) {
            ALOGE("failed to enlarge MeshData storage, data unchanged");
            return -1;
        }
    }

    mBuffer.resize(bufSize + size);
    memcpy(&mBuffer[bufSize], buf, size);
    return bufSize;
}

void * MeshData::getBuf(int offset) {
    return static_cast<void *>(&mBuffer[offset]);
}

Mesh::Mesh()
    : mPrimitiveType            (PRIMITIVE_TYPE_TRIANGLES)
    , mNumVertices              (0)
    , mNumFaces                 (0)
    , mMaterialIndex            (0)
    , mPosOffset                (0)
    , mPosNumComponents         (0)
    , mPosBytesComponent        (0)
    , mHasPos                   (false)
    , mNumColorChannels         (0)
    , mNumTextureCoordChannels  (0)
    , mNormalOffset             (0)
    , mNormalNumComponents      (0)
    , mNormalBytesComponent     (0)
    , mHasNormal                (false)
    , mTangentOffset            (0)
    , mTangentNumComponents     (0)
    , mTangentBytesComponent    (0)
    , mHasTangent               (false)
    , mBitangentOffset          (0)
    , mBitangentNumComponents   (0)
    , mBitangentBytesComponent  (0)
    , mHasBitangent             (false) {
    memset(&mColorOffset[0], 0, MAX_COLOR_SETS * sizeof(unsigned int));
    memset(&mColorNumComponents[0], 0, MAX_COLOR_SETS * sizeof(unsigned int));
    memset(&mColorBytesComponent[0], 0, MAX_COLOR_SETS * sizeof(unsigned int));
    memset(&mTextureCoordOffset[0], 0, MAX_TEXTURECOORDS * sizeof(unsigned int));
    memset(&mTextureCoordNumComponents[0], 0, MAX_TEXTURECOORDS * sizeof(unsigned int));
    memset(&mTextureCoordBytesComponent[0], 0, MAX_TEXTURECOORDS * sizeof(unsigned int));
}

bool Mesh::hasVertexPositions() const {
    return mHasPos;
}

bool Mesh::hasVertexColors(unsigned int channel) const {
    if( channel >= MAX_COLOR_SETS)
        return false;
    else
        // these values are 0 by default
        return  mColorNumComponents[channel] &&
                mColorBytesComponent[channel] &&
                mNumVertices > 0;
}

bool Mesh::hasVertexColors() const {
    return mNumColorChannels > 0;
}

bool Mesh::hasVertexTextureCoords(unsigned int channel) const {
    if( channel >= MAX_TEXTURECOORDS)
        return false;
    else
        // these values are 0 by default
        return  mTextureCoordNumComponents[channel] &&
                mTextureCoordNumComponents[channel] &&
                mNumVertices > 0;
}

bool Mesh::hasVertexTextureCoords() const {
    return mNumTextureCoordChannels > 0;
}

bool Mesh::hasVertexNormals() const {
    return mHasNormal;
}

bool Mesh::hasVertexTangentsAndBitangents() const {
    return mHasTangent && mHasBitangent;
}

bool Mesh::hasFaces() const {
    return !mTriangleFaces.empty() && mNumFaces > 0;
}

unsigned int Mesh::getNumVertices() const {
    return mNumVertices;
}

unsigned int Mesh::getNumColorChannels() const {
    return mNumColorChannels;
}

unsigned int Mesh::getNumTextureCoordChannels() const {
    return mNumTextureCoordChannels;
}

unsigned int Mesh::getNumFaces() const {
    return mNumFaces;
}

unsigned int Mesh::getNumIndices() const {
    return mNumFaces * 3;
}

unsigned int Mesh::getPositionNumComponent() const {
    return mPosNumComponents;
}

unsigned int Mesh::getPositionBufStride() const {
    return mPosNumComponents * mPosBytesComponent;
}

unsigned int Mesh::getPositionBufSize() const {
    return getPositionBufStride() * mNumVertices;
}

unsigned int Mesh::getPositionOffset() const {
    return mPosOffset;
}

void * Mesh::getPositionBuf() {
    return mMeshData.getBuf(mPosOffset);
}

unsigned int Mesh::getColorBufSize(int channel) const {
    return mColorBytesComponent[channel] *
        mColorNumComponents[channel] * mNumVertices;
}

unsigned int Mesh::getColorBufSize() const {
    int totalSize = 0;
    for (unsigned int i=0; i<getNumColorChannels(); i++)
        totalSize += getColorBufSize(i);
    return totalSize;
}

unsigned int Mesh::getTextureCoordBufSize(int channel) const {
    return mTextureCoordBytesComponent[channel] *
        mTextureCoordNumComponents[channel] * mNumVertices;
}

unsigned int Mesh::getTextureCoordBufSize() const {
    int totalSize = 0;
    for (unsigned int i=0; i<getNumTextureCoordChannels(); i++)
        totalSize += getTextureCoordBufSize(i);
    return totalSize;
}

unsigned int Mesh::getNormalNumComponent() const {
    return mNormalNumComponents;
}

unsigned int Mesh::getNormalBufStride() const {
    return mNormalNumComponents * mNormalBytesComponent;
}

unsigned int Mesh::getNormalBufSize() const {
    return getNormalBufStride() * mNumVertices;
}

unsigned int Mesh::getNormalOffset() const {
    return mNormalOffset;
}

void * Mesh::getNormalBuf() {
    return mMeshData.getBuf(mNormalOffset);
}

unsigned int Mesh::getTangentNumComponent() const {
    return mTangentNumComponents;
}

unsigned int Mesh::getTangentBufStride() const {
    return mTangentNumComponents * mTangentBytesComponent;
}

unsigned int Mesh::getTangentBufSize() const {
    return getTangentBufStride() * mNumVertices;
}

unsigned int Mesh::getTangentOffset() const {
    return mTangentOffset;
}

void * Mesh::getTangentBuf() {
    return mMeshData.getBuf(mTangentOffset);
}

unsigned int Mesh::getBitangentNumComponent() const {
    return mBitangentNumComponents;
}

unsigned int Mesh::getBitangentBufStride() const {
    return mBitangentNumComponents * mBitangentBytesComponent;
}

unsigned int Mesh::getBitangentBufSize() const {
    return getBitangentBufStride() * mNumVertices;
}

unsigned int Mesh::getBitangentOffset() const {
    return mBitangentOffset;
}

void * Mesh::getBitangentBuf() {
    return mMeshData.getBuf(mBitangentOffset);
}

unsigned int Mesh::getVertexBufSize() const {
    int totalSize = 0;
    if (hasVertexPositions())       totalSize += getPositionBufSize();
    if (hasVertexColors())          totalSize += getColorBufSize();
    if (hasVertexTextureCoords())   totalSize += getTextureCoordBufSize();
    if (hasVertexNormals())         totalSize += getNormalBufSize();
    if (hasVertexTangentsAndBitangents()) {
        totalSize += getTangentBufSize();
        totalSize += getBitangentBufSize();
    }
    return totalSize;
}

void * Mesh::getVertexBuf() {
    return mMeshData.getBuf();
}

unsigned int Mesh::getIndexBufSize() const {
    return getNumIndices() * sizeof(unsigned int);
}

void * Mesh::getIndexBuf() {
    if (mTriangleFaces.empty()) return NULL;
    return &mTriangleFaces[0];
}

void Mesh::appendVertexPositions(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mPosOffset = mMeshData.append(totalSize, buf);
    mPosNumComponents = numComponents;
    mPosBytesComponent = bytesEachComponent;
    mHasPos = true;
}

void Mesh::appendVertexColors(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent, unsigned int channel) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mColorOffset[channel] = mMeshData.append(totalSize, buf);
    mColorNumComponents[channel] = numComponents;
    mColorBytesComponent[channel] = bytesEachComponent;
    mNumColorChannels++;
}

void Mesh::appendVertexTextureCoords(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent, unsigned int channel) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mTextureCoordOffset[channel] = mMeshData.append(totalSize, buf);
    mTextureCoordNumComponents[channel] = numComponents;
    mTextureCoordBytesComponent[channel] = bytesEachComponent;
    mNumTextureCoordChannels++;
}

void Mesh::appendVertexNormals(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mNormalOffset = mMeshData.append(totalSize, buf);
    mNormalNumComponents = numComponents;
    mNormalBytesComponent = bytesEachComponent;
    mHasNormal = true;
}

void Mesh::appendVertexTangents(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mTangentOffset = mMeshData.append(totalSize, buf);
    mTangentNumComponents = numComponents;
    mTangentBytesComponent = bytesEachComponent;
    mHasTangent++;
}

void Mesh::appendVertexBitangents(unsigned char *buf, unsigned int numVertices,
    unsigned int numComponents, unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * numVertices;
    mBitangentOffset = mMeshData.append(totalSize, buf);
    mBitangentNumComponents = numComponents;
    mBitangentBytesComponent = bytesEachComponent;
    mHasBitangent++;
}

void Mesh::reserveDataStorage(int size) {
    mMeshData.reserve(size);
}

void Mesh::dumpVertexPositionBuf(int groupSize) {
    unsigned int bufSize = getPositionBufSize();
    float *buf = (float *)getPositionBuf();
    int num = bufSize/sizeof(float);
    char format[1024];

    if (num)
        PRINT("************ start Mesh::dumpVertexPositionBuf **********");
    for (int i=0; i<num; i+=groupSize) {
        int n = sprintf(format, "%8p:", buf + i);
        int left = (i+groupSize <= num) ? groupSize : num - i;
        for (int k=0; k<left; k++) {
            n += sprintf(format + n, " %+08.6f", buf[i+k]);
        }
        PRINT("%s", format);
    }
    if (num)
        PRINT("************ end Mesh::dumpVertexPositionBuf ************");
}

void Mesh::dumpIndexBuf(int groupSize) {
    unsigned int bufSize = getIndexBufSize();
    unsigned short *buf = (unsigned short *)getIndexBuf();
    // Attention: supports only integer type indices
    int num = bufSize/sizeof(unsigned short);
    char format[1024];

    if (num)
        PRINT("************ start Mesh::dumpIndexBuf **********");
    for (int i=0; i<num; i+=groupSize) {
        int n = sprintf(format, "%8p:", buf + i);
        int left = (i+groupSize <= num) ? groupSize : num - i;
        for (int k=0; k<left; k++) {
            n += sprintf(format + n, " %8u", buf[i+k]);
        }
        PRINT("%s", format);
    }
    if (num)
        PRINT("************ end Mesh::dumpIndexBuf ************");
}

} //namespace
