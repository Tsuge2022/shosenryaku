#pragma once
#include "DataTypes.h" // For map, unit, team structures etc.
#include <Siv3D.hpp>   // For FilePath, CSV, JSON etc.

// Forward declare manager classes if ProjectManager needs to interact with their data directly
// class MapManager;
// class UnitManager;
// class BuildingManager;
// class TeamManager;
// Or, pass data structures (like Grid<int32> for map) to save/load functions.

class ProjectManager
{
public:
    ProjectManager();  // Constructor
    ~ProjectManager(); // Destructor

    // Map Data
    bool SaveMapData(const FilePath& filePath, const Grid<int32>& mapGrid, const HashTable<int32, Chip>& chipData);
    bool LoadMapData(const FilePath& filePath, Grid<int32>& mapGrid, HashTable<int32, Chip>& chipData);
    // Overload for saving/loading from specific manager instances if preferred
    // bool SaveMapData(const FilePath& filePath, const MapManager& mapManager);
    // bool LoadMapData(const FilePath& filePath, MapManager& mapManager);


    // Unit Data (Unit definitions/database and potentially placed units in a scenario)
    bool SaveUnitDefinitions(const FilePath& filePath, const HashTable<UnitType, UnitData>& unitDB);
    bool LoadUnitDefinitions(const FilePath& filePath, HashTable<UnitType, UnitData>& unitDB);

    bool SavePlacedUnits(const FilePath& filePath, const Array<Unit*>& unitList); // Careful with pointers; usually save essential data
    bool LoadPlacedUnits(const FilePath& filePath, Array<Unit*>& unitList, const HashTable<UnitType, UnitData>& unitDB);


    // Building Data (similar to units, definitions and placed instances)
    bool SavePlacedCities(const FilePath& filePath, const Array<City*>& cityList);
    bool LoadPlacedCities(const FilePath& filePath, Array<City*>& cityList);

    bool SavePlacedFactories(const FilePath& filePath, const Array<Factory*>& factoryList);
    bool LoadPlacedFactories(const FilePath& filePath, Array<Factory*>& factoryList);

    bool SavePlacedForts(const FilePath& filePath, const Array<Fort*>& fortList);
    bool LoadPlacedForts(const FilePath& filePath, Array<Fort*>& fortList);


    // Game Settings / Scenario Data
    // This could encompass initial team money, turn limits, win conditions, which map to load, etc.
    // For simplicity, using a generic approach. Could use JSON or custom CSV format.
    struct GameSettings // Example structure
    {
        FilePath mapFile;
        FilePath unitFile; // For placed units
        // FilePath cityFile; // etc.
        HashTable<Teams, int> initialMoney;
        Teams startingTeam;
        int gameTurnLimit;
        // ... other settings
    };
    bool SaveGameSettings(const FilePath& filePath, const GameSettings& settings);
    bool LoadGameSettings(const FilePath& filePath, GameSettings& settings);

    // Save/Load entire game state (snapshot for save/load game feature)
    // This is more complex and would involve serializing state from multiple managers.
    // bool SaveGameState(const FilePath& filePath, const GameController& gameController); // Example
    // bool LoadGameState(const FilePath& filePath, GameController& gameController); // Example

private:
    // Helper methods for CSV/JSON parsing or other file operations.
    // For example, converting Unit* to a savable format and back.
};
