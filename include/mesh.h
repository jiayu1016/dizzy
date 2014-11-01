#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>

namespace dzy {

/// how raw buffer data of a triangle face
class TriangleFace {
public:
    unsigned int mIndices[3];
};

/// hold raw buffer data of a Mesh
class MeshData {
public:
    void reserve(int size) { mBuffer.reserve(size); };
    unsigned int append(void *buf, int size);
    inline bool empty() const { return mBuffer.empty(); }
    inline unsigned int getBufSize() { return mBuffer.size(); };

    void * getBuf(int offset = 0);

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
class Mesh {
public:
    enum {
        MAX_COLOR_SETS          = 0x8,
        MAX_TEXTURECOORDS       = 0x8,
    };

    enum PrimitiveType {
        PRIMITIVE_TYPE_TRIANGLE = 0,
        PRIMITIVE_TYPE_NUM
    };

    Mesh(PrimitiveType type, unsigned int numVertices);

    bool            hasVertexPositions() const;
    bool            hasVertexColors(unsigned int channel) const;
    bool            hasVertexColors() const;
    bool            hasVertexTextureCoords(unsigned int channel) const;
    bool            hasVertexTextureCoords() const;
    bool            hasVertexNormals() const;
    bool            hasVertexTangentsAndBitangents() const;
    bool            hasFaces() const;

    unsigned int    getNumVertices() const;
    unsigned int    getNumColorChannels() const;
    unsigned int    getNumTextureCoordChannels() const;
    unsigned int    getNumFaces() const;
    unsigned int    getNumIndices() const;

    virtual unsigned int    getPositionNumComponent() const;
    virtual unsigned int    getPositionBufStride() const;
    unsigned int    getPositionBufSize() const;
    unsigned int    getPositionOffset() const;
    void *          getPositionBuf();

    unsigned int    getColorBufSize(int channel) const;
    unsigned int    getColorBufSize() const;

    unsigned int    getTextureCoordBufSize(int channel) const;
    unsigned int    getTextureCoordBufSize() const;

    unsigned int    getNormalNumComponent() const;
    unsigned int    getNormalBufStride() const;
    unsigned int    getNormalBufSize() const;
    unsigned int    getNormalOffset() const;
    void *          getNormalBuf();

    virtual unsigned int    getTangentNumComponent() const;
    virtual unsigned int    getTangentBufStride() const;
    unsigned int    getTangentBufSize() const;
    unsigned int    getTangentOffset() const;
    void *          getTangentBuf();

    unsigned int    getBitangentNumComponent() const;
    unsigned int    getBitangentBufStride() const;
    unsigned int    getBitangentBufSize() const;
    unsigned int    getBitangentOffset() const;
    void *          getBitangentBuf();

    unsigned int    getVertexBufSize() const;
    void *          getVertexBuf();

    unsigned int    getIndexBufSize() const;
    void *          getIndexBuf();

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
    void buildIndexBuffer(void *buf, int numFaces);

    void            reserveDataStorage(int size);

    void            dumpVertexPositionBuf(int groupSize = 3);
    void            dumpIndexBuf (int groupSize = 3);

    void            setName(std::string name);
    std::string     getName();

    friend class AIAdapter;
    friend class Render;
protected:
    std::string                     mName;
    PrimitiveType                   mPrimitiveType;

    unsigned int                    mNumVertices;
    unsigned int                    mNumFaces;
    std::vector<TriangleFace>       mTriangleFaces;

    MeshData                        mMeshData;

    unsigned int                    mPosOffset;
    unsigned int                    mPosNumComponents;
    unsigned int                    mPosBytesComponent;
    bool                            mHasPos;

    unsigned int                    mColorOffset[MAX_COLOR_SETS];
    unsigned int                    mColorNumComponents[MAX_COLOR_SETS];
    unsigned int                    mColorBytesComponent[MAX_COLOR_SETS];
    unsigned int                    mNumColorChannels;

    unsigned int                    mTextureCoordOffset[MAX_TEXTURECOORDS];
    unsigned int                    mTextureCoordNumComponents[MAX_TEXTURECOORDS];
    unsigned int                    mTextureCoordBytesComponent[MAX_TEXTURECOORDS];
    unsigned int                    mNumTextureCoordChannels;

    unsigned int                    mNormalOffset;
    unsigned int                    mNormalNumComponents;
    unsigned int                    mNormalBytesComponent;
    bool                            mHasNormal;

    unsigned int                    mTangentOffset;
    unsigned int                    mTangentNumComponents;
    unsigned int                    mTangentBytesComponent;
    bool                            mHasTangent;

    unsigned int                    mBitangentOffset;
    unsigned int                    mBitangentNumComponents;
    unsigned int                    mBitangentBytesComponent;
    bool                            mHasBitangent;

    // A mesh use only ONE material, otherwise it is splitted to multiple meshes
    unsigned int                    mMaterialIndex;
    static int                      mCount;
};

class CubeMesh : public Mesh {
public:
    /// ctor, build a CubeMesh
    CubeMesh();
};

}

#endif
