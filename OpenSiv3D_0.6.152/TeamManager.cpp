#include "TeamManager.h"

TeamManager::TeamManager() : currentTurnNumber(1) { // Start with turn 1
    InitializeTeams(); // Initialize teams and set the first currentTeam
}

void TeamManager::InitializeTeams() {
    teamDataList.clear();
    turnOrder.clear();

    // Define teams - example from original Game constructor
    Team redTeam{U"レッド", Palette::Red, 100}; // Initial money 100
    Team blueTeam{U"ブルー", Palette::Blue, 100}; // Initial money 100
    // Team noneTeam{U"なし", Palette::Gray, 0}; // If a neutral team is ever needed

    teamDataList.emplace(Teams::Red, redTeam);
    teamDataList.emplace(Teams::Blue, blueTeam);
    // teamDataList.emplace(Teams::None, noneTeam);


    // Define turn order
    turnOrder.push_back(Teams::Red);
    turnOrder.push_back(Teams::Blue);

    if (!turnOrder.isEmpty()) {
        currentTeam = turnOrder[0];
    } else {
        currentTeam = Teams::None; // Should not happen with hardcoded teams
    }
    // The initial +100 money for the very first team is handled by their definition above.
    // ResetForNewTurn() will handle the +100 for subsequent turns.
}

const Team* TeamManager::GetTeam(Teams teamID) const {
    if (teamDataList.contains(teamID)) {
        return &teamDataList.at(teamID);
    }
    return nullptr;
}

Team* TeamManager::GetTeam(Teams teamID) {
    if (teamDataList.contains(teamID)) {
        return &teamDataList[teamID];
    }
    return nullptr;
}

const HashTable<Teams, Team>& TeamManager::GetAllTeams() const {
    return teamDataList;
}

Teams TeamManager::GetCurrentTeamID() const {
    return currentTeam;
}

int TeamManager::GetCurrentTurn() const {
    return currentTurnNumber;
}

int TeamManager::GetTeamMoney(Teams teamID) const {
    if (const Team* team = GetTeam(teamID)) {
        return team->money;
    }
    return 0; // Or handle error appropriately
}

void TeamManager::SetTeamMoney(Teams teamID, int amount) {
    if (Team* team = GetTeam(teamID)) {
        team->money = amount;
    }
}

void TeamManager::AddMoney(Teams teamID, int amount) {
    if (Team* team = GetTeam(teamID)) {
        team->money += amount;
    }
}

void TeamManager::NextTurn() {
    if (turnOrder.isEmpty()) return;

    int currentIndex = -1;
    for (int i = 0; i < turnOrder.size(); ++i) {
        if (turnOrder[i] == currentTeam) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex != -1) {
        currentIndex = (currentIndex + 1) % turnOrder.size();
        currentTeam = turnOrder[currentIndex];
        if (currentIndex == 0) { // Wrapped around to the first team in order
            currentTurnNumber++;
        }
    } else { // Should not happen if currentTeam is always valid and in turnOrder
        currentTeam = turnOrder[0]; // Default to first team
    }
}

void TeamManager::ResetForNewTurn() {
    // Add base income for the team whose turn it is now starting
    if (teamDataList.contains(currentTeam)) {
        teamDataList[currentTeam].money += 100;
    }
    // Other universal start-of-turn logic for a team could go here.
}
