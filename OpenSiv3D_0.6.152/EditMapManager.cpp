#include "EditMapManager.h"

EditMapManager::EditMapManager() : selectedChipID(0), nextChipIdCounter(0) {
    // Initialize with a default map and one default chip
    mapGrid.resize(Size(10, 10), 0); // Default 10x10 map, all chipID 0

    Chip defaultChip{U"Grass", HSV(120, 0.7, 0.8), 1};
    chipData.emplace(nextChipIdCounter, defaultChip);
    selectedChipID = nextChipIdCounter;
    nextChipIdCounter++;
}

void EditMapManager::LoadMapData(const FilePath& mapPath, const FilePath& chipDataPath) {
    // Load Chip Definitions
    chipData.clear();
    nextChipIdCounter = 0;
    CSV csvChips(chipDataPath);
    if (csvChips) {
        for (size_t i = 1; i < csvChips.rows(); ++i) { // Skip header
            int32 id = Parse<int32>(csvChips[i][0]);
            String name = csvChips[i][1];
            HSV color = Parse<HSV>(csvChips[i][2]);
            int cost = Parse<int32>(csvChips[i][3]);
            chipData.emplace(id, Chip{name, color, cost});
            if (id >= nextChipIdCounter) {
                nextChipIdCounter = id + 1;
            }
        }
    }
    if (chipData.isEmpty()) { // Ensure at least one chip exists
        AddNewChip(U"Default", Palette::Gray, 1); // Add a fallback default chip
    }
    selectedChipID = chipData.begin()->first; // Select the first loaded/created chip


    // Load Map Grid
    CSV csvMap(mapPath);
    if (csvMap) {
        if (csvMap.rows() >= 2 && csvMap.columns(0) >=1 && csvMap.columns(1) >=1 ) {
            const int32 mapWidth = Parse<int32>(csvMap[0][0]);
            const int32 mapHeight = Parse<int32>(csvMap[1][0]);
            mapGrid.resize(Size(mapWidth, mapHeight));

            for (int32 r = 0; r < mapHeight; ++r) {
                if (csvMap.rows() <= (r + 2)) break; // Check if row exists
                for (int32 c = 0; c < mapWidth; ++c) {
                    if (csvMap.columns(r + 2) <= c) break; // Check if col exists in row
                    mapGrid[r][c] = Parse<int32>(csvMap[r + 2][c]);
                }
            }
        } else {
            // Logger << U"Error: Map CSV has invalid format for dimensions.";
            mapGrid.assign(10,10, chipData.begin()->first); // Fallback
        }
    } else {
         // Logger << U"Error: Could not load map file: " << mapPath;
        mapGrid.assign(10,10, chipData.begin()->first); // Fallback
    }
}

void EditMapManager::SaveMapData(const FilePath& mapPath, const FilePath& chipDataPath) const {
    // Save Map Grid
    CSV csvMap;
    csvMap.writeRow(mapGrid.width());
    csvMap.writeRow(mapGrid.height());
    for (auto y : step(mapGrid.height())) {
        for (auto x : step(mapGrid.width())) {
            csvMap.write(mapGrid[y][x]);
        }
        csvMap.newLine();
    }
    csvMap.save(mapPath);

    // Save Chip Definitions
    CSV csvChips;
    csvChips.writeRow(U"ID", U"Name", U"Color(HSV)", U"Cost");
    for (const auto& pair : chipData) {
        csvChips.writeRow(pair.first, pair.second.name, Format(pair.second.color), pair.second.cost);
    }
    csvChips.save(chipDataPath);
}

Size EditMapManager::GetMapSize() const {
    return mapGrid.size();
}

void EditMapManager::ResizeMap(const Size& newSize) {
    if (newSize.x <= 0 || newSize.y <= 0) return;

    Grid<int32> newMap(newSize.y, newSize.x); // Grid constructor is (height, width)
    int32 defaultChip = 0;
    if (!chipData.isEmpty()) {
        defaultChip = chipData.begin()->first; // Use the first chip as default fill
    }

    for (int32 y = 0; y < newSize.y; ++y) {
        for (int32 x = 0; x < newSize.x; ++x) {
            if (mapGrid.inBounds(y, x)) {
                newMap[y][x] = mapGrid[y][x];
            } else {
                newMap[y][x] = defaultChip;
            }
        }
    }
    mapGrid = newMap;
}

const HashTable<int32, Chip>& EditMapManager::GetChipData() const {
    return chipData;
}

Chip* EditMapManager::GetChipDefinition(int32 chipID) {
    if (chipData.contains(chipID)) {
        return &chipData[chipID];
    }
    return nullptr;
}

void EditMapManager::AddNewChip(const String& name, const HSV& color, int cost) {
    chipData.emplace(nextChipIdCounter, Chip{name, color, cost});
    selectedChipID = nextChipIdCounter; // Select the new chip
    nextChipIdCounter++;
}

void EditMapManager::RemoveChip(int32 chipID) {
    if (chipData.size() <= 1) return; // Don't remove the last chip

    if (chipData.contains(chipID)) {
        chipData.erase(chipID);
        if (selectedChipID == chipID) {
            if (!chipData.isEmpty()) {
                selectedChipID = chipData.begin()->first; // Select another chip
            } else {
                // This case should ideally not be reached if we prevent deleting the last chip.
                // If it were possible, we'd need to create a new default chip here.
                selectedChipID = -1; // Invalid state, should re-add a default chip
            }
        }
        // Note: Does not update chip IDs in mapGrid. Users should be warned or this should be handled.
    }
}


int32 EditMapManager::GetSelectedChipID() const {
    return selectedChipID;
}

void EditMapManager::SetSelectedChipID(int32 chipID) {
    if (chipData.contains(chipID)) {
        selectedChipID = chipID;
    }
}

int EditMapManager::GetChipCount() const {
    return static_cast<int>(chipData.size());
}

int32 EditMapManager::GetNextAvailableChipID() const {
    return nextChipIdCounter;
}

void EditMapManager::PlaceChip(Point mapCoords) {
    if (mapGrid.inBounds(mapCoords)) {
        mapGrid(mapCoords) = selectedChipID;
    }
}

void EditMapManager::DrawEditableMap(const Vec2& camPos, double scale, double currentTileSize) const {
    const double displayHexSize = currentTileSize * scale;
    for (auto y : step(mapGrid.height())) {
        for (auto x : step(mapGrid.width())) {
            int32 chipNo = mapGrid[y][x];
            if (chipData.contains(chipNo)) {
                const Chip& currentChip = chipData.at(chipNo);
                Shape2D h;
                if (y % 2 == 0) {
                    h = Shape2D::Hexagon(displayHexSize, Vec2(displayHexSize * Math::Sqrt3 * x + camPos.x, displayHexSize * 1.5 * y + camPos.y));
                } else {
                    h = Shape2D::Hexagon(displayHexSize, Vec2(displayHexSize * Math::Sqrt3 * x + displayHexSize * Math::Sqrt3 / 2 + camPos.x, displayHexSize * 1.5 * y + camPos.y));
                }
                h.draw(currentChip.color);
                h.drawFrame(1, Palette::Black);
            }
        }
    }
}
