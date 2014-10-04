#ifndef ASSIMP_ADAPTER_H
#define ASSIMP_ADAPTER_H

#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>

namespace dzy {

class AssetIOStream : public Assimp::IOStream
{
protected:
    // Used only by friend class AssetIOSystem
    AssetIOStream(void);

public:
    ~AssetIOStream(void);
    std::size_t Read(void* buffer, std::size_t size, std::size_t count);
    std::size_t Write(const void* buffer, std::size_t size, std::size_t count);
    aiReturn Seek(std::size_t offset, aiOrigin origin);
    std::size_t Tell() const;
    std::size_t FileSize() const;
    void Flush();

    friend class AssetIOSystem;

};

class AssetIOSystem : public Assimp::IOSystem
{
public:
    AssetIOSystem() {};
    ~AssetIOSystem() {};

    // Check whether a specific file exists
    bool Exists(const std::string& file) const { return true; }

    // Get the path delimiter character
    char GetOsSeparator() const { return '/'; }

    // Open a custom stream
    AssetIOStream* Open(const std::string& file, const std::string& mode) {
        return new AssetIOStream();
    }
    void Close(AssetIOStream* file) { delete file; }
};

}

#endif
