#include "EditController.h"
#include "InputManager.h"
#include "EditMapManager.h"
#include "EditUnitManager.h"
#include "EditBuildingManager.h"
#include "EditUIManager.h"
#include "ProjectManager.h"
#include "SoundManager.h" // Optional for editor sounds

EditController::EditController()
    : currentTool(EditorTool::Select), currentPanel(EditorPanel::None),
      cameraPosition(Vec2::Zero()), cameraScale(1.0), chipDisplaySize(50.0)
{
    // Constructor: Initialization of managers is deferred to Initialize().
}

EditController::~EditController()
{
    // Destructor: unique_ptrs will automatically clean up manager instances.
}

void EditController::Initialize()
{
    inputManager = std::make_unique<InputManager>();
    editMapManager = std::make_unique<EditMapManager>();
    editUnitManager = std::make_unique<EditUnitManager>();
    editBuildingManager = std::make_unique<EditBuildingManager>();
    editUIManager = std::make_unique<EditUIManager>();
    projectManager = std::make_unique<ProjectManager>();
    // soundManager = std::make_unique<SoundManager>(); // If editor has sounds

    // Initialize managers
    editMapManager->Initialize(20, 15); // Default map size
    // editUnitManager->Initialize(...); // Load or set up unit DB
    editUIManager->Initialize();
    // soundManager->LoadSounds(); // If used

    // Load default data or last edited project
    // LoadEditorData(); // Or handle this through UI interaction
}

void EditController::Update()
{
    if (inputManager) inputManager->Update();

    HandleEditorInput();    // Process camera, tool changes, map interactions
    UpdateEditorLogic();    // Update based on current tool and input

    if (editUIManager) editUIManager->Update(currentTool, currentPanel /*, other state */);
}

void EditController::Draw() const
{
    RenderEditor();
}

void EditController::Shutdown()
{
    // Clean up, save unsaved changes if necessary
    // SaveEditorData(); // Or prompt user
}

void EditController::HandleEditorInput()
{
    if (!inputManager) return;

    // Camera pan (e.g., with middle mouse button)
    if (inputManager->IsMouseButtonPressed(MouseM)) {
        cameraPosition += inputManager->GetMouseDelta();
    }

    // Camera zoom (e.g., with mouse wheel)
    double wheel = inputManager->GetMouseWheel();
    if (wheel != 0) {
        double oldScale = cameraScale;
        cameraScale *= (wheel > 0 ? 1.1 : (1.0 / 1.1));
        cameraScale = Clamp(cameraScale, 0.1, 5.0); // Min/max zoom
        // Zoom towards cursor
        Point mousePos = inputManager->GetMousePosition();
        cameraPosition = mousePos - (mousePos - cameraPosition) * (cameraScale / oldScale);
    }

    // Tool and Panel selection via UI (EditUIManager will handle its own input for this)
    // currentTool = editUIManager->HandleInputForToolSelection();
    // currentPanel = editUIManager->HandleInputForPanelVisibility();

    // If mouse is not over UI, handle map interaction based on currentTool
    if (editUIManager && !editUIManager->IsMouseOverUI()) {
        Point mousePos = inputManager->GetMousePosition();
        // Convert mousePos to map coordinates based on cameraPosition and cameraScale
        // Point mapCoord = ScreenToMapCoords(mousePos, cameraPosition, cameraScale, chipDisplaySize);

        if (inputManager->IsMouseButtonClicked(MouseL)) {
            switch (currentTool) {
                case EditorTool::PlaceChip:
                    // if (editMapManager) editMapManager->PlaceChip(mapCoord, selectedChipId);
                    break;
                case EditorTool::PlaceUnit:
                    // if (editUnitManager) editUnitManager->PlaceUnit(mapCoord, selectedUnitType, selectedTeam);
                    break;
                // Handle other tools
            }
        }
    }
}

void EditController::UpdateEditorLogic()
{
    // Update logic based on current tool, selected items, etc.
    // For example, if properties of a selected item are changed in UI, apply them here.
}

void EditController::RenderEditor() const
{
    // Clear background
    Scene::SetBackground(ColorF(0.3, 0.3, 0.3));

    // Draw map, units, buildings (managed by their respective EditManagers)
    if (editMapManager) {
        // editMapManager->DrawEditableMap(cameraPosition, cameraScale, chipDisplaySize);
    }
    // if (editUnitManager) editUnitManager->DrawEditableUnits(cameraPosition, cameraScale, chipDisplaySize);
    // if (editBuildingManager) editBuildingManager->DrawEditableBuildings(cameraPosition, cameraScale, chipDisplaySize);

    // Draw UI
    if (editUIManager) editUIManager->DrawUI();
}

void EditController::SaveEditorData()
{
    // Use ProjectManager to save map, unit definitions, placed units/buildings, etc.
    // Example:
    // if (projectManager && editMapManager) {
    //    projectManager->SaveMapData(U"map_edit.csv", editMapManager->GetMapGrid(), editMapManager->GetChipData());
    // }
    // if (projectManager && editUnitManager) {
    //    projectManager->SaveUnitDefinitions(U"unit_defs_edit.csv", editUnitManager->GetUnitDatabase());
    //    projectManager->SavePlacedUnits(U"placed_units_edit.csv", editUnitManager->GetPlacedUnits());
    // }
    // ... save building data ...
}

void EditController::LoadEditorData()
{
    // Use ProjectManager to load data into managers
    // Example:
    // if (projectManager && editMapManager) {
    //    projectManager->LoadMapData(U"map_edit.csv", editMapManager->GetMapGrid(), editMapManager->GetChipDataForEdit());
    // }
    // ... load other data ...
}
