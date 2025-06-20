#pragma once
#include "DataTypes.h"
#include <Siv3D.hpp>

// Forward Declarations
class UnitManager;
class MapManager;
class BuildingManager;
class TeamManager;
class GameController;
class SoundManager; // Added, as AI actions might trigger sounds indirectly (though usually via Unit/BuildingManager)
class GameUIManager; // Added, as AI actions might generate console messages indirectly

class AIManager {
public:
    AIManager();

    void PrepareForTurn(Teams aiTeam, MapManager* mapManager, BuildingManager* buildingManager); // New: To calculate strategic grids like cpuDistanceRangeGrid
    bool ThinkAndAct(
        Teams aiTeam,
        MapManager* mapManager,
        UnitManager* unitManager,
        BuildingManager* buildingManager,
        TeamManager* teamManager,
        GameUIManager* gameUIManager, // For potential direct AI messages, though less common
        SoundManager* soundManager   // For potential direct AI sounds, less common
    );

private:
    enum class AIState { IDLE, PRODUCING_UNITS, MOVING_UNITS, ATTACKING_UNITS, TURN_ENDED };
    AIState currentState;
    bool productionPhaseDone;
    bool actionsPhaseDone; // True if all units have tried to move/attack

    Grid<int32> cpuDistanceRangeGrid;
    Grid<int32> evaluationGrid;

    void InitializeCPUDistanceGrid(Size mapSize);
    void InitializeEvaluationGrid(Size mapSize);

    void DecideUnitProduction(Teams aiTeam, BuildingManager* buildingManager, UnitManager* unitManager, TeamManager* teamManager, GameUIManager* gameUIManager);
    // DecideUnitActions now tries to perform one action (move or attack) for one unit.
    // Returns true if an action was initiated (unit started moving or an attack was made).
    bool AttemptUnitAction(Teams aiTeam, MapManager* mapManager, UnitManager* unitManager, BuildingManager* buildingManager, TeamManager* teamManager, GameUIManager* gameUIManager, SoundManager* soundManager);

    void CalculateCPUDistanceRecursive(Point currentPos, int32 currentCost, const Grid<int32>& mapLayout, const HashTable<int32, Chip>& chipData);
    int EvaluatePosition(Point targetPos, Unit* forUnit, Teams aiTeam, MapManager* mapManager, UnitManager* unitManager, BuildingManager* buildingManager, const Grid<int32>& threatMap);

    // Helper to build a threat map based on enemy unit capabilities
    Grid<int32> CreateThreatMap(Teams aiTeam, MapManager* mapManager, UnitManager* unitManager);

    // Pathfinding: AI needs a path to the chosen target tile.
    // This could call a service in MapManager or UnitManager.
    // For now, assume AI picks a target, and UnitManager handles pathing for StartUnitMovement.
    // Array<Point> FindPath(Point start, Point end, int32 movePower, Unit* unit, MapManager* mapManager, UnitManager* unitManager);
};
