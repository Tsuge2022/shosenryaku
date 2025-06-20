#pragma once
#include "DataTypes.h"
#include <Siv3D.hpp>

class TeamManager;
class UnitManager;

enum class EditableBuildingType { None, City, Fort, Factory };

class EditBuildingManager {
public:
    EditBuildingManager();
    ~EditBuildingManager();

    void LoadBuildingData(
        const FilePath& cityCSVPath,
        const FilePath& fortCSVPath,
        // Factories are initialized programmatically for now, like in Game.cpp
        const HashTable<Teams, Team>& teamList // For team colors
    );
    void SaveBuildingData(
        const FilePath& cityCSVPath,
        const FilePath& fortCSVPath
        // Factories save if they become data-driven
    ) const;

    // City Management
    const Array<City>& GetCityDefinitions() const; // City "types" or templates
    City* GetCityDefinition(int index); // Get a specific city definition for editing
    void AddNewCityDefinition(const String& name = U"New City", Teams team = Teams::None, int plusMoney = 100);
    void RemoveCityDefinition(int index);
    bool IsCityPlaced(int definitionIndex) const;
    void PlaceCityInstance(int definitionIndex, Point mapCoords); // Places instance of city definition
    void RemoveCityInstance(Point mapCoords); // Removes instance, makes definition available again
    City* GetPlacedCityAt(Point mapCoords); // Gets the definition of the city at mapCoords

    // Fort Management
    const Array<Fort*>& GetPlacedForts() const; // Forts are directly placed instances
    Fort* GetPlacedFortAt(Point mapCoords);
    void PlaceFortInstance(Teams team, Point mapCoords, const HashTable<Teams, Team>& teamList);
    void RemoveFortInstance(Point mapCoords);

    // Factory Management (simplified: placed directly, not from definitions list for now)
    const Array<Factory*>& GetPlacedFactories() const;
    Factory* GetPlacedFactoryAt(Point mapCoords);
    void PlaceFactoryInstance(Teams team, UnitType unitType, Point mapCoords, const HashTable<Teams, Team>& teamList, int cost = 300);
    void RemoveFactoryInstance(Point mapCoords);


    // Drawing
    void DrawBuildingsOnMap(const Vec2& camPos, double scale, double currentTileSize, const Font& nameFont, const HashTable<UnitType, UnitData>* unitDBForFactoryNames, const HashTable<Teams, Team>& teamDataList) const;

    // Drag and Drop state - managed by EditController/UIManager, but manager can hold the object
    City* cityBeingDragged = nullptr;     // Points to an object in cityDefinitions
    Fort* fortBeingDragged = nullptr;     // Points to an object in placedForts
    Factory* factoryBeingDragged = nullptr; // Points to an object in placedFactories
    Point originalDragPos; // To revert if drag is cancelled or invalid

    // Selection for UI palette (using indices for definitions, or direct pointers for placed instances)
    int selectedCityDefinitionIndex = -1;
    // Forts and Factories might be selected directly from placed instances if no separate definitions are used for them in UI
    EditableBuildingType selectedBuildingTypeForPlacement = EditableBuildingType::None;
    Teams selectedTeamForPlacement = Teams::Red; // Default team for new buildings
    UnitType selectedUnitTypeForFactory = UnitType::Tank; // Default unit for new factory


private:
    Array<City> cityDefinitions; // Editable templates for cities
    Array<bool> cityDefinitionIsPlaced; // Tracks if city definition is used on map

    Array<Fort*> placedForts;     // Direct instances
    Array<Factory*> placedFactories; // Direct instances
};
