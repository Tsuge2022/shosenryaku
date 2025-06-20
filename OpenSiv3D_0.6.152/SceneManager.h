#pragma once
#include "DataTypes.h" // For SceneState enum
#include <Siv3D.hpp>   // For basic Siv3D types if needed directly

// Forward declarations for actual scene classes if they are defined elsewhere
// class TitleScene;
// class GameScene;
// class EditScene;

// A base class for all scenes could be useful
class IScene {
public:
    virtual ~IScene() = default;
    virtual void Initialize() = 0; // Called once when the scene is set
    virtual void Update() = 0;     // Called every frame
    virtual void Draw() const = 0; // Called every frame
    virtual void Shutdown() = 0;   // Called once when changing away from this scene

    SceneState GetSceneState() const { return sceneStateType; }
protected:
    IScene(SceneState type) : sceneStateType(type) {}
    SceneState sceneStateType;
};


class SceneManager
{
public:
    SceneManager();  // Constructor
    ~SceneManager(); // Destructor

    void UpdateCurrentScene();
    void DrawCurrentScene() const;

    // Changes the current scene. Handles shutdown of old and init of new.
    void ChangeScene(SceneState newState);
    SceneState GetCurrentSceneState() const;

    // Potentially register scenes (if using a factory pattern or similar)
    // void RegisterScene(SceneState state, std::unique_ptr<IScene> scene);

private:
    std::unique_ptr<IScene> currentScene; // Using a smart pointer to manage scene lifetime
    SceneState currentSceneState;
    // GameController* gameController; // If scenes need access back to the main controller

    // Helper to perform the actual transition
    void PerformSceneChange(SceneState newState);
};
