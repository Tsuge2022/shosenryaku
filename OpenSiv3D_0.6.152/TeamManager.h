#pragma once
#include "DataTypes.h" // Includes Team, Teams, ColorF, etc.
#include <Siv3D.hpp>   // For HashTable, String, etc.

class TeamManager {
public:
    TeamManager(); // Constructor

    // Initialization
    void InitializeTeams(); // Sets up initial teams (e.g., Red, Blue) with starting money

    // Team Info
    const Team* GetTeam(Teams teamID) const;
    Team* GetTeam(Teams teamID); // Non-const version
    const HashTable<Teams, Team>& GetAllTeams() const;
    Teams GetCurrentTeamID() const;
    int GetCurrentTurn() const;

    // Money Management
    int GetTeamMoney(Teams teamID) const;
    void SetTeamMoney(Teams teamID, int amount);
    void AddMoney(Teams teamID, int amount); // Can be negative to subtract

    // Turn Management
    void NextTurn();
    void ResetForNewTurn(); // Actions for the start of the new current team's turn

private:
    HashTable<Teams, Team> teamDataList;
    Teams currentTeam;
    int currentTurnNumber;
    Array<Teams> turnOrder;
};
