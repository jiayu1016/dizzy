#include <sstream>
#include "log.h"
#include "mesh.h"

using namespace std;

namespace dzy {

unsigned int MeshData::append(void *buf, int size) {
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

int Mesh::mCount = 0;

Mesh::Mesh(PrimitiveType type, unsigned int numVertices)
    : mPrimitiveType            (type)
    , mNumVertices              (numVertices)
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
    ostringstream os;
    os << "Mesh-" << mCount++;
    mName = os.str();
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
    // TODO:
    if (mPrimitiveType == PRIMITIVE_TYPE_TRIANGLE)
        return mNumFaces * 3;
    else return 0;
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

void Mesh::appendVertexPositions(
    void *buf,
    unsigned int numComponents,
    unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * mNumVertices;
    mPosOffset = mMeshData.append(buf, totalSize);
    mPosNumComponents = numComponents;
    mPosBytesComponent = bytesEachComponent;
    mHasPos = true;
}

void Mesh::appendVertexColors(
    void *buf,
    unsigned int numComponents,
    unsigned int bytesEachComponent,
    unsigned int channel) {
    int totalSize = bytesEachComponent * numComponents * mNumVertices;
    mColorOffset[channel] = mMeshData.append(buf, totalSize);
    mColorNumComponents[channel] = numComponents;
    mColorBytesComponent[channel] = bytesEachComponent;
    mNumColorChannels++;
}

void Mesh::appendVertexTextureCoords(
    void *buf,
    unsigned int numComponents,
    unsigned int bytesEachComponent,
    unsigned int channel) {
    int totalSize = bytesEachComponent * numComponents * mNumVertices;
    mTextureCoordOffset[channel] = mMeshData.append(buf, totalSize);
    mTextureCoordNumComponents[channel] = numComponents;
    mTextureCoordBytesComponent[channel] = bytesEachComponent;
    mNumTextureCoordChannels++;
}

void Mesh::appendVertexNormals(
    void *buf,
    unsigned int numComponents,
    unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * mNumVertices;
    mNormalOffset = mMeshData.append(buf, totalSize);
    mNormalNumComponents = numComponents;
    mNormalBytesComponent = bytesEachComponent;
    mHasNormal = true;
}

void Mesh::appendVertexTangents(
    void *buf,
    unsigned int numComponents,
    unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * mNumVertices;
    mTangentOffset = mMeshData.append(buf, totalSize);
    mTangentNumComponents = numComponents;
    mTangentBytesComponent = bytesEachComponent;
    mHasTangent++;
}

void Mesh::appendVertexBitangents(
    void *buf,
    unsigned int numComponents,
    unsigned int bytesEachComponent) {
    int totalSize = bytesEachComponent * numComponents * mNumVertices;
    mBitangentOffset = mMeshData.append(buf, totalSize);
    mBitangentNumComponents = numComponents;
    mBitangentBytesComponent = bytesEachComponent;
    mHasBitangent++;
}

void Mesh::buildIndexBuffer(void *buf, int numFaces) {
    mNumFaces = numFaces;
    mTriangleFaces.resize(mNumFaces);
    int numIndices = 0;
    if (mPrimitiveType == PRIMITIVE_TYPE_TRIANGLE)
        numIndices = mNumFaces * 3;
    memcpy(&mTriangleFaces[0].mIndices[0], buf, numIndices * sizeof(unsigned int));
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

void Mesh::setName(std::string name) {
    mName = name;
}

std::string Mesh::getName() {
    return mName;
}

CubeMesh::CubeMesh()
    : Mesh(PRIMITIVE_TYPE_TRIANGLE, 24) {
    float verts[] = {
        // front
        -0.5f, +0.5f, +0.5f,
        -0.5f, -0.5f, +0.5f,
        +0.5f, -0.5f, +0.5f,
        +0.5f, +0.5f, +0.5f,

        // up
        -0.5f, +0.5f, -0.5f,
        -0.5f, +0.5f, +0.5f,
        +0.5f, +0.5f, +0.5f,
        +0.5f, +0.5f, -0.5f,

        // right
        +0.5f, +0.5f, +0.5f,
        +0.5f, -0.5f, +0.5f,
        +0.5f, -0.5f, -0.5f,
        +0.5f, +0.5f, -0.5f,

        // back
        +0.5f, +0.5f, -0.5f,
        +0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, +0.5f, -0.5f,

        // left
        -0.5f, +0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, +0.5f,
        -0.5f, +0.5f, +0.5f,

        // bottom
        -0.5f, -0.5f, +0.5f,
        -0.5f, -0.5f, -0.5f,
        +0.5f, -0.5f, -0.5f,
        +0.5f, -0.5f, +0.5f,
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3,
        4, 5, 6,
        4, 6, 7,
        8, 9, 10,
        8, 10, 11,
        12, 13, 14,
        12, 14, 15,
        16, 17, 18,
        16, 18, 19,
        20, 21, 22,
        20, 22, 23
    };

    Mesh::appendVertexPositions(verts, 3, sizeof(float));
    Mesh::buildIndexBuffer(indices, 12);
}

} //namespace
