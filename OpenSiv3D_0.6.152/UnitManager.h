#pragma once
#include "DataTypes.h" // Includes Unit, UnitData, UnitType, Teams, Point, etc.
#include <Siv3D.hpp>   // For Array, HashTable, String, ColorF, Vec2, etc.

// Forward declarations
class TeamManager;
class SoundManager;
class GameUIManager;
class BuildingManager; // Added for capture logic in UpdateUnitMovement

class UnitManager {
public:
    UnitManager();
    ~UnitManager();

    // Initialization and Data Loading
    bool LoadUnitStats(const FilePath& unitDataCSVPath);
    void LoadInitialUnits(const FilePath& unitSetDataCSVPath, const HashTable<Teams, Team>& teamList);

    // Unit Creation/Destruction
    Unit* CreateUnit(UnitType type, Point pos, Teams team, ColorF color);
    void DestroyUnit(Unit* unit);

    // Unit State and Information
    const Array<Unit*>& GetUnitList() const;
    const HashTable<UnitType, UnitData>& GetUnitDB() const;
    Unit* GetUnitAt(Point pos) const;

    // Unit Actions & Logic
    // UpdateUnits might not be needed if all updates are handled by specific action methods.
    // void UpdateUnits(const Vec2& camPos, double scale, double size, SoundManager* soundManager, GameUIManager* gameUIManager, TeamManager* teamManager);
    void DrawUnits(const Vec2& camPos, double scale, double size, const Font& nameFont, const Unit* targetUnit) const;

    // Movement
    void StartUnitMovement(Unit* unit, const Array<Point>& route);
    bool UpdateUnitMovement(double timeDelta, double moveTimePerTile, const Vec2& camPosIn, Vec2& camPosOut, double currentMapWidthScale, SoundManager* soundManager, BuildingManager* buildingManager, GameUIManager* gameUIManager, TeamManager* teamManager); // Modified parameters

    // Combat
    void ExecuteBattle(Unit* attacker, Unit* defender, SoundManager* soundManager, GameUIManager* gameUIManager, TeamManager* teamManager);

    // Turn-based state changes
    void ResetUnitActionsForTeam(Teams team);

    Unit* GetMovingUnit() const; // Helper to know if a unit is currently in animated movement

private:
    Array<Unit*> unitList;
    HashTable<UnitType, UnitData> unitDB;

    struct UnitMovementState {
        Unit* unit = nullptr;
        Array<Point> route;
        int currentLegIndex = 0;
        double legTimer = 0.0;
        // Vec2 visualOffset; // drawPos in Unit struct can serve this purpose.
        Point nextTilePosition; // The immediate next tile the unit is moving towards in the current leg.
    };
    UnitMovementState activeMovement;
};
