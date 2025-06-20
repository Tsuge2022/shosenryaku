#pragma once
#include "DataTypes.h" // Includes <Siv3D.hpp> and common structs
#include <Siv3D.hpp> // Explicitly include for Siv3D types if not fully covered by DataTypes.h

// Forward declaration for Unit is in DataTypes.h, which is included.

class MapManager {
public:
    MapManager();
    bool LoadMap(const FilePath& mapCSVPath, const FilePath& chipCSVPath);
    void DrawMap(const Vec2& camPos, double scale, double size) const;

    void CalculateMoveRange(int32 speed, Point unitPos, const Array<Unit*>& unitList);
    void CalculateAttackRange(int32 range, Point unitPos);

    int32 GetChipCost(int32 chipID) const; // Changed from Point to chipID for direct lookup
    const Grid<int32>& GetMoveRangeGrid() const;
    const Grid<int32>& GetAttackRangeGrid() const;
    const Grid<int32>& GetMapLayoutGrid() const;
    const HashTable<int32, Chip>& GetChipData() const;

    void InitializeMoveRangeGrid(Size mapSize);
    void InitializeAttackRangeGrid(Size mapSize);

private:
    Grid<int32> mapLayoutGrid;
    Grid<int32> moveRangeGrid;
    Grid<int32> attackRangeGrid;
    HashTable<int32, Chip> chipData;

    // Recursive helper functions, names kept similar to original for easier mapping
    void MoveCheckRecursive(int32 n, Point pos, const Array<Unit*>& units);
    void AttackCheckRecursive(int32 n, Point pos);
};
