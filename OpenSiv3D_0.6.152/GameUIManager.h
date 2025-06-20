#pragma once
#include "DataTypes.h" // Includes Font, ColorF, Point, String, etc.
#include <Siv3D.hpp>

// Display class should already be here from a previous step
class Display {
public:
    Display() = default;
    Display(Point _pos, Point _size, Font _f);
    void Add(const String& _text); // Make const String&
    void MouseScroll(double _moveValue);
    void Update();
private:
    struct TextAnim {
        bool isStart = false;
        double timer = 0;
        double time = 0; // Ensure initialized
        double preScroll = 0; // Ensure initialized
        double targetScroll = 0; // Ensure initialized
    };
    Array<TextAnim> animArray;
    Point pos;
    Point size;
    Array<String> texts;
    double scroll = 0;
    Font font;
    void SetAnimation(double _targetSc, double _animTime);
    TextAnim ScrollAnimation(TextAnim _anim);
    void Draw();
};


// Forward declarations
class TeamManager;
class UnitManager;
class BuildingManager;

class GameUIManager {
public:
    GameUIManager();

    void Initialize(const Font& mainFont, const Font& moneyFnt, const Font& nameFnt, const Font& factoryFnt, Point consolePos, Point consoleSize);

    // Console Management
    void AddConsoleMessage(const String& message);
    void UpdateConsole();
    void HandleConsoleScroll(double wheelDelta);

    // Drawing Methods (data provided by GameController or other managers)
    void DrawTurnInfo(int turn, const String& currentTeamName, ColorF teamColor) const;
    void DrawMoneyInfo(const HashTable<Teams, Team>& teams) const;
    void DrawCityInfoUI(const Array<City*>& cities, const HashTable<Teams, Team>& teamData, const Font& smallFont) const;
    void DrawFactoryProductionUI(const Factory* targetFactory, const UnitData& unitToProduce, const Font& mainFont) const;
    void DrawGameEndUI(Teams winningTeam, const String& winningTeamName, ColorF teamColor, const Font& mainFont) const;
    void DrawDebugOverlays(const Grid<int32>& evaRangeGrid, bool isPaused, const Vec2& camPos, double scale, double size, const Font& debugFont) const;

    enum class GameAction { None, EndTurn, ProduceUnit, CancelProduction, BackToTitle }; // Added BackToTitle
    GameAction HandleGameGUI(bool canEndTurn, bool isFactorySelected, const Factory* currentFactory, int currentTeamMoney);
    GameAction HandleEndGameGUI(); // For the "Back to Title" button on game end screen


private:
    Font uiFont;
    Font moneyFont;
    Font nameFont;
    Font factoryFont;

    Display consoleDisplay;

    // Positions/Rects for UI elements
    Rect turnInfoRect;
    Vec2 moneyInfoStartPos;
    Rect cityInfoUIRect;
    Rect factoryProductionUIRect;
    Rect gameEndUIRect;
    // ... other layout variables
};
