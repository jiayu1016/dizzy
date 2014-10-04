#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <string>

namespace dzy {

class SceneManager;
class AppContext;
class Scene {
public:
    Scene();
    virtual ~Scene();

    virtual bool loadAsset(std::shared_ptr<AppContext> appContext,
        const std::string &assetFile) = 0;
    virtual bool load(std::shared_ptr<AppContext> appContext,
        const std::string &file) = 0;
    virtual bool listAssetFiles(std::shared_ptr<AppContext> appContext,
        const std::string &dir) = 0;

protected:

    void * mSceneData;
    std::size_t mSceneSize;
};

struct FlatSceneData {

};

class FlatScene : public Scene, public FlatSceneData {
public:
    explicit FlatScene();
    virtual ~FlatScene();

    virtual bool loadAsset(std::shared_ptr<AppContext> appContext, const std::string &asset);
    virtual bool load(std::shared_ptr<AppContext> appContext, const std::string &file);
    virtual bool listAssetFiles(std::shared_ptr<AppContext> appContext,
        const std::string &dir);

    friend class SceneManager;

};

class SceneManager {
public:
    enum SceneType {
        SCENE_TYPE_FLAT,
    };
    explicit SceneManager();
    ~SceneManager();

    static std::shared_ptr<Scene> createScene(SceneType);
};

} // namespace dzy
#endif
