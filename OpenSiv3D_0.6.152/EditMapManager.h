#pragma once
#include "DataTypes.h" // Includes Chip, Grid, Point, Vec2, etc.
#include <Siv3D.hpp>

class EditMapManager {
public:
    EditMapManager();

    // Initialization and Data Loading/Saving
    void LoadMapData(const FilePath& mapPath, const FilePath& chipDataPath);
    void SaveMapData(const FilePath& mapPath, const FilePath& chipDataPath) const;

    // Map Properties
    Size GetMapSize() const;
    void ResizeMap(const Size& newSize);

    // Chip Management
    const HashTable<int32, Chip>& GetChipData() const;
    Chip* GetChipDefinition(int32 chipID);
    void AddNewChip(const String& name = U"New Chip", const HSV& color = Palette::White, int cost = 1); // Added default params
    void RemoveChip(int32 chipID); // Changed from RemoveLastChip to by ID
    int32 GetSelectedChipID() const;
    void SetSelectedChipID(int32 chipID);
    int GetChipCount() const;
    int32 GetNextAvailableChipID() const;


    // Editing Actions
    void PlaceChip(Point mapCoords);
    // void EraseChip(Point mapCoords);

    // Drawing
    void DrawEditableMap(const Vec2& camPos, double scale, double currentTileSize) const;

private:
    Grid<int32> mapGrid;
    HashTable<int32, Chip> chipData;
    int32 selectedChipID;
    int32 nextChipIdCounter; // To generate unique IDs for new chips
};
