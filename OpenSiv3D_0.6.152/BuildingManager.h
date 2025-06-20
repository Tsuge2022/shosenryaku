#pragma once
#include "DataTypes.h" // Includes City, Factory, Fort, Teams, UnitType, etc.
#include <Siv3D.hpp>   // For Array, HashTable, Font, etc.

// Forward declarations
class TeamManager;
class UnitManager;
class Unit;

class BuildingManager {
public:
    BuildingManager();
    ~BuildingManager();

    // Data Loading
    void LoadCities(const FilePath& cityCSVPath, const HashTable<Teams, Team>& teamList);
    // Factories might be hardcoded or from a scenario file, not just a simple CSV in the original.
    // For now, let's assume a method to initialize them based on potential future CSV or hardcoding.
    void InitializeFactories(const HashTable<Teams, Team>& teamList, const HashTable<UnitType, UnitData>& unitDB); // Placeholder for factory setup
    void LoadForts(const FilePath& fortCSVPath, const HashTable<Teams, Team>& teamList);

    // Getters
    const Array<City*>& GetCityList() const;
    const Array<Factory*>& GetFactoryList() const;
    const Array<Fort*>& GetFortList() const;
    Factory* GetFactoryAt(Point pos) const;
    City* GetCityAt(Point pos) const;
    Fort* GetFortAt(Point pos) const;

    // Building Actions & Logic
    void UpdateBuildings();
    void DrawCities(const Vec2& camPos, double scale, double size, const Font& nameFont) const;
    void DrawFactories(const Vec2& camPos, double scale, double size, const Font& factoryFont, const HashTable<UnitType, UnitData>& unitDB) const;
    void DrawForts(const Vec2& camPos, double scale, double size, const Font& nameFont) const;

    // Interactions
    void CaptureCity(City* city, Unit* capturingUnit);
    void CaptureFort(Fort* fort, Unit* capturingUnit); // Fort team doesn't change, only isCapture flag.
    bool ProduceUnitFromFactory(Factory* factory, TeamManager* teamManager, UnitManager* unitManager);

    // End of turn updates
    void AwardIncomeFromCities(TeamManager* teamManager);

    // Game end condition
    Teams CheckFortVictoryCondition() const;

private:
    Array<City*> cityList;
    Array<Factory*> factoryList;
    Array<Fort*> fortList;
};
