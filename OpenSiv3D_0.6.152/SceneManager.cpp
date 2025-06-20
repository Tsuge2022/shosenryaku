#include "SceneManager.h"
// Include actual scene headers here once they are created
// #include "TitleScene.h"
// #include "GameScene.h"
// #include "EditScene.h"

// Implementation for the IScene base class methods if any are not pure virtual
// (Currently all are pure virtual or have simple implementations in the header)

SceneManager::SceneManager() : currentScene(nullptr), currentSceneState(SceneState::title) // Default to title or an invalid state
{
    // Constructor implementation
    // PerformSceneChange(currentSceneState); // Optionally, load the initial scene here
}

SceneManager::~SceneManager()
{
    // Destructor implementation
    // currentScene unique_ptr will handle deletion of the scene object.
    if (currentScene) {
        currentScene->Shutdown();
    }
}

void SceneManager::UpdateCurrentScene()
{
    if (currentScene)
    {
        currentScene->Update();
    }
}

void SceneManager::DrawCurrentScene() const
{
    if (currentScene)
    {
        currentScene->Draw();
    }
}

void SceneManager::ChangeScene(SceneState newState)
{
    if (newState == currentSceneState && currentScene)
    {
        // Optionally re-initialize if changing to the same scene, or do nothing
        // currentScene->Shutdown();
        // currentScene->Initialize();
        return;
    }
    PerformSceneChange(newState);
}

SceneState SceneManager::GetCurrentSceneState() const
{
    return currentSceneState;
}

void SceneManager::PerformSceneChange(SceneState newState)
{
    if (currentScene)
    {
        currentScene->Shutdown();
    }

    // Example of how scenes might be created.
    // This part will need to be filled in with actual scene classes.
    /*
    switch (newState)
    {
    case SceneState::title:
        // currentScene = std::make_unique<TitleScene>(*this); // Pass SceneManager ref if scenes need to change scenes
        break;
    case SceneState::game:
        // currentScene = std::make_unique<GameScene>(*this);
        break;
    case SceneState::edit:
        // currentScene = std::make_unique<EditScene>(*this);
        break;
    default:
        // Log error or handle unknown state
        currentScene = nullptr; // Or a default error scene
        break;
    }
    */

    currentSceneState = newState; // Update before initialize in case init fails or changes state again

    if (currentScene)
    {
        currentScene->Initialize();
    }
    else
    {
        // Log error: Failed to create the new scene.
        // Potentially fall back to a default scene or handle the error.
    }
}
