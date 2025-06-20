#pragma once
#include "DataTypes.h" // For GameMode, Teams, etc. if needed directly by controller logic

// Forward declarations for all game-specific managers
// These prevent circular dependencies if managers also need to know about GameController (though less common)
class MapManager;
class UnitManager;
class BuildingManager;
class TeamManager;
class GameUIManager;
class AIManager;
class SoundManager;
class InputManager; // Input can also be a game-specific manager
class ProjectManager; // If game controller handles saving/loading operations directly
class SceneManager;   // If GameController is responsible for the main game scene's lifecycle

class GameController
{
public:
    GameController(GameMode mode); // Constructor might take initial game mode
    ~GameController();             // Destructor to clean up managers

    void Initialize(); // Initialize all game systems and load initial data
    void Update();     // Main game loop update function
    void Draw() const; // Main game loop draw function
    void Shutdown();   // Clean up resources before exiting or changing to a different major state (like main menu)

    // Accessors for managers if other systems (e.g. a global event system) need them.
    // Generally, try to keep dependencies minimal.
    MapManager* GetMapManager() const { return mapManager.get(); }
    UnitManager* GetUnitManager() const { return unitManager.get(); }
    BuildingManager* GetBuildingManager() const { return buildingManager.get(); }
    TeamManager* GetTeamManager() const { return teamManager.get(); }
    GameUIManager* GetGameUIManager() const { return gameUIManager.get(); }
    AIManager* GetAIManager() const { return aiManager.get(); }
    SoundManager* GetSoundManager() const { return soundManager.get(); }
    InputManager* GetInputManager() const { return inputManager.get(); }
    ProjectManager* GetProjectManager() const { return projectManager.get(); }
    // SceneManager is not typically part of GameController if GameController *is* a scene.
    // If GameController manages scenes itself (like mini-scenes within the game state), then it's fine.

private:
    GameMode gameMode;

    // Pointers or smart pointers to all the game-specific managers
    // Using unique_ptr for automatic memory management.
    std::unique_ptr<MapManager> mapManager;
    std::unique_ptr<UnitManager> unitManager;
    std::unique_ptr<BuildingManager> buildingManager;
    std::unique_ptr<TeamManager> teamManager;
    std::unique_ptr<GameUIManager> gameUIManager;
    std::unique_ptr<AIManager> aiManager;
    std::unique_ptr<SoundManager> soundManager;
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<ProjectManager> projectManager;
    // std::unique_ptr<SceneManager> sceneManager; // If GameController owns the game scene.

    // Internal game state variables
    bool isPaused;
    Teams winner; // Or some other way to track game end state

    // Private helper methods for the game loop phases
    void HandlePlayerInput();
    void UpdateAI();
    void UpdateGameLogic();
    void CheckEndConditions();
    void RenderGame() const;
};
