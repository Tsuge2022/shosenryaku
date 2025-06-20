#include "GameUIManager.h"
#include "TeamManager.h"      // For Team data
#include "UnitManager.h"      // For UnitData
#include "BuildingManager.h"  // For Factory, City data

// --- Display Class Implementation ---
Display::Display(Point _pos, Point _size, Font _f)
    : pos(_pos), size(_size), font(_f), scroll(0.0) {
        // Ensure TextAnim members are default initialized if not specified in struct
    }

void Display::Add(const String& _text) {
    texts.emplace_back(_text);
    double height = (texts.size() * font.height());
    height -= size.y;
    if (height < 0) {
        scroll = 0;
    } else {
        SetAnimation(height, 0.5);
    }
}

void Display::MouseScroll(double _moveValue) {
    if (animArray.empty()) {
        double height = font.height() * texts.size();
        height -= size.y;
        if (height < 0) {
            height = 0;
        }
        scroll += _moveValue;
        scroll = Clamp(scroll, 0.0, height);
    }
}

void Display::Update() {
    if (!animArray.empty()) {
        TextAnim t = ScrollAnimation(animArray[0]);
        bool endAnim = t.time <= t.timer;
        if (endAnim) {
            animArray.erase(animArray.begin());
        } else {
            animArray[0] = t;
        }
    }
    Draw();
}

void Display::SetAnimation(double _targetSc, double _animTime) {
    TextAnim anim;
    anim.targetScroll = _targetSc;
    anim.time = _animTime;
    anim.timer = 0.0;
    anim.preScroll = scroll;
    anim.isStart = false;
    animArray.emplace_back(anim);
}

Display::TextAnim Display::ScrollAnimation(TextAnim _anim) {
    if (!_anim.isStart) {
        _anim.preScroll = scroll;
        _anim.isStart = true;
    }
    _anim.timer += Scene::DeltaTime();
    double percent = _anim.timer / _anim.time;
    percent = Clamp(percent, 0.0, 1.0);
    double plusValue = _anim.targetScroll - _anim.preScroll;
    scroll = _anim.preScroll + plusValue * EaseInOutSine(percent);
    return _anim;
}

void Display::Draw() {
    const ScopedViewport2D viewport{pos.x, pos.y, size.x, size.y};
    const Transformer2D transformer{Mat3x2::Identity(), Mat3x2::Translate(pos)};
    Rect back{0, 0, size.x, size.y};
    back.draw(ColorF(0, 0, 0, 0.3));
    for (int i = 0; i < texts.size(); ++i) {
        font(texts[i]).draw(10, font.height() * i - scroll);
    }
}

// --- GameUIManager Class Implementation ---
GameUIManager::GameUIManager()
    : uiFont(40, Typeface::Bold), // Values from original Game class
      moneyFont(20, Typeface::Black),
      nameFont(25, Typeface::Black),
      factoryFont(22, Typeface::Bold) // factoryFont was 17 in Edit, 22 in Game
{
    Initialize(); // Call Initialize to set up console and UI rects
}

void GameUIManager::Initialize() {
    // Initialize consoleDisplay with values from original Game class
    consoleDisplay = Display(Point(230, 500), Point(570, 80), factoryFont);

    turnInfoRect = Rect(530, 10, 250, 150);
    moneyInfoStartPos = Vec2(500, 350);
    // Define other specific rects for UI elements if needed
    // These were example values, can be refined or made dynamic
    cityInfoUIRect = Rect(500, 200, 280, 250);
    factoryProductionUIRect = Rect(Arg::center = Scene::Center(), 500, 200);
    gameEndUIRect = Rect(Arg::center = Scene::Center() - Vec2(0, 50), 400, 150);
}


void GameUIManager::AddConsoleMessage(const String& message) {
    consoleDisplay.Add(message);
}

void GameUIManager::UpdateConsole() {
    consoleDisplay.Update();
}

void GameUIManager::HandleConsoleScroll(double wheelDelta) {
    // Assuming GameController checks if console is the target of scroll.
    consoleDisplay.MouseScroll(wheelDelta * 15);
}

void GameUIManager::DrawTurnInfo(int turn, const String& currentTeamName, ColorF teamColor) const {
    Rect{500, 0, 300, 600}.draw(ColorF(0.1, 0.1, 0.1, 0.7)); // Right sidebar background
    turnInfoRect.rounded(10).draw(ColorF(0.2,0.2,0.2,0.8)).drawFrame(1, Palette::White);
    uiFont(currentTeamName + U"軍\n第 " + Format(turn) + U" ターン").draw(Arg::center(turnInfoRect.center()), teamColor);
}

void GameUIManager::DrawMoneyInfo(const HashTable<Teams, Team>& teams) const {
    int i = 0;
    Vec2 drawPos = moneyInfoStartPos;
    for (const auto& teamId : {Teams::Red, Teams::Blue}) { // Assuming a fixed order for display
        if (teams.contains(teamId)) {
            const Team& team = teams.at(teamId);
            moneyFont(team.name + U"資金: " + Format(team.money)).draw(drawPos + Vec2(0, i * 30), team.color);
            i++;
        }
    }
}

void GameUIManager::DrawCityInfoUI(const Array<City*>& cities, const HashTable<Teams, Team>& teamData, const Font& smallFont) const {
    Vec2 currentPos = cityInfoUIRect.pos + Vec2(10,10) ;
    smallFont(U"都市一覧:").draw(currentPos, Palette::White);
    currentPos.y += smallFont.height() + 5;

    for (const auto& city : cities) {
        if (!city) continue;
        if (currentPos.y > cityInfoUIRect.y + cityInfoUIRect.h - smallFont.height() - 5) break; // Stay within bounds

        String ownerName = U"中立";
        ColorF ownerColor = Palette::White;
        if (city->team != Teams::None && teamData.contains(city->team)) {
            ownerName = teamData.at(city->team).name;
            ownerColor = teamData.at(city->team).color;
        }
        smallFont(city->name + U" (" + ownerName + U")").draw(currentPos, ownerColor);
        currentPos.y += smallFont.height() + 2;
    }
}

void GameUIManager::DrawFactoryProductionUI(const Factory* targetFactory, const UnitData& unitToProduce, const Font& mainFont) const {
    if (!targetFactory) return;
    factoryProductionUIRect.draw(Arg::top = ColorF(0.1, 0.1, 0.1, 0.9)); // Background
    mainFont(unitToProduce.name + U"を生産しますか？\nコスト: " + Format(targetFactory->gold)).draw(Arg::center(factoryProductionUIRect.center() - Vec2(0,30)), Palette::White);
    // Buttons are handled by HandleGameGUI
}

void GameUIManager::DrawGameEndUI(Teams winningTeam, const String& winningTeamName, ColorF teamColor, const Font& mainFont) const {
    gameEndUIRect.draw(ColorF(0.1, 0.1, 0.1, 0.9));
    mainFont(winningTeamName + U" の勝利！").draw(Arg::center(gameEndUIRect.center() - Vec2(0,20)), teamColor);
    // Button handled by HandleEndGameGUI
}

void GameUIManager::DrawDebugOverlays(const Grid<int32>& evaRangeGrid, bool isPaused, const Vec2& camPos, double scale, double size, const Font& debugFont) const {
    if (!isPaused || evaRangeGrid.isEmpty()) return;

    const double hexWidth = size * scale;
    for (auto y : step(evaRangeGrid.height())) {
        for (auto x : step(evaRangeGrid.width())) {
            int val = evaRangeGrid[y][x];
            if (val != 99 && val != 0) { // Original condition from Game::DrawDebug
                Shape2D h;
                if (y % 2 == 0) {
                    h = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * x + camPos.x, hexWidth * 1.5 * y + camPos.y));
                } else {
                    h = Shape2D::Hexagon(hexWidth, Vec2(hexWidth * Math::Sqrt3 * x + hexWidth * Math::Sqrt3 / 2 + camPos.x, hexWidth * 1.5 * y + camPos.y));
                }
                h.draw(ColorF(0.3, 0.3, 0.9, 0.3)); // Changed color for visibility
                debugFont(val).draw(Arg::center = h.asPolygon().centroid(), Palette::White);
            }
        }
    }
}

GameUIManager::GameAction GameUIManager::HandleGameGUI(bool canEndTurn, bool isFactorySelected, const Factory* currentFactory, int currentTeamMoney) {
    Vec2 produceButtonPos = factoryProductionUIRect.bl() + Vec2(20, -60); // Bottom-left area of panel
    Vec2 cancelButtonPos = factoryProductionUIRect.br() + Vec2(-120, -60); // Bottom-right area of panel
    Vec2 endTurnButtonPos = Vec2(50, 550); // Original position

    if (isFactorySelected && currentFactory) {
        bool canAfford = currentTeamMoney >= currentFactory->gold;
        // bool tileOccupied = unitManager->GetUnitAt(currentFactory->pos) != nullptr; // This check should be done by GameController before calling
        if (SimpleGUI::Button(U"購入", produceButtonPos, 100, canAfford /*&& !tileOccupied*/)) {
            return GameAction::ProduceUnit;
        }
        if (SimpleGUI::Button(U"中止", cancelButtonPos, 100)) {
            return GameAction::CancelProduction;
        }
    } else {
        if (SimpleGUI::Button(U"ターン終了", endTurnButtonPos, 180, canEndTurn)) {
            return GameAction::EndTurn;
        }
    }
    return GameAction::None;
}

GameUIManager::GameAction GameUIManager::HandleEndGameGUI() {
    if (SimpleGUI::ButtonAt(U"タイトルへ戻る", gameEndUIRect.center() + Vec2(0, 40), 200)) {
        return GameAction::BackToTitle;
    }
    return GameAction::None;
}
