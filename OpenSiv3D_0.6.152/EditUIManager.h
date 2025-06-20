#pragma once
#include "DataTypes.h"
#include <Siv3D.hpp>

// Forward Declarations
class EditMapManager;
class EditUnitManager;
class EditBuildingManager;

enum class EditMode { Map, Unit, Building };

class EditUIManager {
public:
    EditUIManager();
    void Initialize();

    enum class EditAction {
        None,
        ModeChangedToMap, ModeChangedToUnit, ModeChangedToBuilding,
        SaveAll, ExitToTitle, // Renamed Save to SaveAll for clarity
        ChipSelected, UnitTypeSelected, BuildingTypeSelected, TeamSelectedForPlacement, // Clarified purpose
        OpenChipSettings, OpenUnitSettings, OpenBuildingSettings,
        CloseDialog, // Generic close for any dialog
        ToggleEraseMode,
        MapResized // For when map resize buttons are used
    };

    void DrawUI(EditMode currentMode,
                EditMapManager* mapManager,
                EditUnitManager* unitManager,
                EditBuildingManager* buildingManager,
                const HashTable<Teams, Team>& teamData, // For team colors, names
                const Size& currentMapSize // For displaying map size
                );

    EditAction HandleInput(EditMode currentMode,
                           const Vec2& mousePos,
                           EditMapManager* mapManager,
                           EditUnitManager* unitManager,
                           EditBuildingManager* buildingManager,
                           // Pass current selections to avoid direct manager calls in some cases
                           int32 currentSelectedChipID,
                           UnitType currentSelectedUnitType,
                           Teams currentSelectedTeam);


    // Modal Dialog Management
    bool IsChipSettingsDialogOpen() const;
    void OpenChipSettingsDialog(Chip* chipToEdit);
    void DrawChipSettingsDialog(); // Chip pointer stored internally
    EditAction HandleChipSettingsInput();
    void CloseChipSettingsDialog();

    bool IsUnitSettingsDialogOpen() const;
    void OpenUnitSettingsDialog(UnitData* unitDataToEdit);
    void DrawUnitSettingsDialog(const HashTable<UnitType, UnitData>& allUnitTypesForAdvantageSelection); // Pass this for dropdown
    EditAction HandleUnitSettingsInput();
    void CloseUnitSettingsDialog();

    bool IsBuildingSettingsDialogOpen() const;
    void OpenBuildingSettingsDialog(City* cityToEdit);
    void DrawBuildingSettingsDialog(const HashTable<Teams, Team>& teamData); // Pass for team selection dropdown
    EditAction HandleBuildingSettingsInput();
    void CloseBuildingSettingsDialog();

    bool GetEraseMode() const;
    void SetEraseMode(bool erase);


private:
    Font uiFont;
    Font nameFont;
    Font titleFont; // Renamed from Font to titleFont for clarity

    double chipPaletteScrollY = 0;
    double unitPaletteScrollY = 0;
    double buildingPaletteScrollY = 0;

    TextEditState currentTextEditState; // For name/property editing in dialogs

    bool showChipSettingsDialog = false;
    Chip* editingChip = nullptr;

    bool showUnitSettingsDialog = false;
    UnitData* editingUnitData = nullptr;

    bool showBuildingSettingsDialog = false;
    City* editingCity = nullptr;

    bool eraseModeActive = false;

    // UI Element Rects - initialized in Initialize()
    Rect topBarRect;      // For mode change buttons
    Rect leftPaletteRect; // For chips, units, buildings lists
    Rect bottomBarRect;   // For Save, Exit, Map Size
    Rect settingsDialogRect; // Common rect for modal dialogs

    // Drawing Helpers
    void DrawMapEditModeUI(EditMapManager* mapManager);
    void DrawUnitEditModeUI(EditUnitManager* unitManager, const HashTable<Teams, Team>& teamData);
    void DrawBuildingEditModeUI(EditBuildingManager* buildingManager, const HashTable<Teams, Team>& teamData);

    void DrawModeChangeButtons(EditMode currentMode);
    void DrawBottomBarControls(const Size& currentMapSize);

    // Input Helpers
    EditAction HandleMapEditModeInput(const Vec2& mousePos, EditMapManager* mapManager, int32 currentSelectedChipID);
    EditAction HandleUnitEditModeInput(const Vec2& mousePos, EditUnitManager* unitManager, Teams currentSelectedTeam);
    EditAction HandleBuildingEditModeInput(const Vec2& mousePos, EditBuildingManager* buildingManager, Teams currentSelectedTeam);
    EditAction HandleCommonBottomBarInput(const Vec2& mousePos);
    EditAction HandleModeChangeInput(const Vec2& mousePos, EditMode currentMode);

    // Helper for palette scrolling and item clicking
    template<typename T>
    int DrawPaletteItems(const Vec2& startPos, const Array<T>& items, double scrollY, const std::function<void(const T&, const RectF&, bool)>& drawItemFunc, int selectedIndex = -1);
    // Placeholder for map resize input, as it's complex with text fields
    // EditAction HandleMapResizeInput();
    Vec2 mapSizeInputText = Vec2(20,20); // Placeholder for SimpleGUI::TextBox for map size
    TextEditState mapWidthTextState;
    TextEditState mapHeightTextState;


};
