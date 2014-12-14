#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <memory>
#include "nameobj.h"
#include "transform.h"

namespace dzy {

/// hold raw buffer data of a triangle face
class TriangleFace {
public:
    unsigned int mIndices[3];
};

struct VertexWeight
{
    unsigned int mVertexIndex;
    // the weight sum of all bones that have influcence to a vertex
    // is equal to 1, so this value ranges from 0 ~ 1
    float mWeight;

    VertexWeight();
    VertexWeight(unsigned int index, float weight);
};

class Mesh;
/// Bones have a name to index a unique node in scene graph
struct Bone : public NameObj {
    std::vector<VertexWeight> mWeights;
    Transform mTransform;

    Bone(const std::string& name);
    Bone(const Bone& other);
    ~Bone();
    void transform(std::shared_ptr<Mesh> mesh, const Transform& nodeTransform);
};


/// hold raw buffer data of a Mesh
class MeshData {
public:
    void reserve(int size) { mBuffer.reserve(size); };
    /// append size-length buffer to current data store
    ///
    ///     @param buf the buffer to copy
    ///     @param size the size of the buffer in bytes
    ///     @return the offset of the buf in the data store
    unsigned int append(void *buf, int size);

    /// enlarge size to current data store, no data copied
    ///
    ///     @param size the size of the buffer in bytes
    ///     @return the offset of the buf in the data store
    unsigned int append(int size);

    /// tell if the data store is empty
    inline bool empty() const { return mBuffer.empty(); }

    /// return the size of the data store in bytes
    inline unsigned int getBufSize() { return mBuffer.size(); };

    /// get the raw buffer pointer
    ///
    ///     @param offset the offset pointer to the data store
    ///     @return the raw buffer pointer
    void* getBuf(int offset = 0);

private:
    std::vector<unsigned char>      mBuffer;
};

class AIAdapter;
class Render;
/// The base class of all kind of Meshes
///
///     This class is used for building Mesh manuall when you already
///     have vertex datas. If you want to create Meshes programmably,
///     use the derivative classes instead.
class Mesh : public NameObj {
public:
    enum {
        MAX_COLOR_SETS          = 0x8,
        MAX_TEXTURECOORDS       = 0x8,
    };

    enum PrimitiveType {
        PRIMITIVE_TYPE_TRIANGLE = 0,
        PRIMITIVE_TYPE_NUM
    };

    Mesh(PrimitiveType type, unsigned int numVertices, const std::string &name = "");

    bool            hasVertexPositions() const;
    bool            hasVertexColors(unsigned int channel) const;
    bool            hasVertexColors() const;
    bool            hasVertexTextureCoords(unsigned int channel) const;
    bool            hasVertexTextureCoords() const;
    bool            hasVertexNormals() const;
    bool            hasVertexTangentsAndBitangents() const;
    bool            hasFaces() const;
    bool            hasBones() const;

    unsigned int    getNumVertices() const;
    unsigned int    getNumColorChannels() const;
    unsigned int    getNumTextureCoordChannels() const;
    unsigned int    getNumFaces() const;
    unsigned int    getNumIndices() const;
    unsigned int    getNumBones() const;
    std::shared_ptr<Bone>   getBone(int idx);

    virtual unsigned int    getPositionNumComponent() const;
    virtual unsigned int    getPositionBufStride() const;
    unsigned int    getPositionBufSize() const;
    unsigned int    getOriginalPositionOffset() const;
    void*           getOriginalPositionBuf();
    unsigned int    getTransformedPositionOffset() const;
    void*           getTransformedPositionBuf();
    unsigned int    getPositionOffset() const;
    void*           getPositionBuf();


    unsigned int    getColorNumComponent(int channel) const;
    unsigned int    getColorBufStride(int channel) const;
    unsigned int    getColorBufSize(int channel) const;
    unsigned int    getColorBufSize() const;
    unsigned int    getColorOffset(int channel) const;
    void*           getColorBuf(int channel);

    unsigned int    getTextureCoordBufSize(int channel) const;
    unsigned int    getTextureCoordBufSize() const;

    unsigned int    getNormalNumComponent() const;
    unsigned int    getNormalBufStride() const;
    unsigned int    getNormalBufSize() const;
    unsigned int    getOriginalNormalOffset() const;
    void*           getOriginalNormalBuf();
    unsigned int    getTransformedNormalOffset() const;
    void*           getTransformedNormalBuf();
    unsigned int    getNormalOffset() const;
    void*           getNormalBuf();

    virtual unsigned int    getTangentNumComponent() const;
    virtual unsigned int    getTangentBufStride() const;
    unsigned int    getTangentBufSize() const;
    unsigned int    getTangentOffset() const;
    void*           getTangentBuf();

    unsigned int    getBitangentNumComponent() const;
    unsigned int    getBitangentBufStride() const;
    unsigned int    getBitangentBufSize() const;
    unsigned int    getBitangentOffset() const;
    void*           getBitangentBuf();

    unsigned int    getVertexBufSize() const;
    void*           getVertexBuf();

    unsigned int    getIndexBufSize() const;
    void*           getIndexBuf();

    // these appendVertexXXX functions are used to build MeshData's raw
    // buffer into "structure of arrays", the order is not important, internal
    // variables are used to track the offset
    void appendVertexPositions(
        void *buf,
        unsigned int numComponents,
        unsigned int bytesEachComponent);
    void appendVertexColors(
        void *buf,
        unsigned int numComponents,
        unsigned int bytesEachComponent,
        unsigned int channel);
    void appendVertexTextureCoords(
        void *buf,
        unsigned int numComponents,
        unsigned int bytesEachComponent,
        unsigned int channel);
    void appendVertexNormals(
        void *buf,
        unsigned int numComponents,
        unsigned int bytesEachComponent);
    void appendVertexTangents(
        void *buf,
        unsigned int numComponents,
        unsigned int bytesEachComponent);
    void appendVertexBitangents(
        void *buf,
        unsigned int numComponents,
        unsigned int bytesEachComponent);
    void allocateTransformDataArea();
    void buildIndexBuffer(void *buf, int numFaces);
    void transform(unsigned int vertexIdx, float weight, Transform& transform);

    void reserveDataStorage(int size);
    void dumpBuf(Log::Flag f, void *buff, unsigned int bufSize, int groupSize = 3);
    void dumpIndexBuf (Log::Flag f, int groupSize = 3);

    friend class AIAdapter;
    friend class Render;
protected:
    PrimitiveType                       mPrimitiveType;

    unsigned int                        mNumVertices;
    unsigned int                        mNumFaces;
    std::vector<TriangleFace>           mTriangleFaces;

    std::vector<std::shared_ptr<Bone> > mBones;

    MeshData                            mMeshData;
    int                                 mTransformedPosOffset;
    int                                 mTransformedNormalOffset;

    unsigned int                        mPosOffset;
    unsigned int                        mPosNumComponents;
    unsigned int                        mPosBytesComponent;
    bool                                mHasPos;

    unsigned int                        mColorOffset[MAX_COLOR_SETS];
    unsigned int                        mColorNumComponents[MAX_COLOR_SETS];
    unsigned int                        mColorBytesComponent[MAX_COLOR_SETS];
    unsigned int                        mNumColorChannels;

    unsigned int                        mTextureCoordOffset[MAX_TEXTURECOORDS];
    unsigned int                        mTextureCoordNumComponents[MAX_TEXTURECOORDS];
    unsigned int                        mTextureCoordBytesComponent[MAX_TEXTURECOORDS];
    unsigned int                        mNumTextureCoordChannels;

    unsigned int                        mNormalOffset;
    unsigned int                        mNormalNumComponents;
    unsigned int                        mNormalBytesComponent;
    bool                                mHasNormal;

    unsigned int                        mTangentOffset;
    unsigned int                        mTangentNumComponents;
    unsigned int                        mTangentBytesComponent;
    bool                                mHasTangent;

    unsigned int                        mBitangentOffset;
    unsigned int                        mBitangentNumComponents;
    unsigned int                        mBitangentBytesComponent;
    bool                                mHasBitangent;

    // A mesh use only ONE material, otherwise it is splitted to multiple meshes
    unsigned int                        mMaterialIndex;
};

class CubeMesh : public Mesh {
public:
    CubeMesh(const std::string& name = "");
};

class PyramidMesh : public Mesh {
public:
    PyramidMesh(const std::string& name = "");
};

}

#endif
