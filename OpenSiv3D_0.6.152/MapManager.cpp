#include "MapManager.h"
// Note: DataTypes.h includes Siv3D.hpp, so direct Siv3D.hpp include might be redundant
// but kept for explicitness based on instructions.

MapManager::MapManager() {
    // Constructor is intentionally light, actual loading happens in LoadMap
}

bool MapManager::LoadMap(const FilePath& mapCSVPath, const FilePath& chipCSVPath) {
    // Load ChipData
    CSV csvChip(chipCSVPath);
    if (not csvChip) {
        // Logger << U"Failed to load ChipData.csv"; // Example logging
        return false;
    }
    chipData.clear();
    for (size_t i = 1; i < csvChip.rows(); ++i) { // Skip header row
        Chip c{csvChip[i][1], Parse<HSV>(csvChip[i][2]), Parse<int32>(csvChip[i][3])};
        chipData.emplace(Parse<int32>(csvChip[i][0]), c);
    }

    // Load MapLayout
    CSV csvMap(mapCSVPath);
    if (not csvMap) {
        // Logger << U"Failed to load mapCSV.csv"; // Example logging
        return false;
    }
    // Assuming format from Main.cpp: map width in csvMap[0][0], height in csvMap[1][0]
    // Data starts from row 2.
    const int32 mapWidth = Parse<int32>(csvMap[0][0]);
    const int32 mapHeight = Parse<int32>(csvMap[1][0]);
    mapLayoutGrid.resize(mapHeight, mapWidth);

    for (int32 row = 0; row < mapHeight; ++row) {
        for (int32 col = 0; col < mapWidth; ++col) {
            // CSV data for map starts at row 2 in the original file
            if (csvMap.rows() > (row + 2) && csvMap.columns(row + 2) > col) {
                 mapLayoutGrid[row][col] = Parse<int32>(csvMap[row + 2][col]);
            } else {
                // Logger << U"Warning: Map data missing for " << Point(col, row);
                mapLayoutGrid[row][col] = 0; // Default to chip 0 or an error chip ID
            }
        }
    }

    InitializeMoveRangeGrid(mapLayoutGrid.size());
    InitializeAttackRangeGrid(mapLayoutGrid.size());
    return true;
}

void MapManager::DrawMap(const Vec2& camPos, double scale, double size) const {
    const double width = size * scale; // Renamed from 'width' in Game::DrawMap to avoid conflict
    for (auto y : step(mapLayoutGrid.height())) {
        for (auto x : step(mapLayoutGrid.width())) {
            int32 chipNo = mapLayoutGrid[y][x];
            if (chipData.contains(chipNo)) {
                Shape2D h;
                if (y % 2 == 0) {
                    h = Shape2D::Hexagon(width, Vec2(width * Math::Sqrt3 * x + camPos.x, width * 1.5 * y + camPos.y));
                } else {
                    h = Shape2D::Hexagon(width, Vec2(width * Math::Sqrt3 * x + width * Math::Sqrt3 / 2 + camPos.x, width * 1.5 * y + camPos.y));
                }
                h.draw(chipData.at(chipNo).color);
                h.drawFrame(1, Palette::Black);
            }
        }
    }
}

void MapManager::InitializeMoveRangeGrid(Size mapSize) {
    moveRangeGrid.resize(mapSize);
    for (auto y : step(moveRangeGrid.height())) {
        for (auto x : step(moveRangeGrid.width())) {
            moveRangeGrid[y][x] = -1;
        }
    }
}

void MapManager::InitializeAttackRangeGrid(Size mapSize) {
    attackRangeGrid.resize(mapSize);
    for (auto y : step(attackRangeGrid.height())) {
        for (auto x : step(attackRangeGrid.width())) {
            attackRangeGrid[y][x] = -1;
        }
    }
}

void MapManager::CalculateMoveRange(int32 speed, Point unitPos, const Array<Unit*>& unitList) {
    if (!mapLayoutGrid.inBounds(unitPos)) return;
    InitializeMoveRangeGrid(mapLayoutGrid.size()); // Reset before calculation
    MoveCheckRecursive(speed, unitPos, unitList);
}

void MapManager::MoveCheckRecursive(int32 n, Point pos, const Array<Unit*>& units) {
    moveRangeGrid[pos.y][pos.x] = n;

    const Point evenOffsets[] = {{0, -1}, {1, 0}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};
    const Point oddOffsets[] = {{1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 0}, {0, -1}};
    const Point* offsets = (pos.y % 2 == 0) ? evenOffsets : oddOffsets;

    for (int i = 0; i < 6; ++i) {
        Point p = pos + offsets[i];

        if (mapLayoutGrid.inBounds(p)) {
            int32 chipID = mapLayoutGrid[p.y][p.x];
            if (!chipData.contains(chipID)) continue;

            int32 cost = chipData.at(chipID).cost;
            int32 s = n - cost;

            bool isUnitOnTile = false;
            for (const auto& u : units) {
                if (u && u->pos == p) {
                    isUnitOnTile = true;
                    break;
                }
            }

            if (s > -1 && moveRangeGrid[p.y][p.x] < s && !isUnitOnTile) {
                MoveCheckRecursive(s, p, units);
            }
        }
    }
}

void MapManager::CalculateAttackRange(int32 range, Point unitPos) {
    if (!mapLayoutGrid.inBounds(unitPos)) return;
    InitializeAttackRangeGrid(mapLayoutGrid.size()); // Reset before calculation
    AttackCheckRecursive(range, unitPos);
}

void MapManager::AttackCheckRecursive(int32 n, Point pos) {
    attackRangeGrid[pos.y][pos.x] = n;
    if (n <= 0) { // Range decreases, stop if non-positive (0 means can attack current tile, but not further)
        return;
    }

    const Point evenOffsets[] = {{0, -1}, {1, 0}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};
    const Point oddOffsets[] = {{1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 0}, {0, -1}};
    const Point* offsets = (pos.y % 2 == 0) ? evenOffsets : oddOffsets;

    for (int i = 0; i < 6; ++i) {
        Point p = pos + offsets[i];
        if (mapLayoutGrid.inBounds(p)) {
            // In original code, attack range check was n, and recursive call was n-1.
            // attackRangeGrid stores remaining range. If current tile is 'n', neighbors are 'n-1'.
            if (attackRangeGrid[p.y][p.x] < (n - 1)) { // Check if a better path to this tile was already found
                 AttackCheckRecursive(n - 1, p);
            }
        }
    }
}

int32 MapManager::GetChipCost(int32 chipID) const {
    if (chipData.contains(chipID)) {
        return chipData.at(chipID).cost;
    }
    return 999; // Or some other indicator of impassable/invalid
}

const Grid<int32>& MapManager::GetMoveRangeGrid() const {
    return moveRangeGrid;
}

const Grid<int32>& MapManager::GetAttackRangeGrid() const {
    return attackRangeGrid;
}

const Grid<int32>& MapManager::GetMapLayoutGrid() const {
    return mapLayoutGrid;
}

const HashTable<int32, Chip>& MapManager::GetChipData() const {
    return chipData;
}
