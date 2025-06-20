#include "EditBuildingManager.h"
#include "TeamManager.h" // Not strictly needed if team data comes via HashTable
#include "UnitManager.h" // For UnitType

EditBuildingManager::EditBuildingManager() {
    // Initialize default selections or leave empty until data is loaded.
    selectedCityDefinitionIndex = -1;
}

EditBuildingManager::~EditBuildingManager() {
    // cityDefinitions are objects, not pointers, so managed by Array.
    cityDefinitions.clear();
    cityDefinitionIsPlaced.clear();

    for (Fort* fort : placedForts) { delete fort; }
    placedForts.clear();
    for (Factory* factory : placedFactories) { delete factory; }
    placedFactories.clear();
}

void EditBuildingManager::LoadBuildingData(
    const FilePath& cityCSVPath,
    const FilePath& fortCSVPath,
    const HashTable<Teams, Team>& teamList) {

    // Load Cities (as definitions)
    cityDefinitions.clear();
    cityDefinitionIsPlaced.clear();
    CSV csvCities(cityCSVPath);
    if (csvCities) {
        for (size_t i = 1; i < csvCities.rows(); ++i) { // Skip header
            City city;
            city.name = csvCities[i][0];
            city.team = static_cast<Teams>(Parse<int>(csvCities[i][1]));
            if (teamList.contains(city.team)) {
                city.color = teamList.at(city.team).color;
            } else {
                city.color = Palette::Lightgray; // Default for Teams::None or missing
            }
            city.plusMoney = Parse<int>(csvCities[i][2]);
            city.pos = Parse<Point>(csvCities[i][3]); // Position from CSV might be its placed pos

            bool isPlaced = Parse<bool>(csvCities[i][4]); // 'setCities' from original Edit

            cityDefinitions.push_back(city);
            cityDefinitionIsPlaced.push_back(isPlaced);
            if(isPlaced) {
                // If it's marked as placed and has a valid position, it's an "instance"
                // The model here is that cityDefinitions holds ALL potential cities,
                // and their .pos and the flag cityDefinitionIsPlaced determine if they are active on map.
            } else {
                 cityDefinitions.back().pos = Point{-1,-1}; // Mark as not on map
            }
        }
    }
     if (cityDefinitions.isEmpty()) { // Ensure at least one definition
        AddNewCityDefinition(U"Default City", Teams::None, 0);
    }


    // Load Forts (as direct instances)
    for (Fort* fort : placedForts) { delete fort; }
    placedForts.clear();
    CSV csvForts(fortCSVPath);
    if (csvForts) {
        for (size_t i = 1; i < csvForts.rows(); ++i) { // Skip header
            Fort* f = new Fort();
            f->team = static_cast<Teams>(Parse<int>(csvForts[i][0]));
            if (teamList.contains(f->team)) {
                f->color = teamList.at(f->team).color;
            } else {
                 f->color = Palette::Darkgray;
            }
            f->pos = Parse<Point>(csvForts[i][1]);
            f->isCapture = false; // Default for editor
            placedForts.push_back(f);
        }
    }

    // Initialize Factories (example hardcoded from Game.cpp, as Edit.cpp didn't manage them)
    for (Factory* factory : placedFactories) { delete factory; }
    placedFactories.clear();
    if (teamList.contains(Teams::Red)) {
        Factory* f1 = new Factory();
        f1->makeUnit = UnitType::Tank; f1->pos = Point(2,3); f1->gold = 300; f1->team = Teams::Red; f1->color = teamList.at(Teams::Red).color;
        placedFactories.push_back(f1);
    }
    if (teamList.contains(Teams::Blue)) {
        Factory* f2 = new Factory();
        f2->makeUnit = UnitType::Heli; f2->pos = Point(8,3); f2->gold = 300; f2->team = Teams::Blue; f2->color = teamList.at(Teams::Blue).color;
        placedFactories.push_back(f2);
    }
}

void EditBuildingManager::SaveBuildingData(
    const FilePath& cityCSVPath,
    const FilePath& fortCSVPath) const {

    CSV csvCities;
    csvCities.writeRow(U"名前", U"チーム", U"お金", U"位置", U"配置するかどうか");
    for (size_t i = 0; i < cityDefinitions.size(); ++i) {
        const auto& city = cityDefinitions[i];
        csvCities.writeRow(city.name, static_cast<int>(city.team), city.plusMoney, city.pos, cityDefinitionIsPlaced[i]);
    }
    csvCities.save(cityCSVPath);

    CSV csvForts;
    csvForts.writeRow(U"チーム", U"位置");
    for (const auto* fort : placedForts) {
        if(fort) csvForts.writeRow(static_cast<int>(fort->team), fort->pos);
    }
    csvForts.save(fortCSVPath);

    // Factories saving would go here if they become data-driven
}

// City Management
const Array<City>& EditBuildingManager::GetCityDefinitions() const { return cityDefinitions; }

City* EditBuildingManager::GetCityDefinition(int index) {
    if (cityDefinitions.inBounds(index)) {
        return &cityDefinitions[index];
    }
    return nullptr;
}

void EditBuildingManager::AddNewCityDefinition(const String& name, Teams team, int plusMoney) {
    cityDefinitions.emplace_back(City{name, Point{-1,-1}, plusMoney, Palette::White, false, team}); // Pos (-1,-1) means not placed
    cityDefinitionIsPlaced.push_back(false);
}

void EditBuildingManager::RemoveCityDefinition(int index) {
    if (cityDefinitions.inBounds(index)) {
        cityDefinitions.remove_at(index);
        cityDefinitionIsPlaced.remove_at(index);
        if (selectedCityDefinitionIndex == index) selectedCityDefinitionIndex = -1;
        else if (selectedCityDefinitionIndex > index) selectedCityDefinitionIndex--;
    }
}

bool EditBuildingManager::IsCityPlaced(int definitionIndex) const {
    if (cityDefinitionIsPlaced.inBounds(definitionIndex)) {
        return cityDefinitionIsPlaced[definitionIndex];
    }
    return false;
}

void EditBuildingManager::PlaceCityInstance(int definitionIndex, Point mapCoords) {
    if (cityDefinitions.inBounds(definitionIndex)) {
        // Ensure no other city (definition or instance) is at mapCoords
        RemoveCityInstance(mapCoords);

        cityDefinitions[definitionIndex].pos = mapCoords;
        cityDefinitionIsPlaced[definitionIndex] = true;
    }
}

void EditBuildingManager::RemoveCityInstance(Point mapCoords) {
    for (size_t i = 0; i < cityDefinitions.size(); ++i) {
        if (cityDefinitionIsPlaced[i] && cityDefinitions[i].pos == mapCoords) {
            cityDefinitions[i].pos = Point{-1, -1}; // Mark as not on map
            cityDefinitionIsPlaced[i] = false;
            return;
        }
    }
}

City* EditBuildingManager::GetPlacedCityAt(Point mapCoords) {
    for (size_t i = 0; i < cityDefinitions.size(); ++i) {
        if (cityDefinitionIsPlaced[i] && cityDefinitions[i].pos == mapCoords) {
            return &cityDefinitions[i];
        }
    }
    return nullptr;
}


// Fort Management
const Array<Fort*>& EditBuildingManager::GetPlacedForts() const { return placedForts; }

Fort* EditBuildingManager::GetPlacedFortAt(Point mapCoords) {
    for (Fort* f : placedForts) { if (f && f->pos == mapCoords) return f; }
    return nullptr;
}

void EditBuildingManager::PlaceFortInstance(Teams team, Point mapCoords, const HashTable<Teams, Team>& teamList) {
    RemoveFortInstance(mapCoords); // Remove if one exists
    Fort* newFort = new Fort();
    newFort->pos = mapCoords;
    newFort->team = team;
    newFort->isCapture = false;
    if(teamList.contains(team)){ newFort->color = teamList.at(team).color; }
    else {newFort->color = Palette::Darkgray;}
    placedForts.push_back(newFort);
}

void EditBuildingManager::RemoveFortInstance(Point mapCoords) {
    placedForts.remove_if([mapCoords](Fort* f){
        if(f && f->pos == mapCoords) { delete f; return true; }
        return false;
    });
}

// Factory Management
const Array<Factory*>& EditBuildingManager::GetPlacedFactories() const { return placedFactories; }

Factory* EditBuildingManager::GetPlacedFactoryAt(Point mapCoords) {
    for (Factory* f : placedFactories) { if (f && f->pos == mapCoords) return f; }
    return nullptr;
}

void EditBuildingManager::PlaceFactoryInstance(Teams team, UnitType unitType, Point mapCoords, const HashTable<Teams, Team>& teamList, int cost) {
    RemoveFactoryInstance(mapCoords); // Remove if one exists
    Factory* newFactory = new Factory();
    newFactory->pos = mapCoords;
    newFactory->team = team;
    newFactory->makeUnit = unitType;
    newFactory->gold = cost;
    if(teamList.contains(team)){ newFactory->color = teamList.at(team).color; }
    else { newFactory->color = Palette::Darkgray; }
    placedFactories.push_back(newFactory);
}

void EditBuildingManager::RemoveFactoryInstance(Point mapCoords) {
     placedFactories.remove_if([mapCoords](Factory* f){
        if(f && f->pos == mapCoords) { delete f; return true; }
        return false;
    });
}


void EditBuildingManager::DrawBuildingsOnMap(const Vec2& camPos, double scale, double currentTileSize, const Font& nameFont, const HashTable<UnitType, UnitData>* unitDBForFactoryNames, const HashTable<Teams, Team>& teamDataList) const {
    const double hexWidth = currentTileSize * scale;

    // Draw Cities
    for (size_t i = 0; i < cityDefinitions.size(); ++i) {
        if (cityDefinitionIsPlaced[i]) {
            const auto& city = cityDefinitions[i];
            Shape2D r;
            if (city.pos.y % 2 == 0) { r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * city.pos.x + camPos.x, hexWidth * 1.5 * city.pos.y + camPos.y)); }
            else { r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * city.pos.x + hexWidth * Math::Sqrt3 / 2 + camPos.x, hexWidth * 1.5 * city.pos.y + camPos.y)); }

            ColorF displayColor = city.color;
            if(city.team != Teams::None && teamDataList.contains(city.team)){ // Update color if team changed
                displayColor = teamDataList.at(city.team).color;
            } else if (city.team == Teams::None) {
                 displayColor = Palette::Lightgray;
            }

            r.drawFrame(3, displayColor);
            r.draw(Palette::White);
            if (scale >= 0.5) nameFont(city.name).draw(Arg::center = r.asPolygon().centroid(), Palette::Black);
        }
    }

    // Draw Forts
    for (const auto* fort : placedForts) {
        if (!fort) continue;
        Shape2D r;
        if (fort->pos.y % 2 == 0) { r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * fort->pos.x + camPos.x, hexWidth * 1.5 * fort->pos.y + camPos.y)); }
        else { r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * fort->pos.x + hexWidth * Math::Sqrt3 / 2 + camPos.x, hexWidth * 1.5 * fort->pos.y + camPos.y)); }

        ColorF displayColor = fort->color;
        if(fort->team != Teams::None && teamDataList.contains(fort->team)){
             displayColor = teamDataList.at(fort->team).color;
        } else if (fort->team == Teams::None) {
            displayColor = Palette::Darkgray;
        }
        r.draw(Palette::White);
        r.drawFrame(2, displayColor);
        if (scale >= 0.5) nameFont(U"砦").draw(Arg::center = r.asPolygon().centroid(), Palette::Black);
    }

    // Draw Factories
    if (unitDBForFactoryNames) { // Ensure unitDB is provided for names
        for (const auto* factory : placedFactories) {
            if (!factory) continue;
            Shape2D r;
            if (factory->pos.y % 2 == 0) { r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * factory->pos.x + camPos.x, hexWidth * 1.5 * factory->pos.y + camPos.y)); }
            else { r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * factory->pos.x + hexWidth * Math::Sqrt3 / 2 + camPos.x, hexWidth * 1.5 * factory->pos.y + camPos.y)); }

            ColorF displayColor = factory->color;
            if(factory->team != Teams::None && teamDataList.contains(factory->team)){
                displayColor = teamDataList.at(factory->team).color;
            } else if (factory->team == Teams::None) {
                displayColor = Palette::Darkgray;
            }
            r.draw(Palette::Gainsboro);
            r.drawFrame(2, displayColor);
            if (scale >= 0.5 && unitDBForFactoryNames->contains(factory->makeUnit)) {
                nameFont(unitDBForFactoryNames->at(factory->makeUnit).name.substr(0,1) + U"工場").draw(Arg::center = r.asPolygon().centroid(), Palette::Black);
            } else if (scale >= 0.5) {
                 nameFont(U"工場").draw(Arg::center = r.asPolygon().centroid(), Palette::Black);
            }
        }
    }
}


int EditBuildingManager::GetSelectedCityDefinitionIndex() const { return selectedCityDefinitionIndex; }
void EditBuildingManager::SetSelectedCityDefinitionIndex(int index) {
    if (index >= -1 && index < static_cast<int>(cityDefinitions.size())) {
        selectedCityDefinitionIndex = index;
    }
}
City* EditBuildingManager::GetCityDefinitionForEdit(int index) {
    if (cityDefinitions.inBounds(index)) {
        return &cityDefinitions[index];
    }
    return nullptr;
}

// SetSelectedBuildingForPlacement methods are conceptual for EditController to manage which building type is active.
// Actual selected instances for editing properties would be direct pointers.
// The manager itself doesn't need these SetSelectedBuildingForPlacement methods as much as EditController does.
// Instead, it provides access to its building data for EditUIManager to display and EditController to manage selection state.
