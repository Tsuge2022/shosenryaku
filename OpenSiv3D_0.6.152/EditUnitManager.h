#pragma once
#include "DataTypes.h" // Includes Unit, UnitData, UnitType, Teams, Point, etc.
#include <Siv3D.hpp>

class EditUnitManager {
public:
    EditUnitManager();
    ~EditUnitManager();

    // Data Loading/Saving
    void LoadUnitStats(const FilePath& unitDataPath);
    void SaveUnitStats(const FilePath& unitDataPath) const;

    void LoadUnitPlacements(const FilePath& unitPlacementPath, const HashTable<Teams, Team>& teamList);
    void SaveUnitPlacements(const FilePath& unitPlacementPath) const;

    // Unit Definitions (stats from UnitData.csv)
    const HashTable<UnitType, UnitData>& GetUnitDefinitions() const;
    UnitData* GetUnitDefinition(UnitType type);

    // Unit Instances on Map
    const Array<Unit*>& GetPlacedUnits() const;
    void PlaceUnit(Point mapCoords); // Uses selectedType and selectedTeam
    void RemoveUnitAt(Point mapCoords);
    Unit* GetUnitAt(Point mapCoords);

    // Drawing (for units being edited on the map)
    void DrawUnitsOnMap(const Vec2& camPos, double scale, double currentTileSize, const Font& nameFont, const HashTable<Teams, Team>& teamDataList) const;

    // Selection for placement
    UnitType GetSelectedUnitType() const;
    void SetSelectedUnitType(UnitType type);
    Teams GetSelectedTeam() const;
    void SetSelectedTeam(Teams team);


private:
    Array<Unit*> placedUnits;
    HashTable<UnitType, UnitData> unitDefinitions;
    UnitType selectedUnitTypeForPlacement;
    Teams selectedTeamForPlacement;
};
