#pragma once
#include "DataTypes.h" // For common types if needed by controller's own logic

// Forward declarations for all edit-specific managers
class EditMapManager;
class EditUnitManager;
class EditBuildingManager;
class EditUIManager;
class InputManager;   // For handling raw input
class ProjectManager; // For saving and loading map/unit/building data
class SoundManager;   // If editor has sounds

class EditController
{
public:
    EditController();
    ~EditController();

    void Initialize(); // Initialize all editor systems and load/create default data
    void Update();     // Main editor loop update function
    void Draw() const; // Main editor loop draw function
    void Shutdown();   // Clean up resources

private:
    // Pointers or smart pointers to all the edit-specific managers
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<EditMapManager> editMapManager;
    std::unique_ptr<EditUnitManager> editUnitManager;
    std::unique_ptr<EditBuildingManager> editBuildingManager;
    std::unique_ptr<EditUIManager> editUIManager;
    std::unique_ptr<ProjectManager> projectManager;
    std::unique_ptr<SoundManager> soundManager; // Optional

    // Editor state variables
    EditorTool currentTool = EditorTool::Select; // From EditUIManager.h (or define locally)
    EditorPanel currentPanel = EditorPanel::None; // From EditUIManager.h

    // Camera/View state for the map editor
    Vec2 cameraPosition{0, 0};
    double cameraScale = 1.0;
    double chipDisplaySize = 50.0;


    // Selected item for properties panel (union or variant could be useful here)
    // int selectedChipId = -1;
    // UnitType selectedUnitType = UnitType::Max; // or some invalid default
    // ...

    // Private helper methods for the editor loop phases
    void HandleEditorInput(); // Process input for camera, tool selection, map interaction
    void UpdateEditorLogic(); // Update based on current tool and input
    void RenderEditor() const;    // Draw map, units, buildings, UI

    void SaveEditorData();
    void LoadEditorData();
};
