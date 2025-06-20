#include "UnitManager.h"
#include "TeamManager.h"    // For team colors, names in messages
#include "SoundManager.h"   // For battle/movement sounds
#include "GameUIManager.h"  // For console messages
#include "BuildingManager.h"// For capturing buildings

UnitManager::UnitManager() {
    // Constructor
}

UnitManager::~UnitManager() {
    for (Unit* unit : unitList) {
        delete unit;
    }
    unitList.clear();
}

bool UnitManager::LoadUnitStats(const FilePath& unitDataCSVPath) {
    CSV csv(unitDataCSVPath);
    if (!csv) {
        Logger << U"Failed to load UnitData.csv from: " << unitDataCSVPath;
        return false;
    }
    unitDB.clear();
    for (size_t i = 1; i < csv.rows(); ++i) { // Skip header row
        UnitData u;
        // Assuming CSV format: Name, Range, Speed, AdvUnitID, (Implicitly HP, Power from somewhere or default)
        // The UnitData struct has: hp, name, power, speed, range, advUnit
        // Original Main.cpp Game constructor:
        // u.name = unitDataCSV[i][0];
        // u.range = Parse<int>(unitDataCSV[i][1]);
        // u.speed = Parse<int>(unitDataCSV[i][2]);
        // u.advUnit = (UnitType)Parse<int>(unitDataCSV[i][3]);
        // HP and Power are missing from this CSV in original, let's add defaults or assume they are set elsewhere.
        // For now, let's assume they might be in the CSV or use placeholders.
        // Let's use the structure from Main.cpp UnitData CSV loading:
        u.name = csv[i][0];
        u.range = Parse<int>(csv[i][1]);
        u.speed = Parse<int>(csv[i][2]);
        u.advUnit = static_cast<UnitType>(Parse<int>(csv[i][3]));
        // Placeholder HP and Power, as they were not in the UnitData.csv in Main.cpp's Game constructor
        u.hp = 100; // Example placeholder
        u.power = 10; // Example placeholder

        // The enum UnitType is Tank, Heli, Human, Max.
        // The CSV parsing implies (i-1) maps to the enum.
        unitDB.emplace(static_cast<UnitType>(i - 1), u);
    }
    return true;
}

void UnitManager::LoadInitialUnits(const FilePath& unitSetDataCSVPath, const HashTable<Teams, Team>& teamList) {
    CSV csv(unitSetDataCSVPath);
    if (!csv) {
        Logger << U"Failed to load UnitSetData.csv from: " << unitSetDataCSVPath;
        return;
    }
    for (Unit* unit : unitList) { delete unit; } // Clear existing units
    unitList.clear();

    for (size_t y = 1; y < csv.rows(); y++) { // Skip header
        Teams t = static_cast<Teams>(Parse<int>(csv[y][0]));
        UnitType type = static_cast<UnitType>(Parse<int>(csv[y][1]));
        Point pos = Parse<Point>(csv[y][2]);

        ColorF color = Palette::Gray; // Default color if team not found
        if (teamList.contains(t)) {
            color = teamList.at(t).color;
        }

        CreateUnit(type, pos, t, color);
    }
}

Unit* UnitManager::CreateUnit(UnitType type, Point pos, Teams team, ColorF color) {
    if (!unitDB.contains(type)) {
        Logger << U"Error: UnitType not found in unitDB when trying to create unit.";
        return nullptr;
    }
    Unit* newUnit = new Unit();
    newUnit->type = type;
    newUnit->pos = pos;
    newUnit->team = team;
    newUnit->color = color;
    newUnit->hp = unitDB[type].hp; // Set HP from DB
    newUnit->isMove = false;
    newUnit->isAttack = false;
    newUnit->moveTimer = 0.0;
    newUnit->prePos = pos;
    newUnit->drawPos = Vec2(pos.x, pos.y); // Initialize drawPos correctly

    unitList.push_back(newUnit);
    return newUnit;
}

void UnitManager::DestroyUnit(Unit* unit) {
    if (unit) {
        unitList.remove(unit);
        delete unit;
    }
}

const Array<Unit*>& UnitManager::GetUnitList() const {
    return unitList;
}

const HashTable<UnitType, UnitData>& UnitManager::GetUnitDB() const {
    return unitDB;
}

Unit* UnitManager::GetUnitAt(Point pos) const {
    for (Unit* u : unitList) {
        if (u->pos == pos) {
            return u;
        }
    }
    return nullptr;
}

void UnitManager::DrawUnits(const Vec2& camPos, double scale, double size, const Font& nameFont, const Unit* targetUnit) const {
    const double hexWidth = size * scale; // 'width' in original Game::DrawUnit

    for (const auto& u : unitList) {
        Shape2D r;
        Vec2 baseDrawPos;

        if (u->isMove && activeMovement.unit == u) { // If this unit is the one actively moving with animation
             // Use interpolated drawPos for smooth animation
            if (u->prePos.y % 2 == 0) { // For hex grid coordinate system
                 baseDrawPos = Vec2(hexWidth * Math::Sqrt3 * u->drawPos.x , hexWidth * 1.5 * u->drawPos.y);
            } else {
                 baseDrawPos = Vec2(hexWidth * Math::Sqrt3 * u->drawPos.x + hexWidth * Math::Sqrt3 / 2 , hexWidth * 1.5 * u->drawPos.y );
            }
        } else {
            // Static unit or unit not currently being animated by UpdateUnitMovement, use its current pos
             if (u->pos.y % 2 == 0) {
                baseDrawPos = Vec2(hexWidth * Math::Sqrt3 * u->pos.x, hexWidth * 1.5 * u->pos.y);
            } else {
                baseDrawPos = Vec2(hexWidth * Math::Sqrt3 * u->pos.x + hexWidth * Math::Sqrt3 / 2, hexWidth * 1.5 * u->pos.y);
            }
        }
        r = Shape2D::Hexagon(hexWidth, baseDrawPos + camPos);

        r.draw(u->color);
        if (u->isMove) { // Unit has finished its move action for the turn
            r.drawFrame(3, Palette::White);
        } else if (targetUnit != nullptr && u == targetUnit) { // Unit is currently selected by player
            r.drawFrame(2, Palette::Black);
        } else { // Default frame
            r.drawFrame(2, ColorF(u->color.rgb(), u->color.a * 0.7)); // Slightly darker/desaturated frame
        }

        if (scale >= 0.6 && unitDB.contains(u->type)) { // Draw name if zoomed in enough
            nameFont(unitDB.at(u->type).name).draw(Arg::center = r.asPolygon().centroid());
        }
    }
}

void UnitManager::StartUnitMovement(Unit* unit, const Array<Point>& route) {
    if (!unit || route.isEmpty()) return;

    activeMovement.unit = unit;
    activeMovement.route = route;
    activeMovement.currentLegIndex = 0;
    activeMovement.legTimer = 0.0;

    unit->isMove = false; // Mark as "currently moving", not "has finished moving"
    unit->isAttack = false; // Cannot attack while moving.
    unit->prePos = unit->pos; // Current position is the start of the first leg's "previous"
    if (!route.isEmpty()) {
      activeMovement.nextTilePosition = route[0]; // The first actual tile to move to.
    } else {
      activeMovement.unit = nullptr; // No route, no movement.
      return;
    }
    // unit->drawPos is already Vec2(unit->pos) from creation or previous move end.
}

bool UnitManager::UpdateUnitMovement(double timeDelta, double moveTimePerTile, const Vec2& camPosIn, Vec2& camPosOut, double currentMapHexWidth, SoundManager* soundManager, BuildingManager* buildingManager, GameUIManager* gameUIManager, TeamManager* teamManager) {
    if (!activeMovement.unit) return false; // No unit is currently set to move

    Unit* movingUnit = activeMovement.unit;
    movingUnit->moveTimer += timeDelta; // Use general moveTimer on unit, or activeMovement.legTimer

    if (movingUnit->moveTimer >= moveTimePerTile) { // Finished one leg of the journey
        movingUnit->pos = activeMovement.nextTilePosition; // Snap to the tile
        movingUnit->prePos = activeMovement.nextTilePosition; // For the next leg, this is the "previous"
        movingUnit->drawPos = Vec2(movingUnit->pos.x, movingUnit->pos.y); // Snap drawPos
        movingUnit->moveTimer = 0; // Reset timer for the leg
        activeMovement.currentLegIndex++;

        if (activeMovement.currentLegIndex >= activeMovement.route.size()) { // Entire route finished
            movingUnit->isMove = true; // Mark as "has finished moving action for the turn"

            // Building Capture Logic (from original Game::MoveUnit)
            if(buildingManager && teamManager) {
                City* city = buildingManager->GetCityAt(movingUnit->pos);
                if (city && city->team != movingUnit->team) { // Capture city
                    city->team = movingUnit->team;
                    city->color = movingUnit->color; // Assuming unit color is team color
                    city->isCapture = true; // Assuming City has this flag
                    if(gameUIManager && teamManager->GetTeam(movingUnit->team)) {
                        gameUIManager->DrawConsole(); // This is a placeholder for AddMessage or similar
                        // gameUIManager->consoleDisplay.Add(teamManager->GetTeam(movingUnit->team)->name + U"が都市" + city->name + U"を占領");
                    }
                }
                Fort* fort = buildingManager->GetFortAt(movingUnit->pos);
                if (fort && fort->team != movingUnit->team) { // Capture fort
                    fort->team = movingUnit->team;
                    fort->color = movingUnit->color;
                    fort->isCapture = true;
                     if(gameUIManager && teamManager->GetTeam(movingUnit->team)) {
                        // gameUIManager->consoleDisplay.Add(teamManager->GetTeam(movingUnit->team)->name + U"が要塞を占領");
                    }
                }
            }

            activeMovement.unit = nullptr; // Clear active movement
            return false; // Movement finished
        } else {
            activeMovement.nextTilePosition = activeMovement.route[activeMovement.currentLegIndex];
        }
    } else { // Still moving along the current leg
        double percentOfLeg = movingUnit->moveTimer / moveTimePerTile;
        Vec2 startDrawPos = Vec2(movingUnit->prePos.x, movingUnit->prePos.y);
        Vec2 endDrawPos = Vec2(activeMovement.nextTilePosition.x, activeMovement.nextTilePosition.y);

        // Simple linear interpolation for drawPos
        movingUnit->drawPos = startDrawPos.lerp(endDrawPos, percentOfLeg);

        // Camera follow logic (simplified from original)
        // This needs careful conversion from hex grid coords to screen coords if hexWidth means display size.
        // The original camPos logic was complex. For now, let's assume camPosOut is updated by GameController based on movingUnit->drawPos.
        // Example:
        // if (movingUnit->prePos.y % 2 == 0) {
        //    camPosOut = -Vec2(currentMapHexWidth * Math::Sqrt3 * movingUnit->drawPos.x - Scene::Width() / 2, currentMapHexWidth * 1.5 * movingUnit->drawPos.y - Scene::Height() / 2);
        // } else {
        //    camPosOut = -Vec2(currentMapHexWidth * Math::Sqrt3 * movingUnit->drawPos.x + currentMapHexWidth * Math::Sqrt3 / 2 - Scene::Width() / 2, currentMapHexWidth * 1.5 * movingUnit->drawPos.y - Scene::Height() / 2);
        // }
    }
    return true; // Still moving
}


void UnitManager::ExecuteBattle(Unit* attacker, Unit* defender, SoundManager* soundManager, GameUIManager* gameUIManager, TeamManager* teamManager) {
    if (!attacker || !defender || !unitDB.contains(attacker->type) || !unitDB.contains(defender->type) || !teamManager || !gameUIManager) {
        return;
    }

    const UnitData& attackerData = unitDB.at(attacker->type);
    const UnitData& defenderData = unitDB.at(defender->type);
    const Team& attackerTeam = *teamManager->GetTeam(attacker->team); // Assume GetTeam returns valid pointer or ref
    const Team& defenderTeam = *teamManager->GetTeam(defender->team);


    // gameUIManager->consoleDisplay.Add(attackerTeam.name + U"軍の" + attackerData.name + U"の攻撃！"); // Placeholder

    bool attackerWins = false;
    if (attackerData.advUnit == defender->type) { // Attacker has advantage
        attackerWins = true;
    } else if (defenderData.advUnit == attacker->type) { // Defender has advantage
        attackerWins = false;
    } else { // Neutral engagement (mutual destruction in original)
        // gameUIManager->consoleDisplay.Add(U"相打ち！"); // Placeholder
        if (soundManager) soundManager->PlaySoundEffect(SoundEffectID::ExplosionLarge); // Example sound
        DestroyUnit(attacker);
        DestroyUnit(defender);
        return;
    }

    if (attackerWins) {
        // gameUIManager->consoleDisplay.Add(attackerTeam.name + U"軍の" + attackerData.name + U"の勝ち！"); // Placeholder
        DestroyUnit(defender);
        if (soundManager) {
            switch(attacker->type) { // Sounds from original
                case UnitType::Tank: soundManager->PlaySoundEffect(SoundEffectID::UnitAttackTank); break; // Was boomSound
                case UnitType::Heli: soundManager->PlaySoundEffect(SoundEffectID::UnitAttackHeli); break; // Was gunSound
                case UnitType::Human: soundManager->PlaySoundEffect(SoundEffectID::UnitAttackHuman); break; // Was cannonSound
                default: break;
            }
        }
    } else { // Defender wins
        // gameUIManager->consoleDisplay.Add(defenderTeam.name + U"軍の" + defenderData.name + U"の勝ち！"); // Placeholder
        DestroyUnit(attacker);
         if (soundManager) {
            switch(defender->type) {
                case UnitType::Tank: soundManager->PlaySoundEffect(SoundEffectID::UnitAttackTank); break;
                case UnitType::Heli: soundManager->PlaySoundEffect(SoundEffectID::UnitAttackHeli); break;
                case UnitType::Human: soundManager->PlaySoundEffect(SoundEffectID::UnitAttackHuman); break;
                default: break;
            }
        }
    }
}

void UnitManager::ResetUnitActionsForTeam(Teams team) {
    for (Unit* u : unitList) {
        if (u->team == team) {
            u->isMove = false;
            u->isAttack = false;
        }
    }
}

Unit* UnitManager::GetMovingUnit() const {
    return activeMovement.unit;
}
