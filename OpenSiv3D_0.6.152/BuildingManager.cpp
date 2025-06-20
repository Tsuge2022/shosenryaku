#include "BuildingManager.h"
#include "TeamManager.h"
#include "UnitManager.h"
#include "Unit.h" // Included for Unit type, though it's in DataTypes.h

BuildingManager::BuildingManager() {
    // Constructor
}

BuildingManager::~BuildingManager() {
    for (City* city : cityList) { delete city; }
    cityList.clear();
    for (Factory* factory : factoryList) { delete factory; }
    factoryList.clear();
    for (Fort* fort : fortList) { delete fort; }
    fortList.clear();
}

void BuildingManager::LoadCities(const FilePath& cityCSVPath, const HashTable<Teams, Team>& teamList) {
    CSV csv(cityCSVPath);
    if (!csv) {
        Logger << U"Failed to load cityCSV.csv from: " << cityCSVPath;
        return;
    }
    cityList.clear(); // Clear existing before loading
    for (size_t i = 1; i < csv.rows(); ++i) { // Skip header
        City* c = new City();
        c->name = csv[i][0];
        c->team = static_cast<Teams>(Parse<int>(csv[i][1]));
        if (teamList.contains(c->team)) {
            c->color = teamList.at(c->team).color;
        } else {
            c->color = Palette::Gray; // Default if team not found
        }
        c->plusMoney = Parse<int>(csv[i][2]);
        c->pos = Parse<Point>(csv[i][3]);
        c->isCapture = false; // Default not captured, or load from CSV if available
        cityList.push_back(c);
    }
}

// Replicating hardcoded factories from original Game constructor for now
void BuildingManager::InitializeFactories(const HashTable<Teams, Team>& teamList, const HashTable<UnitType, UnitData>& unitDB) {
    factoryList.clear(); // Clear existing

    // Factory 1 (Red Team, Tank)
    if (teamList.contains(Teams::Red) && unitDB.contains(UnitType::Tank)) {
        Factory* f1 = new Factory();
        f1->makeUnit = UnitType::Tank;
        f1->pos = Point(2, 3); // Example position from original
        f1->gold = 300;       // Example cost
        f1->team = Teams::Red;
        f1->color = teamList.at(Teams::Red).color;
        factoryList.push_back(f1);
    }

    // Factory 2 (Blue Team, Heli)
    if (teamList.contains(Teams::Blue) && unitDB.contains(UnitType::Heli)) {
        Factory* f2 = new Factory();
        f2->makeUnit = UnitType::Heli;
        f2->pos = Point(8, 3); // Example position
        f2->gold = 300;       // Example cost
        f2->team = Teams::Blue;
        f2->color = teamList.at(Teams::Blue).color;
        factoryList.push_back(f2);
    }
    // Add more factories if needed, or adapt to load from a file/scenario data
}


void BuildingManager::LoadForts(const FilePath& fortCSVPath, const HashTable<Teams, Team>& teamList) {
    CSV csv(fortCSVPath);
    if (!csv) {
        Logger << U"Failed to load fortCSV.csv from: " << fortCSVPath;
        return;
    }
    fortList.clear(); // Clear existing
    for (size_t i = 1; i < csv.rows(); ++i) { // Skip header
        Fort* f = new Fort();
        f->team = static_cast<Teams>(Parse<int>(csv[i][0]));
         if (teamList.contains(f->team)) {
            f->color = teamList.at(f->team).color;
        } else {
            f->color = Palette::Gray;
        }
        f->pos = Parse<Point>(csv[i][1]);
        f->isCapture = false; // Default not captured
        fortList.push_back(f);
    }
}

const Array<City*>& BuildingManager::GetCityList() const { return cityList; }
const Array<Factory*>& BuildingManager::GetFactoryList() const { return factoryList; }
const Array<Fort*>& BuildingManager::GetFortList() const { return fortList; }

Factory* BuildingManager::GetFactoryAt(Point pos) const {
    for (Factory* f : factoryList) { if (f->pos == pos) return f; }
    return nullptr;
}
City* BuildingManager::GetCityAt(Point pos) const {
    for (City* c : cityList) { if (c->pos == pos) return c; }
    return nullptr;
}
Fort* BuildingManager::GetFortAt(Point pos) const {
    for (Fort* f : fortList) { if (f->pos == pos) return f; }
    return nullptr;
}


void BuildingManager::UpdateBuildings() {
    // Placeholder for passive updates (e.g., resource generation animation, repair, etc.)
}

void BuildingManager::DrawCities(const Vec2& camPos, double scale, double size, const Font& nameFont) const {
    const double hexWidth = size * scale;
    for (const auto& c : cityList) {
        Shape2D r;
        if (c->pos.y % 2 == 0) {
            r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * c->pos.x + camPos.x, hexWidth * 1.5 * c->pos.y + camPos.y));
        } else {
            r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * c->pos.x + hexWidth * Math::Sqrt3 / 2 + camPos.x, hexWidth * 1.5 * c->pos.y + camPos.y));
        }
        r.drawFrame(3, c->color);
        r.draw(Palette::White); // Inner color
        if (scale >= 0.5) nameFont(c->name).draw(Arg::center = r.asPolygon().centroid(), Palette::Black);
    }
}

void BuildingManager::DrawFactories(const Vec2& camPos, double scale, double size, const Font& factoryFont, const HashTable<UnitType, UnitData>& unitDB) const {
    const double hexWidth = size * scale;
    for (const auto& f : factoryList) {
        Shape2D r;
        if (f->pos.y % 2 == 0) {
            r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * f->pos.x + camPos.x, hexWidth * 1.5 * f->pos.y + camPos.y));
        } else {
            r = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * f->pos.x + hexWidth * Math::Sqrt3 / 2 + camPos.x, hexWidth * 1.5 * f->pos.y + camPos.y));
        }
        r.draw(Palette::White);
        r.drawFrame(2, f->color);
        if (scale >= 0.5 && unitDB.contains(f->makeUnit)) {
             factoryFont(unitDB.at(f->makeUnit).name + U"\n工場").draw(Arg::center = r.asPolygon().centroid(), Palette::Black);
        }
    }
}

void BuildingManager::DrawForts(const Vec2& camPos, double scale, double size, const Font& nameFont) const {
    const double hexWidth = size * scale;
    for (const auto& f : fortList) {
        Shape2D fortShape;
        if (f->pos.y % 2 == 0) {
            fortShape = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * f->pos.x + camPos.x, hexWidth * 1.5 * f->pos.y + camPos.y));
        } else {
            fortShape = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * f->pos.x + hexWidth * Math::Sqrt3 / 2 + camPos.x, hexWidth * 1.5 * f->pos.y + camPos.y));
        }
        fortShape.draw(Palette::White);
        fortShape.drawFrame(2, f->isCapture ? Palette::Gray : f->color); // Indicate capture status visually
        if (scale >= 0.5) nameFont(U"要塞").draw(Arg::center = fortShape.asPolygon().centroid(), Palette::Black);
    }
}

void BuildingManager::CaptureCity(City* city, Unit* capturingUnit) {
    if (city && capturingUnit) {
        city->team = capturingUnit->team;
        city->color = capturingUnit->color; // Assuming unit color is team color
        city->isCapture = true; // Mark as captured, potentially for scoring or income
    }
}

void BuildingManager::CaptureFort(Fort* fort, Unit* capturingUnit) {
    if (fort && capturingUnit) {
        if (fort->team != capturingUnit->team) { // A fort is 'captured' if taken by an opposing team
            fort->isCapture = true;
            // The fort's team itself does not change, only its 'isCapture' status.
            // Victory condition depends on all forts being 'isCapture=true' AND belonging to the same non-neutral team (implicitly).
        }
    }
}

bool BuildingManager::ProduceUnitFromFactory(Factory* factory, TeamManager* teamManager, UnitManager* unitManager) {
    if (!factory || !teamManager || !unitManager) return false;

    Team* producingTeam = teamManager->GetTeam(factory->team);
    if (!producingTeam || producingTeam->money < factory->gold) {
        return false; // Not enough money or team not found
    }

    if (unitManager->GetUnitAt(factory->pos) != nullptr) {
        return false; // Factory tile is occupied
    }

    teamManager->AddMoney(factory->team, -factory->gold);
    // Assuming UnitManager::CreateUnit handles adding to unit list and setting HP
    unitManager->CreateUnit(factory->makeUnit, factory->pos, factory->team, factory->color);
    return true;
}

void BuildingManager::AwardIncomeFromCities(TeamManager* teamManager) {
    if (!teamManager) return;
    for (const auto& city : cityList) {
        // Original logic: if (c->team == nowTeam) then add money.
        // This implies cities only generate income for the current player.
        // A more common model is all captured cities generate for their controlling team each turn.
        // Let's assume the latter: if a city is captured (or belongs to a non-neutral team), it provides income.
        if (city->team != Teams::None) { // Or use city->isCapture if that's the definitive flag
            teamManager->AddMoney(city->team, city->plusMoney);
        }
    }
}

Teams BuildingManager::CheckFortVictoryCondition() const {
    if (fortList.isEmpty()) return Teams::None;

    Teams firstCapturedTeam = Teams::None;
    bool firstFound = false;

    for (const auto& fort : fortList) {
        if (!fort->isCapture) return Teams::None; // All forts must be captured

        if (fort->team == Teams::None) return Teams::None; // Captured by neutral doesn't count

        if (!firstFound) {
            firstCapturedTeam = fort->team;
            firstFound = true;
        } else {
            if (firstCapturedTeam != fort->team) return Teams::None; // All forts must be captured by the SAME team
        }
    }
    // If loop completes and all captured forts belong to the same (non-None) team, that team wins.
    return firstFound ? firstCapturedTeam : Teams::None;
}
