#include "EditUnitManager.h"

EditUnitManager::EditUnitManager()
    : selectedUnitTypeForPlacement(UnitType::Tank), selectedTeamForPlacement(Teams::Red) {
    // Initialize with default selections. Actual stats loaded by LoadUnitStats.
}

EditUnitManager::~EditUnitManager() {
    for (Unit* unit : placedUnits) {
        delete unit;
    }
    placedUnits.clear();
}

void EditUnitManager::LoadUnitStats(const FilePath& unitDataPath) {
    unitDefinitions.clear();
    CSV csv(unitDataPath);
    if (!csv) {
        Logger << U"Failed to load UnitData.csv from: " << unitDataPath;
        // Create default stats if file fails to load
        unitDefinitions.emplace(UnitType::Tank, UnitData{100, U"Tank", 10, 5, 2, UnitType::Heli});
        unitDefinitions.emplace(UnitType::Heli, UnitData{80, U"Heli", 8, 7, 3, UnitType::Human});
        unitDefinitions.emplace(UnitType::Human, UnitData{50, U"Human", 5, 4, 1, UnitType::Tank});
        return;
    }

    for (size_t i = 1; i < csv.rows(); ++i) { // Skip header row
        UnitData u;
        u.name = csv[i][0];
        u.range = Parse<int>(csv[i][1]);
        u.speed = Parse<int>(csv[i][2]);
        u.advUnit = static_cast<UnitType>(Parse<int>(csv[i][3]));
        // Assuming HP and Power are not in this CSV based on original Edit::Edit and Game::Game structure.
        // We should add them to CSV or define defaults here. For now, using defaults.
        u.hp = 100; // Default HP
        u.power = 10; // Default Power

        // The UnitType enum order is Tank, Heli, Human. CSV rows (i-1) correspond to this.
        if ((i - 1) < static_cast<size_t>(UnitType::Max)) {
             unitDefinitions.emplace(static_cast<UnitType>(i - 1), u);
        }
    }
     if (unitDefinitions.isEmpty()){ // Fallback if CSV was empty or only header
        unitDefinitions.emplace(UnitType::Tank, UnitData{100, U"Tank", 10, 5, 2, UnitType::Heli});
    }
}

void EditUnitManager::SaveUnitStats(const FilePath& unitDataPath) const {
    CSV csv;
    csv.writeRow(U"名前", U"攻撃範囲", U"移動距離", U"有利タイプ"); // Header from original Edit::DrawGUI
    // Iterate in enum order for consistency if possible, though HashTable doesn't guarantee order.
    // For saving, it's better to iterate based on the enum values if they are contiguous.
    for (int i = 0; i < static_cast<int>(UnitType::Max); ++i) {
        UnitType type = static_cast<UnitType>(i);
        if (unitDefinitions.contains(type)) {
            const UnitData& data = unitDefinitions.at(type);
            csv.writeRow(data.name, data.range, data.speed, static_cast<int>(data.advUnit));
        }
    }
    csv.save(unitDataPath);
}

void EditUnitManager::LoadUnitPlacements(const FilePath& unitPlacementPath, const HashTable<Teams, Team>& teamList) {
    for (Unit* unit : placedUnits) { delete unit; }
    placedUnits.clear();

    CSV csv(unitPlacementPath);
    if (!csv) {
        Logger << U"Failed to load UnitSetData.csv from: " << unitPlacementPath;
        return;
    }

    for (size_t i = 1; i < csv.rows(); ++i) { // Skip header
        Teams teamId = static_cast<Teams>(Parse<int>(csv[i][0]));
        UnitType type = static_cast<UnitType>(Parse<int>(csv[i][1]));
        Point pos = Parse<Point>(csv[i][2]);

        ColorF color = Palette::White; // Default
        if (teamList.contains(teamId)) {
            color = teamList.at(teamId).color;
        }

        if (unitDefinitions.contains(type)) {
            Unit* newUnit = new Unit();
            newUnit->type = type;
            newUnit->pos = pos;
            newUnit->team = teamId;
            newUnit->color = color;
            newUnit->hp = unitDefinitions.at(type).hp; // Set HP from definitions
            // Initialize other members like drawPos, prePos, isMove, isAttack as needed for editor display
            newUnit->drawPos = Vec2(pos.x, pos.y);
            newUnit->prePos = pos;
            newUnit->isMove = false;
            newUnit->isAttack = false;
            placedUnits.push_back(newUnit);
        }
    }
}

void EditUnitManager::SaveUnitPlacements(const FilePath& unitPlacementPath) const {
    CSV csv;
    csv.writeRow(U"チーム", U"種類", U"位置"); // Header from original Edit::DrawGUI
    for (const auto* unit : placedUnits) {
        if (unit) {
            csv.writeRow(static_cast<int>(unit->team), static_cast<int>(unit->type), unit->pos);
        }
    }
    csv.save(unitPlacementPath);
}


const HashTable<UnitType, UnitData>& EditUnitManager::GetUnitDefinitions() const {
    return unitDefinitions;
}

UnitData* EditUnitManager::GetUnitDefinition(UnitType type) {
    if (unitDefinitions.contains(type)) {
        return &unitDefinitions[type];
    }
    return nullptr;
}

const Array<Unit*>& EditUnitManager::GetPlacedUnits() const {
    return placedUnits;
}

void EditUnitManager::PlaceUnit(Point mapCoords) {
    RemoveUnitAt(mapCoords); // Remove existing unit at the location first

    if (unitDefinitions.contains(selectedUnitTypeForPlacement)) {
        const UnitData& stats = unitDefinitions.at(selectedUnitTypeForPlacement);
        Unit* newUnit = new Unit();
        newUnit->type = selectedUnitTypeForPlacement;
        newUnit->pos = mapCoords;
        newUnit->team = selectedTeamForPlacement;
        // Color should be determined by team, need TeamManager access or pass teamColors
        // For now, use a placeholder or selected team's default color if available
        // This implies teamDataList needs to be passed or accessed.
        // For simplicity in this step, let's use a basic color.
        // newUnit->color = teamColors.contains(selectedTeamForPlacement) ? teamColors.at(selectedTeamForPlacement).color : Palette::White;
        newUnit->color = (selectedTeamForPlacement == Teams::Red) ? Palette::Red : Palette::Blue; // Placeholder
        newUnit->hp = stats.hp;
        newUnit->drawPos = Vec2(mapCoords.x, mapCoords.y);
        newUnit->prePos = mapCoords;
        newUnit->isMove = false;
        newUnit->isAttack = false;
        placedUnits.push_back(newUnit);
    }
}

void EditUnitManager::RemoveUnitAt(Point mapCoords) {
    size_t oldSize = placedUnits.size();
    placedUnits.remove_if([mapCoords](Unit* u) {
        if (u && u->pos == mapCoords) {
            delete u;
            return true;
        }
        return false;
    });
}

Unit* EditUnitManager::GetUnitAt(Point mapCoords) {
    for (Unit* u : placedUnits) {
        if (u && u->pos == mapCoords) {
            return u;
        }
    }
    return nullptr;
}

void EditUnitManager::DrawUnitsOnMap(const Vec2& camPos, double scale, double currentTileSize, const Font& nameFont, const HashTable<Teams, Team>& teamDataList) const {
    const double hexWidth = currentTileSize * scale;
    for (const auto* u : placedUnits) {
        if (!u) continue;

        Shape2D r;
        Vec2 displayPos = Vec2(u->pos.x, u->pos.y); // In editor, units are static on tiles

        if (u->pos.y % 2 == 0) {
            r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * displayPos.x + camPos.x, hexWidth * 1.5 * displayPos.y + camPos.y));
        } else {
            r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * displayPos.x + hexWidth * Math::Sqrt3 / 2 + camPos.x, hexWidth * 1.5 * displayPos.y + camPos.y));
        }

        ColorF unitColor = u->color; // Use stored color, which should have been set based on team
        r.draw(unitColor);
        r.drawFrame(2, ColorF(unitColor.rgb(), unitColor.a * 0.7));

        if (scale >= 0.6 && unitDefinitions.contains(u->type)) {
            nameFont(unitDefinitions.at(u->type).name).draw(Arg::center = r.asPolygon().centroid());
        }
    }
}

UnitType EditUnitManager::GetSelectedUnitType() const {
    return selectedUnitTypeForPlacement;
}

void EditUnitManager::SetSelectedUnitType(UnitType type) {
    if (unitDefinitions.contains(type) || type < UnitType::Max) { // Allow selection up to Max for UI purposes
        selectedUnitTypeForPlacement = type;
    }
}

Teams EditUnitManager::GetSelectedTeam() const {
    return selectedTeamForPlacement;
}

void EditUnitManager::SetSelectedTeam(Teams team) {
    selectedTeamForPlacement = team;
}
