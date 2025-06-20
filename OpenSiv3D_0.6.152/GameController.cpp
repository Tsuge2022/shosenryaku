#include "GameController.h"
#include "MapManager.h"
#include "UnitManager.h"
#include "BuildingManager.h"
#include "TeamManager.h"
#include "GameUIManager.h"
#include "AIManager.h"
#include "SoundManager.h"
#include "InputManager.h"
#include "ProjectManager.h"
// #include "SceneManager.h" // If GameController uses a SceneManager instance

GameController::GameController(GameMode mode)
    : gameMode(mode), isPaused(false), winner(Teams::None) // Initialize winner to a default state
{
    // Constructor: Initialization of managers is deferred to Initialize()
    // to allow for potential error handling or order dependencies.
}

GameController::~GameController()
{
    // Destructor: unique_ptrs will automatically clean up manager instances.
    // If Shutdown() is not always called, ensure cleanup here or in Initialize() if re-initializing.
}

void GameController::Initialize()
{
    // Create instances of all managers
    inputManager = std::make_unique<InputManager>();
    mapManager = std::make_unique<MapManager>();
    unitManager = std::make_unique<UnitManager>();
    buildingManager = std::make_unique<BuildingManager>();
    teamManager = std::make_unique<TeamManager>();
    gameUIManager = std::make_unique<GameUIManager>();
    aiManager = std::make_unique<AIManager>();
    soundManager = std::make_unique<SoundManager>();
    projectManager = std::make_unique<ProjectManager>();
    // sceneManager = std::make_unique<SceneManager>(); // If used

    // Initialize managers (load data, set up initial states)
    // Order might be important here.
    // Example: projectManager->LoadGameSettings(...) might dictate map/unit files.
    // mapManager->LoadMap(...);
    // unitManager->LoadUnitDatabase(...);
    // buildingManager->LoadBuildingData(...); // For cities, factories, forts
    // teamManager->InitializeTeams(...); // Load initial team data, money, etc.
    // soundManager->LoadSounds(); already called in its constructor usually

    // Set initial game state
    isPaused = false;
    winner = Teams::None;
    // currentTeam = teamManager->GetCurrentTeamEnum(); // Or similar
}

void GameController::Update()
{
    if (inputManager) inputManager->Update(); // Update input manager first

    if (isPaused) {
        // Handle pause menu input or specific paused state updates
        return;
    }

    if (winner != Teams::None) {
        // Handle game over state (e.g., display winner, wait for input to return to menu)
        return;
    }

    HandlePlayerInput(); // Process input from the current human player

    if (gameMode == GameMode::CPUMode && teamManager && aiManager && teamManager->GetCurrentTeamEnum() != Teams::Blue /* assuming player is Blue */) {
         UpdateAI(); // If it's AI's turn
    }

    UpdateGameLogic();   // Update unit movements, ongoing battles, production, etc.
    CheckEndConditions(); // Check if a team has won

    if (gameUIManager) gameUIManager->UpdateDisplay(); // Update console/UI elements
}

void GameController::Draw() const
{
    if (mapManager) mapManager->DrawMap();
    if (buildingManager) buildingManager->DrawBuildings();
    if (unitManager) unitManager->DrawUnits();

    // Draw movement/attack ranges if a unit is selected (logic to be added)
    // if (mapManager && unitManager && unitManager->GetSelectedUnit()) {
    //    mapManager->DrawDrawableMapGrid(); // Assuming this draws pre-calculated ranges
    // }

    if (gameUIManager) {
        gameUIManager->DrawTurnInfo();
        gameUIManager->DrawMoneyInfo();
        gameUIManager->DrawDisplay(); // For console output
        // Draw other UI elements
    }

    if (isPaused) {
        // Draw pause menu
    }

    if (winner != Teams::None) {
        // Draw game over screen / winner declaration
    }
}

void GameController::Shutdown()
{
    // Clean up resources, save game state if necessary.
    // Managers are cleaned up by unique_ptr, but explicit shutdown actions can be called here.
    // Example: projectManager->SaveGameSettings(...);
}

void GameController::HandlePlayerInput()
{
    // Placeholder: Process input for selecting units, issuing commands, interacting with UI.
    // This will involve InputManager and call methods on other managers.
    // Example:
    // if (inputManager->IsMouseButtonClicked(MouseL)) {
    //    Point mousePos = inputManager->GetMousePosition(); // Convert to map coords
    //    // Try to select unit, or if unit selected, try to move/attack
    // }
}

void GameController::UpdateAI()
{
    if (aiManager && teamManager) {
        // aiManager->ThinkAndAct(teamManager->GetCurrentTeamEnum(), *this); // Pass GameController or individual managers
    }
}

void GameController::UpdateGameLogic()
{
    if (unitManager) unitManager->UpdateUnits(); // Handle animations, etc.
    if (buildingManager) buildingManager->UpdateBuildings(); // Handle production, etc.

    // Check for turn end conditions (e.g., all units moved/attacked, or player clicks "End Turn")
    // if (ShouldEndTurn()) {
    //    if (teamManager) teamManager->NextTurn();
    //    // Replenish resources, reset unit states for the new turn
    // }
}

void GameController::CheckEndConditions()
{
    // Placeholder: Check if any team has met victory conditions.
    // Example: All enemy HQs captured, or last enemy unit destroyed.
    // If so, set `winner`.
}

void GameController::RenderGame() const
{
    // This is essentially the Draw() method.
    // Kept for conceptual clarity if Draw() becomes more complex.
}
