#pragma once
#include <Siv3D.hpp>

enum class SceneState
{
	title,
	game,
	edit
};

enum class GameMode
{
	LocalMode,
	CPUMode,
	NetMode
};

struct Chip
{
	String name;
	HSV color;
	int cost;
};

enum class UnitType
{
	Tank,
	Heli,
	Human,
	Max
};

enum class Teams
{
	Red,
	Blue,
	None
};

struct UnitData
{
	int hp;
	String name;
	int power;
	int speed;
	int range;
	UnitType advUnit = UnitType::Tank;
};

struct Fort
{
	Point pos;
	Teams team;
	bool isCapture;
	ColorF color;
};

struct City
{
	String name;
	Point pos;
	int plusMoney;
	ColorF color;
	bool isCapture = false;
	Teams team;
};

// Forward declaration for Unit in case Factory needs it before full definition (though not strictly necessary here as Unit is small)
struct Unit;

struct Factory
{
	UnitType makeUnit;
	Point pos;
	int gold;
	Teams team;
	ColorF color;
	Unit* CreateUnit()
	{
		Unit* u = new Unit(); // This will require Unit to be defined or properly forward-declared if it were more complex.
		// For now, assuming basic new Unit() is fine and details are set externally or by constructor if added.
		// Logic from Main.cpp for CreateUnit:
		// u->pos = pos;
		// u->team = team;
		// u->type = makeUnit;
		// To make this work here, Unit needs these members.
		// Since Unit is defined next, this will compile.
		return u;
	}
};

struct Unit
{
	UnitType type;
	Point pos;
	Array<Point> movePos;
	double moveTimer;
	bool isMove = false;
	bool isAttack = false;
	int hp;
	ColorF color;
	Teams team;
	Point prePos;
	Vec2 drawPos;

	bool HpJudge(int atk)
	{
		hp -= atk;
		if (hp <= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

struct Team
{
	String name;
	ColorF color;
	int money;
};

// Note: The Display class will be moved to GameUIManager.h as per instructions.
// The Game class and Edit class from Main.cpp will be refactored into the respective manager classes.
