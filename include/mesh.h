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
    unsigned int append(int size, unsigned char *buf);
    inline bool empty() const { return mBuffer.empty(); }
    inline unsigned int getBufSize() { return mBuffer.size(); };

    void * getBuf(int offset = 0);

private:
    std::vector<unsigned char>      mBuffer;
};

class AIAdapter;
class Render;
class Mesh {
public:
    enum {
        MAX_COLOR_SETS          = 0x8,
        MAX_TEXTURECOORDS       = 0x8,
    };

    enum PrimitiveType {
        PRIMITIVE_TYPE_TRIANGLES = 0,
        PRIMITIVE_TYPE_NUM
    };

    Mesh();

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

    unsigned int    getPositionNumComponent() const;
    unsigned int    getPositionBufStride() const;
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

    unsigned int    getTangentNumComponent() const;
    unsigned int    getTangentBufStride() const;
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

    void appendVertexPositions(unsigned char *buf, unsigned int numVertices,
        unsigned int numComponents, unsigned int bytesEachComponent);
    void appendVertexColors(unsigned char *buf, unsigned int numVertices,
        unsigned int numComponents, unsigned int bytesEachComponent, unsigned int channel);
    void appendVertexTextureCoords(unsigned char *buf, unsigned int numVertices,
        unsigned int numComponents, unsigned int bytesEachComponent, unsigned int channel);
    void appendVertexNormals(unsigned char *buf, unsigned int numVertices,
        unsigned int numComponents, unsigned int bytesEachComponent);
    void appendVertexTangents(unsigned char *buf, unsigned int numVertices,
        unsigned int numComponents, unsigned int bytesEachComponent);
    void appendVertexBitangents(unsigned char *buf, unsigned int numVertices,
        unsigned int numComponents, unsigned int bytesEachComponent);

    void            reserveDataStorage(int size);

    void            dumpVertexPositionBuf(int groupSize = 3);
    void            dumpIndexBuf (int groupSize = 3);

    friend class AIAdapter;
    friend class Render;
private:
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
};


}

#endif
