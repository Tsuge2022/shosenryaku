#include "AIManager.h"
#include "UnitManager.h"
#include "MapManager.h"
#include "BuildingManager.h"
#include "TeamManager.h"
#include "GameUIManager.h" // For logging AI decisions, if desired
#include "SoundManager.h"   // If AI actions directly trigger sounds (less common)

AIManager::AIManager() : currentState(AIState::IDLE), productionPhaseDone(false), actionsPhaseDone(false) {
    // Initialize grids to a small default size; they'll be resized when map info is available.
    InitializeCPUDistanceGrid(Size(1,1));
    InitializeEvaluationGrid(Size(1,1));
}

void AIManager::InitializeCPUDistanceGrid(Size mapSize) {
    cpuDistanceRangeGrid.resize(mapSize);
    for (auto y : step(cpuDistanceRangeGrid.height())) {
        for (auto x : step(cpuDistanceRangeGrid.width())) {
            cpuDistanceRangeGrid[y][x] = 99; // Original default for "unreachable" or "very far"
        }
    }
}

void AIManager::InitializeEvaluationGrid(Size mapSize) {
    evaluationGrid.resize(mapSize);
    for (auto y : step(evaluationGrid.height())) {
        for (auto x : step(evaluationGrid.width())) {
            evaluationGrid[y][x] = 99; // Default high cost / low desirability
        }
    }
}

void AIManager::PrepareForTurn(Teams aiTeam, MapManager* mapManager, BuildingManager* buildingManager) {
    if (!mapManager || !buildingManager || mapManager->GetMapLayoutGrid().isEmpty()) {
        currentState = AIState::TURN_ENDED; // Cannot act without map info
        return;
    }

    InitializeCPUDistanceGrid(mapManager->GetMapLayoutGrid().size());
    InitializeEvaluationGrid(mapManager->GetMapLayoutGrid().size());

    // Calculate distance to a strategic target (e.g., first enemy/neutral fort)
    // This replicates the logic from Game::Game() constructor for one of the forts.
    // A more advanced AI would pick its targets dynamically.
    Point strategicTargetPos = Point(-1,-1);
    for(const auto& fort : buildingManager->GetFortList()){
        if(fort->team != aiTeam) { // Prioritize non-allied forts
            strategicTargetPos = fort->pos;
            break;
        }
    }
    if(strategicTargetPos == Point(-1,-1) && !buildingManager->GetFortList().isEmpty()){ // fallback to any fort if all are allied (unlikely win condition then)
        strategicTargetPos = buildingManager->GetFortList()[0]->pos;
    }

    if (mapManager->GetMapLayoutGrid().inBounds(strategicTargetPos)) {
        CalculateCPUDistanceRecursive(strategicTargetPos, 0, mapManager->GetMapLayoutGrid(), mapManager->GetChipData());
    }

    productionPhaseDone = false;
    actionsPhaseDone = false;
    currentState = AIState::IDLE;
}


bool AIManager::ThinkAndAct(
    Teams aiTeam,
    MapManager* mapManager,
    UnitManager* unitManager,
    BuildingManager* buildingManager,
    TeamManager* teamManager,
    GameUIManager* gameUIManager,
    SoundManager* soundManager) {

    if (currentState == AIState::TURN_ENDED) return false; // AI has decided it's done.

    if (unitManager->GetMovingUnit() != nullptr) {
        return true; // AI is waiting for its unit to finish moving.
    }

    // Phase 1: Unit Production (once per turn)
    if (!productionPhaseDone) {
        DecideUnitProduction(aiTeam, buildingManager, unitManager, teamManager, gameUIManager);
        productionPhaseDone = true;
        currentState = AIState::PRODUCING_UNITS; // Or directly to MOVING_UNITS if production is instant
        // return true; // Give a frame for production "animation" or just to break up processing.
                       // For now, let's assume production is instant and flow into actions.
    }
    currentState = AIState::IDLE; // Reset from PRODUCING if it flowed through

    // Phase 2: Unit Actions (multiple steps, one action at a time)
    if (!actionsPhaseDone) {
        bool actionTaken = AttemptUnitAction(aiTeam, mapManager, unitManager, buildingManager, teamManager, gameUIManager, soundManager);
        if (actionTaken) {
            // If unit started moving, GetMovingUnit() will be non-null next ThinkAndAct call.
            // If an attack happened, it was instant. AI might be able to do another action.
            // For simplicity, let's assume one action (move start or attack) per ThinkAndAct call if it leads to a state change.
            return true;
        } else {
            // No more actions could be taken by any unit.
            actionsPhaseDone = true;
        }
    }

    currentState = AIState::TURN_ENDED;
    return false; // AI is done with its turn.
}

void AIManager::DecideUnitProduction(Teams aiTeam, BuildingManager* buildingManager, UnitManager* unitManager, TeamManager* teamManager, GameUIManager* gameUIManager) {
    // Adapted from Game::CPUCreateJudge
    const Array<Factory*>& factories = buildingManager->GetFactoryList();
    Array<Factory*> aiFactories;
    for (Factory* f : factories) {
        if (f->team == aiTeam) {
            aiFactories.push_back(f);
        }
    }
    if (aiFactories.isEmpty()) return;

    HashTable<UnitType, int> teamUnitCounts;
    for (int i = 0; i < static_cast<int>(UnitType::Max); ++i) {
        teamUnitCounts.emplace(static_cast<UnitType>(i), 0);
    }
    for (const auto& u : unitManager->GetUnitList()) {
        if (u->team == aiTeam) {
            teamUnitCounts[u->type]++;
        }
    }

    // Try to produce the unit type the AI has the fewest of, from an available factory.
    // This is a simplified version of the original logic.
    for (int i = 0; i < static_cast<int>(UnitType::Max); ++i) { // Iterate to find least count unit
        UnitType typeToBuild = static_cast<UnitType>(i); // Example: try to build tanks first, then heli, then human
                                                        // A better AI would have preferences.
        int minCount = 10000; // sentinel
        UnitType selectedTypeToBuild = UnitType::Max;

        for(const auto& pair : teamUnitCounts){
            if(pair.second < minCount){
                minCount = pair.second;
                selectedTypeToBuild = pair.first;
            }
        }
        if(selectedTypeToBuild == UnitType::Max) continue; // Should not happen if Max is not in teamUnitCounts keys

        typeToBuild = selectedTypeToBuild; // Build the unit type AI has the fewest of.

        for (Factory* factory : aiFactories) {
            if (factory->makeUnit == typeToBuild) {
                if (teamManager->GetTeamMoney(aiTeam) >= factory->gold) {
                    if (unitManager->GetUnitAt(factory->pos) == nullptr) { // Check if factory tile is clear
                        buildingManager->ProduceUnitFromFactory(factory, teamManager, unitManager);
                        if(gameUIManager) gameUIManager->AddConsoleMessage(U"AI produced " + unitManager->GetUnitDB().at(typeToBuild).name);
                        teamUnitCounts[typeToBuild]++; // Assume it's built, update local count for this decision cycle
                        // Original logic returned after one production.
                        return;
                    }
                }
            }
        }
    }
}


bool AIManager::AttemptUnitAction(Teams aiTeam, MapManager* mapManager, UnitManager* unitManager, BuildingManager* buildingManager, TeamManager* teamManager, GameUIManager* gameUIManager, SoundManager* soundManager) {
    // Iterate through AI units. If one can make a good move, do it and return true.
    // If one can make a good attack, do it and return true.
    // If no unit can do anything useful, return false.

    Grid<int32> threatMap = CreateThreatMap(aiTeam == Teams::Red ? Teams::Blue : Teams::Red, mapManager, unitManager);

    for (Unit* unit : unitManager->GetUnitList()) {
        if (unit->team != aiTeam) continue;

        const UnitData& unitData = unitManager->GetUnitDB().at(unit->type);

        // Try to Move
        if (!unit->isMove) {
            mapManager->CalculateMoveRange(unitData.speed, unit->pos, unitManager->GetUnitList());
            const auto& moveRangeGrid = mapManager->GetMoveRangeGrid();

            InitializeEvaluationGrid(mapManager->GetMapLayoutGrid().size());
            Point bestMoveTarget = unit->pos;
            int bestEval = EvaluatePosition(unit->pos, unit, aiTeam, mapManager, unitManager, buildingManager, threatMap); // Eval current pos

            for (auto y : step(moveRangeGrid.height())) {
                for (auto x : step(moveRangeGrid.width())) {
                    if (moveRangeGrid[y][x] >= 0) { // Reachable tile
                        Point currentTile = Point(x,y);
                        evaluationGrid[y][x] = EvaluatePosition(currentTile, unit, aiTeam, mapManager, unitManager, buildingManager, threatMap);
                        if (evaluationGrid[y][x] < bestEval) { // Lower is better
                            bestEval = evaluationGrid[y][x];
                            bestMoveTarget = currentTile;
                        }
                    }
                }
            }

            if (bestMoveTarget != unit->pos) {
                // Simplified pathing: If MapManager's moveRangeGrid stores parent pointers or path costs,
                // we could reconstruct a path. For now, assume UnitManager::StartUnitMovement
                // can handle moving towards a target within the calculated move range.
                // A* or similar would be more robust.
                // The original MakeRoute is complex and assumes it can write to drawMap.
                // Let's assume for now the AI picks the target, and UnitManager makes a simple path.
                // A proper pathfinding would be needed here.
                // For now, if bestMoveTarget is reachable, construct a simple path (e.g., if adjacent or use moveRangeGrid values to step back)
                Array<Point> pathToTarget;
                // Simplified: if adjacent, just move there. A proper pathfinder is needed for multi-step moves.
                if(abs(unit->pos.x - bestMoveTarget.x) <=1 && abs(unit->pos.y - bestMoveTarget.y) <=1 && moveRangeGrid[bestMoveTarget.y][bestMoveTarget.x] >=0) {
                    pathToTarget.push_back(bestMoveTarget);
                } else if (moveRangeGrid[bestMoveTarget.y][bestMoveTarget.x] >=0) {
                     // Fallback for non-adjacent but reachable: just use target. UnitManager needs robust pathing.
                    pathToTarget.push_back(bestMoveTarget);
                }


                if(!pathToTarget.isEmpty()){
                    unitManager->StartUnitMovement(unit, pathToTarget);
                     if(gameUIManager) gameUIManager->AddConsoleMessage(U"AI " + unitData.name + U" moving to " + Format(bestMoveTarget));
                    return true; // Action taken: unit started moving
                }
            }
            unit->isMove = true; // If no good move found, mark as "done moving" for this turn.
        }

        // Try to Attack (if moved or couldn't find a good move)
        if (unit->isMove && !unit->isAttack) {
            mapManager->CalculateAttackRange(unitData.range, unit->pos);
            const auto& attackRangeGrid = mapManager->GetAttackRangeGrid();
            Unit* bestTarget = nullptr;
            int bestTargetScore = -1; // Higher is better for attack choice

            for (Unit* potentialTarget : unitManager->GetUnitList()) {
                if (potentialTarget->team == aiTeam || !attackRangeGrid.inBounds(potentialTarget->pos) || attackRangeGrid[potentialTarget->pos.y][potentialTarget->pos.x] < 0) {
                    continue; // Not an enemy or not in range
                }

                int currentTargetScore = 100; // Base score for being a target
                if (unitData.advUnit == potentialTarget->type) {
                    currentTargetScore += 50; // Advantage bonus
                }
                if (unitManager->GetUnitDB().at(potentialTarget->type).advUnit == unit->type) {
                    currentTargetScore -= 30; // Disadvantage penalty
                }
                // Could add factors like targetHP, targetValue etc.
                if (currentTargetScore > bestTargetScore) {
                    bestTargetScore = currentTargetScore;
                    bestTarget = potentialTarget;
                }
            }

            if (bestTarget) {
                unitManager->ExecuteBattle(unit, bestTarget, soundManager, gameUIManager, teamManager);
                unit->isAttack = true; // Mark as "done attacking"
                 if(gameUIManager) gameUIManager->AddConsoleMessage(U"AI " + unitData.name + U" attacking " + unitManager->GetUnitDB().at(bestTarget->type).name);
                return true; // Action taken: attack made (even if it was instant)
            }
            unit->isAttack = true; // If no good attack found, mark as "done attacking".
        }
    }
    return false; // No unit took any action this cycle.
}


void AIManager::CalculateCPUDistanceRecursive(Point currentPos, int32 currentCost, const Grid<int32>& mapLayout, const HashTable<int32, Chip>& chipData) {
    // Adapted from Game::CPUCheck
    if (!cpuDistanceRangeGrid.inBounds(currentPos) || !mapLayout.inBounds(currentPos)) return;

    cpuDistanceRangeGrid[currentPos.y][currentPos.x] = currentCost;

    const Point evenOffsets[] = {{0, -1}, {1, 0}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};
    const Point oddOffsets[] = {{1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 0}, {0, -1}};
    const Point* offsets = (currentPos.y % 2 == 0) ? evenOffsets : oddOffsets;

    for (int i = 0; i < 6; ++i) {
        Point p = currentPos + offsets[i];
        if (mapLayout.inBounds(p)) {
            int32 chipID = mapLayout[p.y][p.x];
            if (!chipData.contains(chipID)) continue;

            int32 newCost = currentCost + chipData.at(chipID).cost;
            if (cpuDistanceRangeGrid.inBounds(p) && cpuDistanceRangeGrid[p.y][p.x] > newCost) {
                CalculateCPUDistanceRecursive(p, newCost, mapLayout, chipData);
            }
        }
    }
}

int AIManager::EvaluatePosition(Point targetPos, Unit* forUnit, Teams aiTeam, MapManager* mapManager, UnitManager* unitManager, BuildingManager* buildingManager, const Grid<int32>& threatMap) {
    // Adapted from Game::EvaCheck - Lower is better
    int eval = 0;

    // 1. Distance to strategic objective (e.g., enemy fort)
    if (cpuDistanceRangeGrid.inBounds(targetPos)) {
        eval += cpuDistanceRangeGrid[targetPos.y][targetPos.x]; // Prefer closer to objective
    } else {
        eval += 999; // Off-grid or uncalculated is bad
    }

    // 2. Tile safety (based on pre-calculated threatMap)
    if (threatMap.inBounds(targetPos)) {
        eval += threatMap[targetPos.y][targetPos.x] * 10; // Higher threat = higher cost = worse
    }

    // 3. Offensive opportunities from this tile
    const UnitData& unitData = unitManager->GetUnitDB().at(forUnit->type);
    mapManager->CalculateAttackRange(unitData.range, targetPos);
    const auto& attackRangeFromTargetPos = mapManager->GetAttackRangeGrid();
    bool canAttackAdvantageously = false;
    for(const auto& enemyUnit : unitManager->GetUnitList()){
        if(enemyUnit->team != aiTeam && attackRangeFromTargetPos.inBounds(enemyUnit->pos) && attackRangeFromTargetPos[enemyUnit->pos.y][enemyUnit->pos.x] >=0){
            if(unitData.advUnit == enemyUnit->type){
                eval -= 30; // Strongly prefer attacking advantageous targets
                canAttackAdvantageously = true;
            } else if (unitManager->GetUnitDB().at(enemyUnit->type).advUnit != forUnit->type) {
                 eval -=10; // Prefer attacking non-disadvantageous targets
            }
        }
    }
    if(canAttackAdvantageously) eval -=15; // Extra bonus if an advantageous attack is possible

    // 4. Capture buildings
    City* cityAtTarget = buildingManager->GetCityAt(targetPos);
    if(cityAtTarget && cityAtTarget->team != aiTeam) eval -= 20; // Prefer capturing cities
    Fort* fortAtTarget = buildingManager->GetFortAt(targetPos);
    if(fortAtTarget && fortAtTarget->team != aiTeam) eval -= 50; // Strongly prefer capturing forts

    // 5. Avoid blocking own factories if unit is not placeholder
    Factory* factoryAtTarget = buildingManager->GetFactoryAt(targetPos);
    if(factoryAtTarget && factoryAtTarget->team == aiTeam) eval += 15; // Slight penalty for camping on own factory

    return eval;
}

Grid<int32> AIManager::CreateThreatMap(Teams enemyTeam, MapManager* mapManager, UnitManager* unitManager) {
    Grid<int32> threatMap(mapManager->GetMapLayoutGrid().size(), 0);
    for (const auto& enemyUnit : unitManager->GetUnitList()) {
        if (enemyUnit->team == enemyTeam) {
            const UnitData& enemyUnitData = unitManager->GetUnitDB().at(enemyUnit->type);
            mapManager->CalculateAttackRange(enemyUnitData.range, enemyUnit->pos);
            const auto& enemyAttackRange = mapManager->GetAttackRangeGrid();
            for (auto y : step(enemyAttackRange.height())) {
                for (auto x : step(enemyAttackRange.width())) {
                    if (enemyAttackRange[y][x] >= 0) {
                        threatMap[y][x] += enemyUnitData.power; // Add enemy's power as threat value
                    }
                }
            }
        }
    }
    return threatMap;
}
