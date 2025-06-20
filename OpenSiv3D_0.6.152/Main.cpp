#include <Siv3D.hpp> // OpenSiv3D v0.6.10
enum SceneState
{
	title,
	game,
	edit
};
enum GameMode
{
	LocalMode,
	CPUMode,
	NetMode
};
int state = title;
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
struct Factory
{
	UnitType makeUnit;
	Point pos;
	int gold;
	Teams team;
	ColorF color;
	Unit* CreateUnit()
	{
		Unit* u = new Unit();
		u->pos = pos;
		u->team = team;
		u->type = makeUnit;
		return u;
	}
};
struct Team
{
	String name;
	ColorF color;
	int money;
};
class Display
{
	struct TextAnim
	{
		bool isStart = false;

		//アニメーション前後の時間
		double timer = 0;
		double time;
		//アニメーション前後のスクロール位置
		double preScroll;
		double targetScroll;
	};
	Array<TextAnim> animArray;
	Point pos;
	Point size;
	Array<String> texts;
	bool isMoving;
	double scroll;
	Font font;
	void SetAnimation(double _targetSc, double _animTime)
	{
		TextAnim anim;
		anim.targetScroll = _targetSc;
		anim.time = _animTime;
		animArray.emplace_back(anim);
	}
	void Draw()
	{
		const ScopedViewport2D viewport{ pos.x,pos.y,size.x,size.y };
		const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(pos) };
		Rect back{ 0,0,size.x,size.y };
		back.draw(ColorF(0, 0, 0, 0.3));
		for (int i = 0; i < texts.size(); ++i)
		{
			//Rect(0, font.height() * i - scroll, size.x, font.height()).rounded(25).drawFrame(2, ColorF(0.8, 0.3, 0.1, 1));
			font(texts[i]).draw(10, font.height() * i - scroll);
		}
	}
	TextAnim ScrollAnimation(TextAnim _anim)
	{
		if (!_anim.isStart)
		{
			_anim.preScroll = scroll;
			_anim.isStart = true;
		}
		_anim.timer += Scene::DeltaTime();
		double percent = _anim.timer / _anim.time;
		double plusValue = _anim.targetScroll - _anim.preScroll;
		scroll = _anim.preScroll + plusValue * EaseInOutSine(percent);
		return _anim;
	}
public :
	Display(Point _pos,Point _size,Font _f)
	{
		pos = _pos;
		size = _size;
		font = _f;
		scroll = 0;
	}
	
	void Add(String _text)
	{
		texts.emplace_back(_text);
		
		double height = (texts.size() * font.height());
		height -= size.y;
		
		if (height < 0)
		{
			scroll = 0;
		}
		else
		{
			SetAnimation(height, 0.5);
		}
	}
	void MouseScroll(double _moveValue)
	{
		if (animArray.empty())
		{
			double height = font.height() * texts.size();
			height -= size.y;
			if (height < 0)
			{
				height = 0;
			}
			scroll += _moveValue;

			if (scroll < 0)
			{
				scroll = 0;
			}
			else if (scroll > height)
			{
				scroll = height;
			}
		}
	}
	void Update()
	{
		if (!animArray.empty())
		{
			TextAnim t = ScrollAnimation(animArray[0]);
			bool endAnim = t.time <= t.timer;
			if (endAnim)
			{
				animArray.erase(animArray.begin());
			}
			else
			{
				animArray[0] = t;
			}
		}
		Draw();
	}
};
class Game
{
private:
	enum Mode
	{
		local,
		net,
		CPU
	};
	Mode mode = local;
	Grid<int32>map;
	CSV mapCSV{U"CSVFile/mapCSV.csv"};
	Grid<int32> drawMap = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};
	Grid<int32> attackRange ={
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};
	Grid<int32> cpuDisRange = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};
	Grid<int32> drawEvaRange = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};
	HashTable<int32, Chip> chipdata;
	HashTable<UnitType, UnitData> unitDB;
	HashTable<Teams, Team> teamList;
	Array<Unit*> unitList;
	Array<City*> cityList;
	Array<Factory*> factoryList;
	Array<Fort*> fortList;
	Unit* movingUnit= nullptr;
	enum SceneEnum
	{
		game,
		end
	};
	SceneEnum scene = game;
	Unit* targetUnit = nullptr;
	const Audio music{ U"bgm&se/BGM.mp3" };
	const Audio tEndSound{ U"bgm&se/決定ボタン.mp3" };
	const Audio boomSound{ U"bgm&se/爆発1.mp3" };
	const Audio gunSound{ U"bgm&se/重機関銃を乱射1.mp3" };
	const Audio cannonSound{ U"bgm&se/大砲1.mp3" };
	const Audio bigBoomSound{ U"bgm&se/爆発2.mp3" };
	const Audio endSound{ U"bgm&se/歓声と拍手.mp3" };
	const Audio buySound{ U"bgm&se/レジスターで精算.mp3" };
	Factory* targetFact = nullptr;
	Font font{ 40,Typeface::Bold };
	Font moneyFont{ 20,Typeface::Black };
	Font nameFont{ 25,Typeface::Black };
	Font factoryFont{ 22,Typeface::Bold };
	Vec2 camPos{ 0,0 };
	Display console = Display(Point(230, 500), Point(570, 80), factoryFont);
	int turn = 1;
	Timer timer;
	double waitTime = 0.2;
	double moveTime = 0.2;
	int moveCount = 0;
	int useCost = 0;
	double scale = 1;
	double size = 50;
	double mapSize = 20;
	bool isPause = false;
	bool isStartTurn = false;
	double width = size * scale;
	Teams cpuTeam = Teams::Red;
	Teams nowTeam = Teams::Red;
	int maxTeamNo = 0;
	void InitMoveRange()
	{
		for (auto y : step(drawMap.height()))
		{
			for (auto x : step(drawMap.width()))
			{
				drawMap[y][x] = -1;
			}
		}
	}
	void InitAttackRange()
	{
		for (auto y : step(attackRange.height()))
		{
			for (auto x : step(attackRange.width()))
			{
				attackRange[y][x] = -1;
			}
		}
	}
	void InitDisRange()
	{
		for (auto y : step(cpuDisRange.height()))
		{
			for (auto x : step(cpuDisRange.width()))
			{
				cpuDisRange[y][x] = 99;
			}
		}
	}
	void InitEvaRange()
	{
		for (auto y : step(cpuDisRange.height()))
		{
			for (auto x : step(cpuDisRange.width()))
			{
				drawEvaRange[y][x] = 99;
			}
		}
	}
	void GameUpdate()
	{
		if (KeySpace.pressed())
		{
			isPause = true;
		}
		else
		{
			isPause = false;
		}
		width = size * scale;
		//画面の拡大・縮小
		if (Mouse::Wheel() != 0)
		{
			if (Rect(Point(230, 500), Point(570, 80)).mouseOver())
			{
				console.MouseScroll(Mouse::Wheel() * 15);
			}
			else
			{
				scale -= Mouse::Wheel() / 15;
				if (Mouse::Wheel() > 0)
				{
					camPos = Vec2(camPos.x + width, camPos.y + width);
				}
				else
				{
					camPos = Vec2(camPos.x - width, camPos.y - width);
				}
			}
			
		}
		//カメラ位置調整
		if (MouseM.pressed())
		{
			camPos += Cursor::Delta();
		}
		//終了処理
		int count = 0;
		for (auto&& fo : fortList)
		{
			if (fo->isCapture)
			{
				count++;
			}
		}
		if (count == 1)
		{
			scene = end;
			endSound.playOneShot();
		}
		if (!CPUUpdate(cpuTeam,mode))
		{
			if (!MoveUnit())
			{
				for (auto&& u : unitList)
				{
					if (u->team != nowTeam) continue;
					Shape2D unit;
					if (u->pos.y % 2 == 0)
					{
						unit = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * u->pos.x + camPos.x, width * 1.5 * u->pos.y + camPos.y));
					}
					else
					{
						unit = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * u->pos.x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * u->pos.y + camPos.y));
					}
					if (unit.asPolygon().leftClicked() && !u->isAttack && targetFact == nullptr)
					{
						targetUnit = u;
					}
				}

				if (targetUnit != nullptr)
				{
					if (!targetUnit->isMove)
					{
						if (MouseL.down())
						{
							for (auto y : step(map.height()))
							{
								for (auto x : step(map.width()))
								{
									Shape2D r;
									if (y % 2 == 0)
									{
										r = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y));

									}
									else
									{
										r = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y));
									}

									if (r.asPolygon().leftClicked())
									{
										Point pos{ x,y };
										InitMoveRange();
										int speed = unitDB[targetUnit->type].speed;
										MoveCheck(speed, targetUnit->pos);

										if (drawMap[pos.y][pos.x] >= 0 && pos != targetUnit->pos)
										{
											useCost = speed - drawMap[pos.y][pos.x];
											movingUnit = targetUnit;
											InitMoveRange();

											Array<Point> route{};
											MakeRoute(useCost, targetUnit->pos, Array<Point>(), pos, &route);
											movingUnit->movePos = route;
											targetUnit = nullptr;
										}
									}
								}
							}
						}
						else if (MouseR.down())
						{
							targetUnit = nullptr;
						}

					}
					else if (!targetUnit->isAttack)
					{
						if (MouseL.down())
						{
							for (auto y : step(map.height()))
							{
								for (auto x : step(map.width()))
								{
									Shape2D r;
									if (y % 2 == 0)
									{
										r = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y));

									}
									else
									{
										r = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y));
									}
									if (r.asPolygon().leftClicked())
									{
										Point pos{ x,y };
										InitAttackRange();
										int range = unitDB[targetUnit->type].range;
										AttackCheck(range, targetUnit->pos);
										if (attackRange[pos.y][pos.x] >= 0 && pos != targetUnit->pos)
										{
											for (auto&& u : unitList)
											{
												if (targetUnit->team != u->team)
												{
													if (u->pos == pos)
													{
														targetUnit->isAttack = true;
														Battle(targetUnit, u);
														
														targetUnit = nullptr;
														break;
													}
												}
											}
										}
									}
								}
							}
						}
						else if (MouseR.down())
						{
							targetUnit = nullptr;
						}
					}

				}
				else
				{
					for (auto&& f : factoryList)
					{
						if (f->team != nowTeam) continue;
						Shape2D fact;
						if (f->pos.y % 2 == 0)
						{
							fact = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * f->pos.x + camPos.x, width * 1.5 * f->pos.y + camPos.y));
						}
						else
						{
							fact = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * f->pos.x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * f->pos.y + camPos.y));
						}
						if (fact.asPolygon().leftClicked())
						{
							targetFact = f;
						}
					}
				}

				
			}
		}
		else
		{

		}
		/*	for (auto&& u : unitList)
			{
				if (u->team == red)
				{
					redFlag = true;
				}
				else if (u->team == blue)
				{
					blueFlag = true;
				}
			}*/
	}
	bool CPUUpdate(Teams team,Mode _mode)
	{
		bool isUpdate = false;
		if (isPause) return true;
		if (_mode == Mode::CPU)
		{
			if (nowTeam != team) return isUpdate;
			if (!MoveUnit())
			{
				if (!isStartTurn)
				{
					CPUCreateJudge(team);
					isStartTurn = false;
				}
				

				Unit* enemy = nullptr;
				Unit* myUnit = nullptr;
				for (auto&& u : unitList)
				{
					if (u->team != team)continue;
					if (!u->isMove)
					{
						isUpdate = true;
						InitMoveRange();
						InitEvaRange();
						int speed = unitDB[u->type].speed;
						MoveCheck(speed, u->pos);
						Array<Point> newPoss;
						int eva = 99;
						for (auto y : step(map.height()))
						{
							for (auto x : step(map.width()))
							{
								InitMoveRange();
								MoveCheck(speed, u->pos);
								Point pos = Point(x, y);
								if (drawMap[y][x] == - 1) continue;
								//if (pos == u->pos)continue;
								int newEva = EvaCheck(pos,u);
								drawEvaRange[y][x] = newEva;
								if (eva > newEva)
								{
									eva = newEva;
									newPoss.clear();
									newPoss.emplace_back(pos);
								}
								else if (eva == newEva)
								{
									newPoss.emplace_back(pos);
								}

								
								if (eva == 0)
								{
									break;
								}
							}
						}
						Point p;
						if (newPoss.size() != 0)
						{
							p = newPoss.choice();

							InitMoveRange();
							MoveCheck(speed, u->pos);
							useCost = speed - drawMap[p.y][p.x];
							movingUnit = u;
							InitMoveRange();
							Array<Point> route;
							MakeRoute(useCost, movingUnit->pos, Array<Point>(), p, &route);
							movingUnit->movePos = route;
							break;
						}
						else
						{
							u->isMove = true;
						}
					}
					else if (!u->isAttack)
					{
						
						isUpdate = true;
						u->isAttack = true;
						for (auto&& enemyU : unitList)
						{
							if (u->team == enemyU->team) continue;
							InitAttackRange();
							int range = unitDB[u->type].range;
							AttackCheck(range, u->pos);
							Point pos{ enemyU->pos.x,enemyU->pos.y };

							if (attackRange[pos.y][pos.x] >= 0 && (unitDB[u->type].advUnit == enemyU->type))
							{
								enemy = enemyU;
								myUnit = u;
								break;
							}
							
						}
						
					}
				}
				if (enemy != nullptr)
				{
					Battle(myUnit,enemy);
				}
			}
			else
			{
				isUpdate = true;
			}
			if (!isUpdate)
			{
				EndTurn();
			}
		}

		
		
		return isUpdate;
	}
	void CPUCreateJudge(Teams team)
	{
		Array<Factory*> factories;
		for (auto&& f : factoryList)
		{
			if (f->team != team)continue;
			factories.emplace_back(f);
		}
		HashTable<UnitType, int>teamUnitValue;
		for (int i = 0; i < (int)UnitType::Max; ++i)
		{
			teamUnitValue.emplace((UnitType)i, 0);
		}
		for (auto&& u : unitList)
		{
			if (u->team != team)continue;
			teamUnitValue[u->type]++;
		}
		
		bool finUnit[(int)UnitType::Max]{ false };
		while (true)
		{
			UnitType t = UnitType::Max;
			for (int i = 0; i < (int)UnitType::Max; ++i)
			{
				if (finUnit[i])continue;
				if (teamUnitValue[(UnitType)i] < teamUnitValue[t])
				{
					t = (UnitType)i;
				}
			}
			if (t == UnitType::Max)
			{
				return;
			}
			bool haveUnit = false;
			for (auto&& f : factories)
			{
				
				if (f->makeUnit == t)
				{
					haveUnit = true;
					if (f->gold > teamList[team].money)
					{
						finUnit[(int)t] = true;
					}
					else
					{
						Unit* newUnit = f->CreateUnit();
						newUnit->color = f->color;
						newUnit->prePos = newUnit->pos;
						newUnit->drawPos = newUnit->pos;
						//u->hp = unitDB[u->type].hp;
						unitList.emplace_back(newUnit);
						finUnit[(int)t] = true;
						return;
					}
				}
			}
			if (!haveUnit) return;
		}
		
		
	}
	void Battle(Unit* a, Unit* b)
	{
		console.Add(teamList[a->team].name + U"軍の" + unitDB[a->type].name + U"の攻撃！");
		if (a->type == b->type)
		{
			unitList.remove(a);
			unitList.remove(b);
			bigBoomSound.playOneShot();
		}
		else if (unitDB[a->type].advUnit == b->type)
		{
			unitList.remove(b);
			console.Add(teamList[a->team].name + U"軍の" + unitDB[a->type].name + U"の勝ち！");
			//サウンド処理
			switch(a->type)
			{
			case UnitType::Tank :
				boomSound.playOneShot();
				break;
			case UnitType::Heli :
				gunSound.playOneShot();
				break;
			case UnitType::Human:
				cannonSound.playOneShot();
				break;
			}
		}
		else
		{
			unitList.remove(a);
			console.Add(teamList[b->team].name + U"軍の" + unitDB[b->type].name + U"の勝ち！");
			//サウンド処理
			switch (b->type)
			{
			case UnitType::Tank:
				boomSound.playOneShot();
				break;
			case UnitType::Heli:
				gunSound.playOneShot();
				break;
			case UnitType::Human:
				cannonSound.playOneShot();
				break;
			}
		}
	}
	bool MoveUnit()
	{
		if (movingUnit != nullptr)
		{
			scale = 1;
			if (moveTime <= movingUnit->moveTimer)
			{
				if (movingUnit->movePos.size() != moveCount)
				{
					movingUnit->prePos = movingUnit->movePos[moveCount];
					movingUnit->moveTimer = 0;
					moveCount++;
				}
				if (movingUnit->movePos.size() == moveCount)
				{
					movingUnit->isMove = true;
					movingUnit->drawPos = movingUnit->movePos[moveCount - 1];
					movingUnit->prePos = Point((int)movingUnit->drawPos.x, (int)movingUnit->drawPos.y);
					movingUnit->moveTimer = 0;
					useCost = 0;
					movingUnit->pos = movingUnit->movePos[moveCount - 1];
					for (auto&& c : cityList)
					{
						if (c->pos == movingUnit->pos)
						{
							c->team = movingUnit->team;
							c->color = movingUnit->color;
							c->isCapture = true;
						}
					}
					for (auto&& f : fortList)
					{
						if (f->team == movingUnit->team) continue;
						if (f->pos == movingUnit->pos)
						{
							f->isCapture = true;
						}
					}
					movingUnit->movePos.clear();
					moveCount = 0;
					movingUnit = nullptr;
					return false;
				}
				
			}
			Vec2 movePos = Vec2(0,0);
			if (movingUnit->movePos[moveCount].y != movingUnit->prePos.y)
			{
				if (movingUnit->movePos[moveCount].y % 2 == 0)
				{
						movePos.x -= 0.5;
				}
				else
				{			
						movePos.x += 0.5;		
				}

			}
			movingUnit->moveTimer += Scene::DeltaTime();
			if (moveTime > movingUnit->moveTimer)
			{
				movingUnit->drawPos = movingUnit->prePos + ((movingUnit->movePos[moveCount] - movingUnit->prePos) + movePos) * (movingUnit->moveTimer / moveTime);
				if (movingUnit->prePos.y % 2 == 0)
				{
					camPos = -Vec2(width * sqrt(3) * movingUnit->drawPos.x - width * 8, width * 1.5 * movingUnit->drawPos.y - width * 5);
				}
				else
				{
					camPos = -Vec2(width * sqrt(3) * movingUnit->drawPos.x + width * sqrt(3) / 2 - width * 8, width * 1.5 * movingUnit->drawPos.y - width * 5);
				}
				
			}
		}
		else
		{
			return false;
			
		}
		return true;
	}
	void DrawMap()
	{
		for (auto y : step(map.height()))
		{
			for (auto x : step(map.width()))
			{
				int chipNo = map[y][x];
				if (y % 2 == 0)
				{
					Shape2D h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y)).draw(chipdata[chipNo].color);
					h.drawFrame(1, Palette::Black);
				}
				else
				{
					Shape2D h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y)).draw(chipdata[chipNo].color);
					h.drawFrame(1, Palette::Black);
				}

				//RectF{ x * width + camPos.x,y * width + camPos.y,width }.draw(chipdata[chipNo].color);
			}
		}
	}
	void DrawMove()
	{
		if (targetUnit != nullptr)
		{
			if (!targetUnit->isMove)
			{
				int unitSpeed = unitDB[targetUnit->type].speed;
				InitMoveRange();
				MoveCheck(unitSpeed, targetUnit->pos);
				for (auto y : step(map.height()))
				{
					for (auto x : step(map.width()))
					{
						int moveNo = drawMap[y][x];
						if (moveNo >= 0)
						{
							if (y % 2 == 0)
							{
								Shape2D h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y)).draw(ColorF(0.8, 0.1, 0.1, 0.3));
								//font(moveNo).draw(Arg::center(h.asPolygon().centroid()));
							}
							else
							{
								Shape2D h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y)).draw(ColorF(0.8, 0.1, 0.1, 0.3));
								//font(moveNo).draw(Arg::center(h.asPolygon().centroid()));
							}
							
						}
					}
				}
			}
		}

	}
	void DrawAttackRange()
	{
		if (targetUnit != nullptr)
		{
			if (targetUnit->isMove)
			{
				int unitRange = unitDB[targetUnit->type].range;
				InitAttackRange();
				AttackCheck(unitRange, targetUnit->pos);
				for (auto y : step(map.height()))
				{
					for (auto x : step(map.width()))
					{
						int atkNo = attackRange[y][x];
						if (atkNo >= 0)
						{
							if (y % 2 == 0)
							{
								Shape2D h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y)).draw(ColorF(0.8, 0.0, 0.0, 0.5));
								//font(atkNo).draw(Arg::center(h.asPolygon().centroid()));
							}
							else
							{
								Shape2D h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y)).draw(ColorF(0.8, 0.0, 0.0, 0.5));
								//font(atkNo).draw(Arg::center(h.asPolygon().centroid()));
							}
							//font(atkNo).draw(x * width + 10 + camPos.x, (y * width) + camPos.y);
						}
					}
				}
			}
		}
	}
	void DrawUnit()
	{
	
		for (auto&& u : unitList)
		{
			Shape2D r;
			
			if (u->prePos.y % 2 == 0)
			{
				r = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * u->drawPos.x + camPos.x, width * 1.5 * u->drawPos.y + camPos.y)).draw(ColorF(0.8, 0.0, 0.0, 0.5));
			}
			else
			{
				r = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * u->drawPos.x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * u->drawPos.y + camPos.y)).draw(ColorF(0.8, 0.0, 0.0, 0.5));
			}
			r.draw(u->color);
			if (u->isMove)
			{
				r.drawFrame(3, Palette::White);
			}
			else if (targetUnit != nullptr)
			{
				if (u == targetUnit)
				{
					r.drawFrame(2, Palette::Black);
				}
				else
				{
					r.drawFrame(2, ColorF(u->color.rgba() + Vec4(0.6, 0.6, 0.6, 0)));
				}
			}
			else
			{
				{
					r.drawFrame(2, ColorF(u->color.rgba() + Vec4(0.6, 0.6, 0.6, 0)));
				}
			}
			//nameFont(unitDB[u->type].name).drawAt(u->pos.x * 50 + 23, (u->pos.y * 50) + 20);
			if (scale >= 0.6)
			{
				nameFont(unitDB[u->type].name).draw(Arg::center(r.asPolygon().centroid()));
			}

			/*Rect hpBackRect{ u->pos.x * 50,u->pos.y * 50,55,15};
			hpBackRect.pos.x -= 2; hpBackRect.pos.y -= 5;
			hpBackRect.draw(Palette::Black);
			RectF hpRect{ hpBackRect.leftCenter().x + 2,hpBackRect.pos.y + 2,52 * (static_cast<double>(u->hp) / unitDB[u->type].hp),10 };
			RectF hpRedRect{ hpBackRect.leftCenter().x + 2,hpBackRect.pos.y + 2,52,10 };
			hpRedRect.draw(Palette::Red);
			hpRect.draw(Palette::Greenyellow);*/

		}
		
	}
	void DrawCity()
	{
		for (auto&& c : cityList)
		{

			Shape2D r;
			if (c->pos.y % 2 == 0)
			{
				r = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * c->pos.x + camPos.x, width * 1.5 * c->pos.y + camPos.y)).draw(ColorF(0.8, 0.0, 0.0, 0.5));
			}
			else
			{
				r = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * c->pos.x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * c->pos.y + camPos.y)).draw(ColorF(0.8, 0.0, 0.0, 0.5));

			}
			r.drawFrame(3, c->color);
			r.draw(Palette::White);
			nameFont(c->name).draw(Arg::center(r.asPolygon().centroid()), Palette::Black);
		}
	}
	void DrawFort()
	{
		for (auto&& f : fortList)
		{
			Shape2D fort;
			if (f->pos.y % 2 == 0)
			{
				fort = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * f->pos.x + camPos.x, width * 1.5 * f->pos.y + camPos.y)).draw(ColorF(0.8, 0.0, 0.0, 0.5));
			}
			else
			{
				fort = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * f->pos.x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * f->pos.y + camPos.y)).draw(ColorF(0.8, 0.0, 0.0, 0.5));

			}
			fort.draw(Palette::White); fort.drawFrame(2, f->color);
			nameFont(U"要塞").draw(Arg::center(fort.asPolygon().centroid()),Palette::Black);
		}
	}
	void DrawDebug()
	{
		if(targetUnit == nullptr && isPause)
		for (auto y : step(map.height()))
		{
			for (auto x : step(map.width()))
			{
				int moveNo = drawEvaRange[y][x];
				if (moveNo >= 0)
				{
					if (y % 2 == 0)
					{
						Shape2D h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y)).draw(ColorF(0.3, 0.3, 0.3, 0.3));
						font(moveNo).draw(Arg::center(h.asPolygon().centroid()));
					}
					else
					{
						Shape2D h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y)).draw(ColorF(0.3, 0.3, 0.3, 0.3));
						font(moveNo).draw(Arg::center(h.asPolygon().centroid()));
					}

				}
			}
		}
	}
	void DrawFactory()
	{
			for (auto&& f : factoryList)
			{
				Shape2D r;
				if (f->pos.y % 2 == 0)
				{
					r = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * f->pos.x + camPos.x, width * 1.5 * f->pos.y + camPos.y)).draw(ColorF(0.8, 0.0, 0.0, 0.5));
				}
				else
				{
					r = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * f->pos.x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * f->pos.y + camPos.y)).draw(ColorF(0.8, 0.0, 0.0, 0.5));

				}
				r.draw(Palette::White);
				r.drawFrame(2, f->color);
				factoryFont(unitDB[f->makeUnit].name + U"\n工場").draw(Arg::center(r.asPolygon().centroid()), Palette::Black);
			}
		
	}
	void MoveCheck(int n, Point pos)
	{

		drawMap[pos.y][pos.x] = n;
		if (pos.y % 2 == 0)
		{
			for (int i = 0; i < 6; ++i)
			{
				//↗
				if (i == 0)
				{
					if (pos.y > 0)
					{
						Point p = {pos.x,pos.y - 1};
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}

						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{

							MoveCheck(s,p);//自身を呼び出す。
						}

					}
				}
				//→
				else if (i == 1)
				{
					if (pos.x < map.width() - 1)
					{
						Point p = { pos.x + 1,pos.y};
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit) {
							MoveCheck(s,p);
						}

					}
				}
				//↘
				else if (i == 2)
				{
					if (pos.y < map.height() - 1)
					{
						Point p = { pos.x,pos.y + 1};
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MoveCheck(s,p);
						}

					}
				}
				//↙
				else if (i == 3)
				{
					if (pos.x > 0 && pos.y < map.height() - 1)
					{
						Point p = { pos.x - 1,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MoveCheck(s,p);
						}

					}
				}
				//←
				else if (i == 4)
				{
					if (pos.x > 0)
					{
						Point p = { pos.x - 1,pos.y};
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MoveCheck(s, p);
						}

					}
				}
				//↖
				else if (i == 5)
				{
					if (pos.x > 0 && pos.y > 0)
					{
						Point p = { pos.x - 1,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MoveCheck(s, p);
						}

					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 6; ++i)
			{
				//↗
				if (i == 0)
				{
					if (pos.y > 0 && pos.x < map.width() - 1)
					{
						Point p = { pos.x + 1,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}

						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{

							MoveCheck(s, p);//自身を呼び出す。
						}

					}
				}
				//→
				else if (i == 1)
				{
					if (pos.x < map.width() - 1)
					{
						Point p = { pos.x + 1,pos.y };
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit) {
							MoveCheck(s, p);
						}

					}
				}
				//↘
				else if (i == 2)
				{
					if (pos.y < map.height() - 1 && pos.x < map.width() - 1)
					{
						Point p = { pos.x + 1,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MoveCheck(s, p);
						}

					}
				}
				//↙
				else if (i == 3)
				{
					if (pos.y < map.height() - 1)
					{
						Point p = { pos.x,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MoveCheck(s, p);
						}

					}
				}
				//←
				else if (i == 4)
				{
					if (pos.x > 0)
					{
						Point p = { pos.x - 1,pos.y };
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MoveCheck(s, p);
						}

					}
				}
				//↖
				else if (i == 5)
				{
					if (pos.y > 0)
					{
						Point p = { pos.x,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = n - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MoveCheck(s, p);
						}

					}
				}
			}
		}
	}
	void AttackCheck(int n, Point pos)
	{

		attackRange[pos.y][pos.x] = n;
		if (n <= -1)
		{
			return;
		}
		if (pos.y % 2 == 0)
		{
			for (int i = 0; i < 6; ++i)
			{
				//↗
				if (i == 0)
				{
					if (pos.y > 0)
					{
						if (attackRange[pos.y - 1][pos.x] < n)
						{
							AttackCheck(n - 1, Point(pos.x, pos.y - 1));//自身を呼び出す。
						}
					}
				}
				//→
				else if (i == 1)
				{
					if (pos.x < map.width() - 1)
					{
						if (attackRange[pos.y][pos.x + 1] < n)
						{

							AttackCheck(n - 1, Point(pos.x + 1, pos.y));//自身を呼び出す。
						}
					}
				}
				//↘
				else if (i == 2)
				{
					if (pos.y < map.height() - 1)
					{
						if (attackRange[pos.y + 1][pos.x] < n)
						{

							AttackCheck(n - 1, Point(pos.x, pos.y + 1));//自身を呼び出す。
						}
					}
				}
				//↙
				else if (i == 3)
				{
					if (pos.x > 0 &&pos.y < map.height() - 1)
					{
						if (attackRange[pos.y + 1][pos.x - 1] < n)
						{

							AttackCheck(n - 1, Point(pos.x - 1, pos.y + 1));//自身を呼び出す。
						}
					}
				}
				//←
				else if (i == 4)
				{
					if (pos.x > 0)
					{
						if (attackRange[pos.y][pos.x - 1] < n)
						{

							AttackCheck(n - 1, Point(pos.x - 1, pos.y));//自身を呼び出す。
						}
					}
				}
				//↖
				else if (i == 5)
				{
					if (pos.x > 0 && pos.y > 0)
					{
						if (attackRange[pos.y - 1][pos.x - 1] < n)
						{

							AttackCheck(n - 1, Point(pos.x - 1, pos.y - 1));//自身を呼び出す。
						}
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 6; ++i)
			{
				//↗
				if (i == 0)
				{
					if (pos.x < map.width() - 1 && pos.y > 0)
					{
						if (attackRange[pos.y - 1][pos.x + 1] < n)
						{
							AttackCheck(n - 1, Point(pos.x + 1, pos.y - 1));//自身を呼び出す。
						}
					}
				}
				//→
				else if (i == 1)
				{
					if (pos.x < map.width() - 1)
					{
						if (attackRange[pos.y][pos.x + 1] < n)
						{

							AttackCheck(n - 1, Point(pos.x + 1, pos.y));//自身を呼び出す。
						}
					}
				}
				//↘
				else if (i == 2)
				{
					if (pos.x < map.width() - 1 && pos.y < map.height() - 1)
					{
						if (attackRange[pos.y + 1][pos.x + 1] < n)
						{

							AttackCheck(n - 1, Point(pos.x + 1, pos.y + 1));//自身を呼び出す。
						}
					}
				}
				//↙
				else if (i == 3)
				{
					if (pos.y < map.height() - 1)
					{
						if (attackRange[pos.y + 1][pos.x] < n)
						{

							AttackCheck(n - 1, Point(pos.x, pos.y + 1));//自身を呼び出す。
						}
					}
				}
				//←
				else if (i == 4)
				{
					if (pos.x > 0)
					{
						if (attackRange[pos.y][pos.x - 1] < n)
						{

							AttackCheck(n - 1, Point(pos.x - 1, pos.y));//自身を呼び出す。
						}
					}
				}
				//↖
				else if (i == 5)
				{
					if (pos.y > 0)
					{
						if (attackRange[pos.y - 1][pos.x] < n)
						{

							AttackCheck(n - 1, Point(pos.x, pos.y - 1));//自身を呼び出す。
						}
					}
				}
			}
		}

	}
	void CPUCheck(int n, Point pos)
	{

		cpuDisRange[pos.y][pos.x] = n;
		Point p = { pos.x,pos.y};
		if (pos.y % 2 == 0)
		{
			for (int i = 0; i < 6; ++i)
			{
				//↗
				if (i == 0)
				{
					if (pos.y > 0)
					{
						p = { pos.x,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}

					}
				}
				//→
				else if (i == 1)
				{
					if (pos.x < map.width() - 1)
					{
						p = { pos.x + 1,pos.y };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}

					}
				}
				//↘
				else if (i == 2)
				{
					if (pos.y < map.height() - 1)
					{
						p = { pos.x,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}

					}
				}
				//↙
				else if (i == 3)
				{
					if (pos.x > 0 && pos.y < map.height() - 1)
					{
						p = { pos.x - 1,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}

					}
				}
				//←
				else if (i == 4)
				{
					if (pos.x > 0)
					{
						p = { pos.x - 1,pos.y };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}

					}
				}
				//↖
				else if (i == 5)
				{
					if (pos.x > 0 && pos.y > 0)
					{
						p = { pos.x - 1,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}

					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 6; ++i)
			{
				//↗
				if (i == 0)
				{
					if (pos.y > 0 && pos.x < map.width() - 1)
					{
						p = { pos.x + 1,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}
					}
				}
				//→
				else if (i == 1)
				{
					if (pos.x < map.width() - 1)
					{
						p = { pos.x + 1,pos.y };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s) {
							CPUCheck(s, p);
						}

					}
				}
				//↘
				else if (i == 2)
				{
					if (pos.y < map.height() - 1 && pos.x < map.width() - 1)
					{
						p = { pos.x + 1,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}

					}
				}
				//↙
				else if (i == 3)
				{
					if (pos.y < map.height() - 1)
					{
						p = { pos.x,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}

					}
				}
				//←
				else if (i == 4)
				{
					if (pos.x > 0)
					{
						p = { pos.x - 1,pos.y };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;

						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}

					}
				}
				//↖
				else if (i == 5)
				{
					if (pos.y > 0)
					{
						p = { pos.x,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = n + chipdata[chip].cost;
						if (cpuDisRange[p.y][p.x] > s)
						{
							CPUCheck(s, p);
						}
					}
				}
			}
		}
	}
	void MakeRoute(int cost, Point pos, Array<Point> route, Point targetPos,Array<Point>* _returnArray)
	{
		route.emplace_back(pos);
		if (pos == targetPos)
		{
			if (_returnArray->size() == 0)
			{
				for (auto&& r : route)
				{
					_returnArray->emplace_back(r);
				}
			}
			
			return;
		}
		drawMap[pos.y][pos.x] = cost;
		if (pos.y % 2 == 0)
		{
			for (int i = 0; i < 6; ++i)
			{
				//↗
				if (i == 0)
				{
					if (pos.y > 0)
					{
						Point p = { pos.x,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}

						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MakeRoute(s, p, route, targetPos,_returnArray);//自身を呼び出す。
						}
					}
				}
				//→
				else if (i == 1)
				{
					if (pos.x < map.width() - 1)
					{
						Point p = { pos.x + 1,pos.y };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit) {
							MakeRoute(s, p, route, targetPos,_returnArray);
						}

					}
				}
				//↘
				else if (i == 2)
				{
					if (pos.y < map.height() - 1)
					{
						Point p = { pos.x,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MakeRoute(s, p, route, targetPos, _returnArray);
						}

					}
				}
				//↙
				else if (i == 3)
				{
					if (pos.x > 0 && pos.y < map.height() - 1)
					{
						Point p = { pos.x - 1,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MakeRoute(s, p, route, targetPos, _returnArray);
						}

					}
				}
				//←
				else if (i == 4)
				{
					if (pos.x > 0)
					{
						Point p = { pos.x - 1,pos.y };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MakeRoute(s, p, route, targetPos, _returnArray);
						}

					}
				}
				//↖
				else if (i == 5)
				{
					if (pos.x > 0 && pos.y > 0)
					{
						Point p = { pos.x - 1,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MakeRoute(s, p, route, targetPos, _returnArray);
						}

					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 6; ++i)
			{
				//↗
				if (i == 0)
				{
					if (pos.y > 0 && pos.x < map.width() - 1)
					{
						Point p = { pos.x + 1,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}

						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{

							MakeRoute(s, p, route, targetPos, _returnArray);//自身を呼び出す。
						}

					}
				}
				//→
				else if (i == 1)
				{
					if (pos.x < map.width() - 1)
					{
						Point p = { pos.x + 1,pos.y };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit) {
							MakeRoute(s, p, route, targetPos,_returnArray);
						}

					}
				}
				//↘
				else if (i == 2)
				{
					if (pos.y < map.height() - 1 && pos.x < map.width() - 1)
					{
						Point p = { pos.x + 1,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MakeRoute(s, p, route, targetPos, _returnArray);
						}

					}
				}
				//↙
				else if (i == 3)
				{
					if (pos.y < map.height() - 1)
					{
						Point p = { pos.x,pos.y + 1 };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MakeRoute(s, p, route, targetPos, _returnArray);
						}

					}
				}
				//←
				else if (i == 4)
				{
					if (pos.x > 0)
					{
						Point p = { pos.x - 1,pos.y };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MakeRoute(s, p, route, targetPos, _returnArray);
						}

					}
				}
				//↖
				else if (i == 5)
				{
					if (pos.y > 0)
					{
						Point p = { pos.x,pos.y - 1 };
						int chip = map[p.y][p.x];
						int s = cost - chipdata[chip].cost;
						bool isUnit = false;
						for (auto&& u : unitList)
						{
							if (u->pos == p)
							{
								isUnit = true;
								break;
							}
						}
						if (s > -1 && drawMap[p.y][p.x] < s && !isUnit)
						{
							MakeRoute(s, p, route, targetPos, _returnArray);
						}

					}
				}
			}
		}

	}
	int EvaCheck(Point _pos,Unit* _myu)
	{
		int eva = 0;
		if (cpuDisRange[_pos.y][_pos.x] == 99)
		{
			return 100;
		}
		else
		{
			eva = cpuDisRange[_pos.y][_pos.x];
			if (eva == 0)
			{
				return 0;
			}
		}
		bool canAtk = false;
		for (auto&& u : unitList)
		{
			if (u->team == _myu->team)continue;
			if (u->type == unitDB[_myu->type].advUnit)
			{
				
						InitAttackRange();
						int range = unitDB[_myu->type].range;
						AttackCheck(range,_pos);
						if (attackRange[u->pos.y][u->pos.x] >= 0)
						{
							canAtk = true;
							canAtk -= 5;
							break;
						}
					if (canAtk) break;
			}
			else if(unitDB[u->type].advUnit == _myu->type)
			{
				InitMoveRange();
				int speed = unitDB[u->type].speed;
				MoveCheck(speed, u->pos);
				for (auto y : step(map.height()))
				{
					bool canAtk = false;
					for (auto x : step(map.width()))
					{
						Point pos{ x,y };
						if (drawMap[pos.y][pos.x] < 0) continue;
						InitAttackRange();
						int range = unitDB[u->type].range;
						AttackCheck(range,pos);
						if (attackRange[_pos.y][_pos.x] >= 0)
						{
							canAtk = true;
							eva += 10;
							break;
						}
					}

					if (canAtk) break;

				}
			}
		}
		
		

		return eva;
	}
	//int DisCalc(Point _pos1,Point _pos2)
	//{
	//	return Abs(_pos1.x - _pos2.x) + Abs(_pos1.y - _pos2.y);
	//}
	void DrawTurn()
	{
		Rect{ 500,0,300,500 }.draw(ColorF(0.01, 0.01, 0.01, 0.5));
		Rect r{ 530,10,250,150 }; r.rounded(20).draw(Palette::Grey).drawFrame(2, Palette::Whitesmoke);
		font(teamList[nowTeam].name + U"チーム\nのターン").draw(535, 20, teamList[nowTeam].color);
	}
	void DrawMoney()
	{
		for (int i = 0;i < teamList.size();++i)
		{
			moneyFont(teamList[(Teams)i].name + U"所持金:" + Format(teamList[(Teams)i].money)).draw(500, 350 + 45 * i, Palette::White);
		}
	}
	void DrawEndScene()
	{
		for (auto&& f : fortList)
		{
			if (!f->isCapture)
			{
				
				Rect r(Arg::center(Point(250, 250)), 500, 300); r.draw(ColorF(0.01, 0.01, 0.01, 0.5));
				font(teamList[f->team].name + U"の勝利！").draw(150, 210, teamList[f->team].color);
			}
		}
	}
	void DrawCityUI()
	{
		int count = 0;
		for (auto&& c : cityList)
		{
			Shape2D r;
			r = Shape2D::Hexagon(size * 0.8,Vec2(550,220 + 55 * count));
			r.drawFrame(3, c->color);
			r.draw(Palette::White);
			nameFont(c->name).draw(Arg::center(r.asPolygon().centroid()), Palette::Black);

			if (c->isCapture)
			{	
				moneyFont(teamList[c->team].name + U"チーム占領中").draw(590,r.asPolygon().centroid().y - 15, Palette::White);
			}
			count++;
		}
	}
	void DrawFactoryUI()
	{
		Rect r(Arg::center(Point(250, 250)), 500, 300); r.draw(ColorF(0.01, 0.01, 0.01, 0.5));
		font(unitDB[targetFact->makeUnit].name + U"を生産しますか？").draw(150, 210, Palette::White);
	}
	void EndTurn()
	{
		tEndSound.playOneShot();
		targetUnit = nullptr;
		isStartTurn = false;
		for (auto&& u : unitList)
		{
			if (u->team != nowTeam) continue;
			u->isAttack = false;
			u->isMove = false;
		}
		for (auto&& c : cityList)
		{
			if (c->team == nowTeam)
			{
				teamList[c->team].money += c->plusMoney;
			}
		}
		teamList[nowTeam].money += 100;
		nowTeam = (Teams)((int)nowTeam + 1);
		if (nowTeam == Teams::None)
		{
			nowTeam = (Teams)0;
			turn++;
		}
	}
	void OnGUI()
	{
		if (targetFact != nullptr)
		{
			bool isUnit = false;
			for (auto&& u : unitList)
			{
				if (u->pos == targetFact->pos)
				{
					isUnit = true;
				}
			}
			if (SimpleGUI::Button(U"購入", Vec2(450, 500), unspecified, targetFact->gold <= teamList[nowTeam].money && !isUnit))
			{
				buySound.playOneShot();
				teamList[nowTeam].money -= targetFact->gold;
				Unit* u = targetFact->CreateUnit();
				u->color = targetFact->color;
				u->prePos = u->pos;
				u->drawPos = u->pos;
				//u->hp = unitDB[u->type].hp;
				unitList.emplace_back(u);
				targetFact = nullptr;
			}
			else if (SimpleGUI::Button(U"やめる", Vec2(50, 500)))
			{
				tEndSound.playOneShot();
				targetFact = nullptr;
			}
		}
		else if (SimpleGUI::Button(U"ターンエンド", Vec2(50, 500),unspecified,movingUnit == nullptr))
		{
			EndTurn();
		}
	}
	void DrawEndGUI()
	{
		if (SimpleGUI::ButtonAt(U"タイトルへ", Vec2(450, 550)))
		{
			state = title;
		}
	}
public:
	Game(int _mode)
	{
		this->mode = (Mode)_mode;
		CSV unitDataCSV{ U"CSVFile/UnitData.csv" };
		for (int i = 1; i < unitDataCSV.rows(); ++i)
		{
			UnitData u;
			u.name = unitDataCSV[i][0];
			u.range = Parse<int>(unitDataCSV[i][1]);
			u.speed = Parse<int>(unitDataCSV[i][2]);
			u.advUnit = (UnitType)Parse<int>(unitDataCSV[i][3]);
			unitDB.emplace((UnitType)(i - 1), u);
		}
		Team red{ U"レッド",Palette::Red,100};
		Team blue{ U"ブルー",Palette::Blue,100 };
		
		factoryList.emplace_back(new Factory{ UnitType::Tank,Point(2,3),300,Teams::Red,red.color});
		factoryList.emplace_back(new Factory{ UnitType::Heli,Point(8,3),300,Teams::Blue,blue.color });
		teamList.emplace(Teams::Red,red);
		teamList.emplace(Teams::Blue,blue);

		CSV unitCSV{ U"CSVFile/UnitSetData.csv" };
		for (int y = 1; y < unitCSV.rows(); y++)
		{
			Teams t = (Teams)Parse<int>(unitCSV[y][0]);
			Unit* u = new Unit();
			u->type = (UnitType)Parse<int>(unitCSV[y][1]);
			u->pos = Parse<Point>(unitCSV[y][2]);
			u->team = t;
			u->color = teamList[t].color;
			unitList.emplace_back(u);
		}
		maxTeamNo = teamList.size();
		for (auto&& u : unitList)
		{
			u->color = teamList[u->team].color;
			u->drawPos = u->pos;
			u->prePos = u->pos;
		}
		teamList[nowTeam].money += 100;
		if (not mapCSV) // もし読み込みに失敗したら
		{
			throw Error{ U"Failed to load `MapFile.csv`" };
		}
		map = Grid<int32>(Parse<int32>(mapCSV[0][0]), Parse<int32>(mapCSV[1][0]), -1);
		drawMap = map;
		attackRange = map;
		cpuDisRange = map;
		drawEvaRange = map;
		for (int row = 2; row < map.height() + 2; ++row)
		{
			for (int col = 0; col < map.width(); ++col)
			{
				map[row-2][col] = Parse<int32>(mapCSV[row][col]);
			}
		}
		CSV csv{ U"CSVFile/ChipData.csv" };
		for (int i = 1; i < csv.rows(); ++i)
		{
			Chip c{ csv[i][1],Parse<HSV>(csv[i][2]),Parse<int32>(csv[i][3]) };
			chipdata.emplace(Parse<int32>(csv[i][0]), c);
		}
		CSV fortCSV{ U"CSVFile/fortCSV.csv" };
		for (int i = 1; i < fortCSV.rows(); ++i)
		{
			Fort* f = new Fort();
			f->team = (Teams)Parse<int>(fortCSV[i][0]);
			f->color = teamList[f->team].color;
			f->pos = Parse<Point>(fortCSV[i][1]);
			fortList.emplace_back(f);
		}

		CSV cityCSV{ U"CSVFile/cityCSV.csv" };
		for (int i = 1; i < cityCSV.rows(); ++i)
		{
			City* c = new City();
			c->name = cityCSV[i][0];
			c->team = (Teams)Parse<int>(cityCSV[i][1]);
			c->color = teamList[c->team].color;
			c->plusMoney = Parse<int>(cityCSV[i][2]);
			c->pos = Parse<Point>(cityCSV[i][3]);
			cityList.emplace_back(c);
		}
		
		for (auto&& f : fortList)
		{
			if (f->team == Teams::Blue)
			{
				InitDisRange();
				CPUCheck(0, f->pos);
			}
		}
	}
	void Update()
	{
		//music.play();
		if (scene == game)
		{

			GameUpdate();
			DrawMap();
			DrawCity();
			DrawFort();
			DrawFactory();
			DrawMove();
			DrawAttackRange();
			DrawUnit();
			DrawTurn();
			DrawMoney();
			DrawCityUI();
			DrawDebug();
			OnGUI();
			console.Update();
			if (targetFact != nullptr)
			{
				DrawFactoryUI();
			}
		}
		else if (scene == end)
		{
			DrawMap();
			DrawFort();
			DrawCity();
			DrawMove();
			DrawUnit();
			DrawEndScene();
			DrawEndGUI();
		}

	}
	~Game()
	{
		for (auto&& c : cityList)
		{
			delete c;
		}

		for (auto&& f : factoryList)
		{
			delete f;
		}

		for (auto&& u : unitList)
		{
			delete u;
		}
	}
};
class Edit
{
	enum Mode
	{
		MapMode,
		UnitMode,
		BuildMode
	};
	Mode mode;
	Grid<int32>map;
	HashTable<Teams,Team> teamList;
	Array<Unit*>unitList;
	Array<Fort>fortList;
	Array<City>cityList;
	Teams nowTeam;
	CSV mapCSV{ U"CSVFile/mapCSV.csv" };
	Font font{ 40,Typeface::Bold };
	Font moneyFont{ 20,Typeface::Black };
	Font nameFont{ 25,Typeface::Black };
	Font factoryFont{ 17,Typeface::Bold };
	double scale = 1;
	double size = 50;
	int chipMax = 0;
	Fort* currentFort = nullptr;
	City* currentCity = nullptr;
	double chipScrollX = 0;
	double scrollXMax = 0;
	Vec2 mapSize;
	Vec2 camPos{ 0,0 };
	double width = size * scale;
	bool isSet = false;
	bool isUnitMode = false;
	bool isEraseMode = false;
	int selectChip = 0;
	TextEditState inputText;
	HashTable<int32, Chip> chipdata;
	Array<bool> setCities;
	//HashTable<City,bool> setCities;
	HashTable<UnitType, UnitData> unitDB;
	void EditUpdate()
	{
		width = size * scale;
		Rect rightR{ 550,0,250,600 };
		Rect underR{ 0,500,550,100 };
		Rect unitModeR{ rightR.pos.x - 240,0,240,50 };
		Array<Rect> judgeR = { rightR,underR,unitModeR };
		bool isMouseHit = IsHitRects(judgeR);
		if (Mouse::Wheel() != 0)
		{
			if (!rightR.mouseOver())
			{
				scale -= Mouse::Wheel() / 15;
			}
			else
			{
				chipScrollX += Mouse::Wheel() * 15;
				
				if (chipScrollX > scrollXMax)
				{
					chipScrollX = scrollXMax;
				}
				if (chipScrollX < 0)
				{
					chipScrollX = 0;
				}
			}
		}
		if (MouseM.pressed())
		{
			camPos += Cursor::Delta();
		}
		for (auto y : step(map.height()))
		{
			for (auto x : step(map.width()))
			{
				int chipNo = map[y][x];
				Polygon h;
				if (y % 2 == 0)
				{
					h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y)).asPolygon();
				}
				else
				{
					h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y)).asPolygon();
				}
				if (h.leftPressed() && !isMouseHit)
				{
					map[y][x] = selectChip;
				}
			}
		}
		for (int i = 0; i < chipdata.size(); ++i)
		{
			Shape2D h = Shape2D::Hexagon(size, Vec2(rightR.center().x - size, size * 1.5 + 125 * i - chipScrollX));
			if (h.asPolygon().leftClicked())
			{
				if (selectChip == i)
				{
					isSet = true;
				}
				else
				{
					selectChip = i;
				}
			}
		}
	}
	void DrawChipUI()
	{
		Rect r{ 550,0,250,600 };r.draw(ColorF(0.9, 0.9, 0.9, 0.7));
		for (int i = 0; i < chipdata.size(); ++i)
		{
			Shape2D h = Shape2D::Hexagon(size,Vec2(r.center().x - size,size * 1.5 + 125 * i - chipScrollX)).draw(chipdata[i].color).drawFrame(1, Palette::Black);
			font(chipdata[i].name).draw(Vec2(h.asPolygon().centroid().x + size, h.asPolygon().centroid().y - font.fontSize() / 2),Palette::Black);
			if (i == selectChip)
			{
				h.drawFrame(3, Palette::Red);
			}
			
		}
		if (SimpleGUI::Button(U"-", Vec2(r.centerX() + 80,r.pos.y),unspecified, chipMax > 1))
		{
			
				if (selectChip == chipMax - 1)
				{
					selectChip--;
				}
				chipMax--;
				for (auto y : step(map.height()))
				{
					for (auto x : step(map.width()))
					{
						if (map[y][x] == chipMax)
						{
							map[y][x]--;
						}
					}
				}

				chipdata.erase(chipMax);
				scrollXMax = 75 + (chipdata.size() - 1) * 125;
				scrollXMax -= 525;
				if (scrollXMax < 0)
				{
					scrollXMax = 0;
				}
		}
		if (SimpleGUI::Button(U"+", Vec2(r.centerX() + 80, r.br().y - 40)))
		{
			Chip c{U"空白",Palette::White,0};
			chipdata.emplace(chipMax,c);
			chipMax++;
			scrollXMax = 75 + (chipdata.size() - 1) * 125;
			scrollXMax -= 525;
			if (scrollXMax < 0)
			{
				scrollXMax = 0;
			}
		}
	}
	void DrawGUI()
	{
		Rect r{ 0,500,550,100}; r.draw(ColorF(0.9, 0.9, 0.9, 0.7));
		Rect xr{ 320,530,100,60 }; xr.draw(ColorF(1, 1, 1)).drawFrame(3,Palette::Black);
		Rect yr{430,530,100,60 }; yr.draw(ColorF(1, 1, 1)).drawFrame(3,Palette::Black);
		Rect rightR{ 550,0,250,600 };
		nameFont(U"X").draw(Vec2(xr.centerX() - nameFont.fontSize() / 2, xr.pos.y - 6), Palette::Black);
		nameFont(U"Y").draw(Vec2(yr.centerX() - nameFont.fontSize() / 2, yr.pos.y - 6), Palette::Black);
		if (SimpleGUI::Button(U"+", Vec2(xr.centerX() - 2, 550)))
		{
			mapSize.x++;
			Grid<int32> newMap (mapSize.x, mapSize.y,0);
			for (auto y : step(map.height()))
			{
				for (auto x : step(map.width()))
				{
					 newMap[y][x] = map[y][x];
				}
			}
			map = newMap;
		}
		if (SimpleGUI::Button(U"-", Vec2(xr.centerX() - 50, 550)))
		{
			if (mapSize.x > 1)
			{
				mapSize.x--;
				Grid<int32> newMap(mapSize.x, mapSize.y, 0);
				for (auto y : step(map.height()))
				{
					for (int x = 0;x < mapSize.x;x++)
					{
						newMap[y][x] = map[y][x];
					}
				}
				map = newMap;
			}
			else
			{

			}
		}
		if (SimpleGUI::Button(U"+", Vec2(yr.centerX() - 2, 550)))
		{
			mapSize.y++;
			Grid<int32> newMap(mapSize.x, mapSize.y, 0);
			for (auto y : step(map.height()))
			{
				for (auto x : step(map.width()))
				{
					newMap[y][x] = map[y][x];
				}
			}
			map = newMap;
		}
		if (SimpleGUI::Button(U"-", Vec2(yr.centerX() - 50, 550)))
		{
			if (mapSize.y > 1)
			{
				mapSize.y--;
				Grid<int32> newMap(mapSize.x, mapSize.y, 0);
				for (int y = 0; y < mapSize.y; y++)
				{
					for (auto x : step(map.width()))
					{
						newMap[y][x] = map[y][x];
					}
				}
				map = newMap;
			}
			else
			{

			}
		}
		if (SimpleGUI::ButtonAt(U"タイトルへ",Vec2(75, r.centerY())))
		{
			state = title;
		}
		if (SimpleGUI::ButtonAt(U"セーブ", Vec2(250, r.centerY())))
		{
			CSV c;
			c.write(Format(mapSize.x));
			c.newLine();
			c.write(Format(mapSize.y));
			c.newLine();
			for (auto y : step(map.height()))
			{
				for (auto x : step(map.width()))
				{
					 c.write(Format(map[y][x]));
				}
				c.newLine();
			}
			if(FileSystem::IsFile(U"CSVFile/mapCSV.csv"))
			{
				FileSystem::Remove(U"CSVFile/mapCSV.csv",AllowUndo::Yes);
			}
			c.save(U"CSVFile/mapCSV.csv");
			CSV preChipCSV{ U"CSVFile/ChipData.csv" };
			CSV chipCSV;
			chipCSV.writeRow(preChipCSV[0][0], preChipCSV[0][1],preChipCSV[0][2],preChipCSV[0][3]);
			for (int i = 0; i < chipdata.size();++i)
			{
				chipCSV.writeRow(i, chipdata[i].name, Format(chipdata[i].color), chipdata[i].cost);
				/*chipCSV[i + 1][0] = Format(i);
				chipCSV[i + 1][1] = Format(chipdata[i].name);
				chipCSV[i + 1][2] = Format(chipdata[i].color);
				chipCSV[i + 1][3] = Format(chipdata[i].cost);*/
			}
			if (FileSystem::IsFile(U"CSVFile/ChipData.csv"))
			{
				FileSystem::Remove(U"CSVFile/ChipData.csv", AllowUndo::Yes);
			}
			chipCSV.save(U"CSVFile/ChipData.csv");
			CSV unitSetCSV;
			unitSetCSV.writeRow(U"チーム", U"種類", U"位置");
			for (auto&& u : unitList)
			{
				unitSetCSV.writeRow(u->team == Teams::Red ? 0 : 1, (int)u->type, u->pos);
			}
			if (FileSystem::IsFile(U"CSVFile/UnitSetData.csv"))
			{
				FileSystem::Remove(U"CSVFile/UnitSetData.csv", AllowUndo::Yes);
			}
			unitSetCSV.save(U"CSVFile/UnitSetData.csv");
			CSV unitDataCSV;
			unitDataCSV.writeRow(U"名前", U"攻撃範囲", U"移動距離", U"有利タイプ");
			for (int i = 0; i < unitDB.size(); ++i)
			{
				unitDataCSV.writeRow(unitDB[(UnitType)i].name, unitDB[(UnitType)i].range, unitDB[(UnitType)i].speed, (int)unitDB[(UnitType)i].advUnit);
			}
			if (FileSystem::IsFile(U"CSVFile/UnitData.csv"))
			{
				FileSystem::Remove(U"CSVFile/UnitData.csv", AllowUndo::Yes);
			}
			unitDataCSV.save(U"CSVFile/UnitData.csv");
			CSV cityCSV;
			cityCSV.writeRow(U"名前", U"チーム", U"お金", U"位置", U"配置するかどうか");
			for (int i = 0; i < cityList.size(); ++i)
			{
				City c = cityList[i];
				cityCSV.writeRow(c.name,(int)c.team,c.plusMoney,c.pos,setCities[i]);
			}
			if (FileSystem::IsFile(U"CSVFile/cityCSV.csv"))
			{
				FileSystem::Remove(U"CSVFile/cityCSV.csv", AllowUndo::Yes);
			}
			cityCSV.save(U"CSVFile/cityCSV.csv");
			CSV fortCSV;
			fortCSV.writeRow(U"チーム", U"位置");
			for (int i = 0; i < fortList.size(); ++i)
			{
				fortCSV.writeRow((int)fortList[i].team, fortList[i].pos);
			}
			if (FileSystem::IsFile(U"CSVFile/fortCSV.csv"))
			{
				FileSystem::Remove(U"CSVFile/fortCSV.csv", AllowUndo::Yes);
			}
			fortCSV.save(U"CSVFile/fortCSV.csv");
		}
		if (SimpleGUI::Button(U"ユニットモードに変更",Vec2(rightR.pos.x - 240,0)))
		{
			mode = UnitMode;
			chipScrollX = 0;
			selectChip = 0;
			scrollXMax = 75 - 600 + (int)unitDB.size() * 125;
		}
		
	}
	void DrawMap()
	{

		for (auto y : step(map.height()))
		{
			for (auto x : step(map.width()))
			{
				int chipNo = map[y][x];
				if (y % 2 == 0)
				{
					Shape2D h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y)).draw(chipdata[chipNo].color);
					h.drawFrame(1, Palette::Black);
				}
				else
				{
					Shape2D h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y)).draw(chipdata[chipNo].color);
					h.drawFrame(1, Palette::Black);
				}

				//RectF{ x * width + camPos.x,y * width + camPos.y,width }.draw(chipdata[chipNo].color);
			}
		}
	}
	void DrawFort()
	{
		for (auto&& fort : fortList)
		{
			Shape2D f;
			if (fort.pos.y % 2 == 0)
			{
				f = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * fort.pos.x + camPos.x, width * 1.5 * fort.pos.y + camPos.y));
			}
			else
			{
				f = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * fort.pos.x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * fort.pos.y + camPos.y));

			}
			if (mode == Mode::BuildMode)
			{
				f.draw(Palette::White); f.drawFrame(2, fort.color);
			}
			else
			{
				f.draw(ColorF(255,255,255,0.8)); f.drawFrame(2,ColorF(fort.color.rgb(),0.3));
			}
			nameFont(U"要塞").draw(Arg::center(f.asPolygon().centroid()),Palette::Black);
		}
	}
	void SettingGUI()
	{
		Rect r = { 0,0,800,600 }; r.draw(ColorF(Palette::Black,0.6));
		Rect guiR = { 100,100,600,400 }; guiR.draw(Palette::Lightgray);
		if (SimpleGUI::Button(U"×", Vec2(guiR.centerX() + 240, guiR.pos.y)))
		{
			isSet = false;
			inputText.clear();
		}
		SimpleGUI::Headline(U"Name", guiR.pos);
		if (SimpleGUI::TextBox(inputText, Vec2(guiR.pos.x, guiR.pos.y + 40)))
		{
			chipdata[selectChip].name = inputText.text;
		}
		RectF costR = { guiR.leftCenter().x,guiR.center().y - 90,200,120 }; costR.draw();
		SimpleGUI::Headline(U"Cost",Vec2(costR.pos.x,costR.pos.y - 35));
		font(chipdata[selectChip].cost).drawAt(Vec2(costR.center().x, costR.center().y),Palette::Black);
		if (SimpleGUI::Button(U"-", Vec2(costR.leftCenter().x, costR.center().y + 25)) && chipdata[selectChip].cost > 0)
		{
			chipdata[selectChip].cost--;
		}
		if (SimpleGUI::Button(U"+", Vec2(costR.center().x + 45,costR.center().y + 25)) && chipdata[selectChip].cost < 99)
		{
			chipdata[selectChip].cost++;
		}
		SimpleGUI::Headline(U"Color", Vec2(guiR.pos.x, costR.bottomY()));
		SimpleGUI::ColorPicker(chipdata[selectChip].color, Vec2(guiR.pos.x, costR.bottomY() + 35),true);
	}
	bool IsHitRects(Array<Rect> _rects)
	{
		bool isHit = false;
		for (int i = 0; i < _rects.size(); ++i)
		{
			if (_rects[i].mouseOver())
			{
				isHit = true;
				break;
			}
		}
		return isHit;
	}
	void UnitEditUpdate()
	{
		width = size * scale;
		Array<Rect> judgeRect;
		Rect rightR{ 550,0,250,600 };
		Rect underR{ 0,500,550,100 };
		Rect buttonR{0,0,font.fontSize() * teamList[nowTeam].name.size() +60,50 };
		Rect mapModeR{ rightR.pos.x - 220,0,220,50 };
		judgeRect.emplace_back(rightR);
		judgeRect.emplace_back(underR);
		judgeRect.emplace_back(buttonR);
		judgeRect.emplace_back(mapModeR);
		bool isMouseHit = IsHitRects(judgeRect);
		if (Mouse::Wheel() != 0)
		{
			if (!rightR.mouseOver())
			{
				scale -= Mouse::Wheel() / 15;
			}
			else
			{
				chipScrollX += Mouse::Wheel() * 15;

				if (chipScrollX > scrollXMax)
				{
					chipScrollX = scrollXMax;
				}
				if (chipScrollX < 0)
				{
					chipScrollX = 0;
				}
			}
		}
		if (MouseM.pressed())
		{
			camPos += Cursor::Delta();
		}
		for (int i = 0; i < unitDB.size(); ++i)
		{
			Shape2D h = Shape2D::Hexagon(size, Vec2(rightR.center().x - size, size * 1.5 + 125 * i - chipScrollX));
			if (h.asPolygon().leftClicked())
			{
				if (selectChip == i)
				{
					if (!isEraseMode)
					{
						isSet = true;
					}
				}
				else
				{
					selectChip = i;
				}
				isEraseMode = false;
			}
		}
		
		for (auto y : step(map.height()))
		{
			for (auto x : step(map.width()))
			{
				Polygon h;
				if (y % 2 == 0)
				{
					h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y)).asPolygon();
				}
				else
				{
					h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y)).asPolygon();
				}
				if (h.leftClicked() && !isMouseHit)
				{
					if (!isEraseMode)
					{
						Unit* dUni = nullptr;
						for (auto&& u : unitList)
						{
						if (u->pos == Vec2(x, y))
							{
							unitList.remove(u);
							break;
							}
						}
						if (dUni != nullptr)
						{
							delete dUni;
						}
						Unit* uni = new Unit();
						uni ->type = (UnitType)selectChip;
						uni->pos = Point(x, y);
						uni->color = teamList[nowTeam].color;
						unitList.emplace_back(uni);
					}
					else
					{
						Unit* dUni = nullptr;
						for (auto&& u : unitList)
						{
							if (u->pos == Vec2(x, y))
							{
								unitList.remove(u);
								break;
							}
						}
						if (dUni != nullptr)
						{
							delete dUni;
						}
					}
				}
			}
		}
	}
	void UnitChipUI()
	{
		Rect r{ 550,0,250,600 }; r.draw(ColorF(0.9, 0.9, 0.9, 0.7));
		for (int i = 0; i < unitDB.size(); ++i)
		{
			
			UnitType ut = (UnitType)i;
			Shape2D h = Shape2D::Hexagon(size, Vec2(r.center().x - size, size * 1.5 + 125 * i - chipScrollX)).draw().drawFrame(1,Palette::Black);
			font(unitDB[ut].name).draw(Vec2(h.asPolygon().centroid().x + size, h.asPolygon().centroid().y - font.fontSize() / 2), Palette::Black);
			if (i == selectChip && !isEraseMode)
			{
				h.drawFrame(3, Palette::Red);
			}
		}
	}
	void DrawUnit()
	{
		if (mode == Mode::UnitMode)
		{
			for (auto&& u : unitList)
			{
				Shape2D h;
				if (u->pos.y % 2 == 0)
				{
					h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * u->pos.x + camPos.x, width * 1.5 * u->pos.y + camPos.y)).draw(u->color);
				}
				else
				{
					h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * u->pos.x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * u->pos.y + camPos.y)).draw(u->color);
				}
				h.drawFrame(2, ColorF(u->color.rgba() + Vec4(0.6, 0.6, 0.6, 0)));
				if (scale >= 0.6)
				{
					nameFont(unitDB[u->type].name).draw(Arg::center(h.asPolygon().centroid()));
				}
			}
		}
		else
		{
			for (auto&& u : unitList)
			{
				Shape2D h;
				if (u->pos.y % 2 == 0)
				{
					h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * u->pos.x + camPos.x, width * 1.5 * u->pos.y + camPos.y)).draw(ColorF(u->color.rgb(), 0.3));
				}
				else
				{
					h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * u->pos.x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * u->pos.y + camPos.y)).draw(ColorF(u->color.rgb(),0.3));
				}
				h.drawFrame(2, ColorF(u->color.rgb() + Vec3(0.6, 0.6, 0.6),0.3));
				if (scale >= 0.6)
				{
					nameFont(unitDB[u->type].name).draw(Arg::center(h.asPolygon().centroid()));
				}
			}
		}
		

	}
	void DrawUnitUI()
	{
		RectF r{ -10,-10,200,80 }; r.rounded(20).draw(ColorF(0.0, 0.0, 0.0, 0.8));
		font(teamList[nowTeam].name).draw(Vec2(0,0), teamList[nowTeam].color);
	}
	void UnitGUI()
	{
		Rect underR{ 0,500,550,100 }; underR.draw(ColorF(0.9, 0.9, 0.9, 0.7));
		Rect rightR{ 550,0,250,600 };
		
		if (SimpleGUI::Button(U">", Vec2(font.fontSize() * teamList[nowTeam].name.size(),10),60))
		{
			nowTeam = (Teams)((int)nowTeam + 1);
			if (teamList.size() <= (int)nowTeam)
			{
				nowTeam = (Teams)0;
			};
		}
		if (SimpleGUI::ButtonAt(U"タイトルへ", Vec2(75,underR.centerY())))
		{
			state = title;
		}
		if (SimpleGUI::ButtonAt(U"セーブ", Vec2(250, underR.centerY())))
		{
			CSV c;
			c.write(Format(mapSize.x));
			c.newLine();
			c.write(Format(mapSize.y));
			c.newLine();
			for (auto y : step(map.height()))
			{
				for (auto x : step(map.width()))
				{
					c.write(Format(map[y][x]));
				}
				c.newLine();
			}
			if (FileSystem::IsFile(U"CSVFile/mapCSV.csv"))
			{
				FileSystem::Remove(U"CSVFile/mapCSV.csv", AllowUndo::Yes);
			}
			c.save(U"CSVFile/mapCSV.csv");
			CSV chipCSV;
			chipCSV.writeRow(U"種類",U"名前",U"色",U"コスト");
			for (int i = 0; i < chipdata.size(); ++i)
			{
				chipCSV.writeRow(i, chipdata[i].name, Format(chipdata[i].color), chipdata[i].cost);
				/*chipCSV[i + 1][0] = Format(i);
				chipCSV[i + 1][1] = Format(chipdata[i].name);
				chipCSV[i + 1][2] = Format(chipdata[i].color);
				chipCSV[i + 1][3] = Format(chipdata[i].cost);*/
			}
			if (FileSystem::IsFile(U"CSVFile/ChipData.csv"))
			{
				FileSystem::Remove(U"CSVFile/ChipData.csv", AllowUndo::Yes);
			}
			chipCSV.save(U"CSVFile/ChipData.csv");
			CSV unitSetCSV;
			unitSetCSV.writeRow(U"チーム",U"種類",U"位置");
			for (auto&& u : unitList)
			{
				unitSetCSV.writeRow(u->color == teamList[Teams::Red].color ? 0 : 1,(int)u->type,u->pos);
			}
			if (FileSystem::IsFile(U"CSVFile/UnitSetData.csv"))
			{
				FileSystem::Remove(U"CSVFile/UnitSetData.csv", AllowUndo::Yes);
			}
			unitSetCSV.save(U"CSVFile/UnitSetData.csv");
			CSV unitDataCSV;
			unitDataCSV.writeRow(U"名前",U"攻撃範囲",U"移動距離",U"有利タイプ");
			for (int i = 0; i < unitDB.size(); ++i)
			{
				unitDataCSV.writeRow(unitDB[(UnitType)i].name, unitDB[(UnitType)i].range, unitDB[(UnitType)i].speed,(int)unitDB[(UnitType)i].advUnit);
			}
			if (FileSystem::IsFile(U"CSVFile/UnitData.csv"))
			{
				FileSystem::Remove(U"CSVFile/UnitData.csv", AllowUndo::Yes);
			}
			unitDataCSV.save(U"CSVFile/UnitData.csv");
			CSV cityCSV;
			cityCSV.writeRow(U"名前", U"チーム", U"お金", U"位置", U"配置するかどうか");
			for (int i = 0; i < cityList.size(); ++i)
			{
				City c = cityList[i];
				cityCSV.writeRow(c.name, (int)c.team, c.plusMoney, c.pos, setCities[i]);
			}
			if (FileSystem::IsFile(U"CSVFile/cityCSV.csv"))
			{
				FileSystem::Remove(U"CSVFile/cityCSV.csv", AllowUndo::Yes);
			}
			cityCSV.save(U"CSVFile/cityCSV.csv");
			CSV fortCSV;
			fortCSV.writeRow(U"チーム", U"位置");
			for (int i = 0; i < fortList.size(); ++i)
			{
				fortCSV.writeRow((int)fortList[i].team, fortList[i].pos);
			}
			if (FileSystem::IsFile(U"CSVFile/fortCSV.csv"))
			{
				FileSystem::Remove(U"CSVFile/fortCSV.csv", AllowUndo::Yes);
			}
			fortCSV.save(U"CSVFile/fortCSV.csv");
		}
		if (SimpleGUI::Button(U"ビルディングモードに変更", Vec2(rightR.pos.x - 280, 0)))
		{
			isSet = false;
			mode = BuildMode;
			chipScrollX = 0;
			selectChip = 0;
			scrollXMax = 75 - 600 + (int)cityList.size() * 125;
		}
		String str = isEraseMode == true ? U"OFF" : U"ON";
		if (SimpleGUI::ButtonAt(U"消去モード:" + str, Vec2(underR.centerX() + 120, underR.centerY()), unspecified))
		{
			isEraseMode = true;
		}
	}
	void UnitSettingGUI()
	{
		Rect r = { 0,0,800,600 }; r.draw(ColorF(Palette::Black, 0.6));
		Rect guiR = { 100,100,600,400 }; guiR.draw(Palette::Lightgray);
		UnitData* ud = &unitDB[(UnitType)selectChip];
		if (SimpleGUI::Button(U"×", Vec2(guiR.centerX() + 240, guiR.pos.y)))
		{
			isSet = false;
			inputText.clear();
		}
		SimpleGUI::Headline(U"Name", guiR.pos);
		if (SimpleGUI::TextBox(inputText, Vec2(guiR.pos.x, guiR.pos.y + 40)))
		{
			ud->name = inputText.text;
		}
		RectF rangeR = { guiR.leftCenter().x,guiR.center().y - 90,200,120 }; rangeR.draw();
		SimpleGUI::Headline(U"Range", Vec2(rangeR.pos.x, rangeR.pos.y - 35));
		font(unitDB[(UnitType)selectChip].range).drawAt(Vec2(rangeR.center().x, rangeR.center().y), Palette::Black);
		if (SimpleGUI::Button(U"-", Vec2(rangeR.leftCenter().x, rangeR.center().y + 25)) && ud->range > 0)
		{
			ud->range--;
		}
		if (SimpleGUI::Button(U"+", Vec2(rangeR.center().x + 45, rangeR.center().y + 25)) && ud->range < 99)
		{
			ud->range++;
		}
		SimpleGUI::Headline(U"Speed", Vec2(rangeR.pos.x, rangeR.bottomY() + 10));
		RectF speedR = { rangeR.pos.x,rangeR.bottomY() + 45,200,120}; speedR.draw();
		font(unitDB[(UnitType)selectChip].speed).drawAt(Vec2(speedR.center().x, speedR.center().y), Palette::Black);
		if (SimpleGUI::Button(U"-", Vec2(speedR.leftCenter().x, speedR.center().y + 25)) && ud->speed > 0)
		{
			ud->speed--;
		}
		if (SimpleGUI::Button(U"+", Vec2(speedR.center().x + 45, speedR.center().y + 25)) && ud->speed < 99)
		{
			ud->speed++;
		}
		RectF typeR = {	rangeR.rightCenter().x,guiR.center().y - 90,200,120}; typeR.draw();
		SimpleGUI::Headline(U"Adv", Vec2(typeR.pos.x, typeR.pos.y - 35));
		font(unitDB[ud->advUnit].name).drawAt(Vec2(typeR.center().x, typeR.center().y), Palette::Black);
		if (SimpleGUI::Button(U"-", Vec2(typeR.leftCenter().x, typeR.center().y + 25)) && (int)ud->advUnit > 0)
		{
			ud->advUnit = (UnitType)((int)unitDB[(UnitType)selectChip].advUnit - 1);
		}
		if (SimpleGUI::Button(U"+", Vec2(typeR.center().x + 45, typeR.center().y + 25)) && (int)ud->advUnit < (int)UnitType::Max - 1)
		{
			ud->advUnit = (UnitType)((int)unitDB[(UnitType)selectChip].advUnit + 1);
		}
	}
	void BuildUpdate()
	{
		width = size * scale;
		
		Rect rightR{ 550,0,250,600 };
		Rect underR{ 0,500,550,100 };
		Rect mapModeR{ rightR.pos.x - 220,0,220,50 };
		Array<Rect> judgeRect{rightR,underR,mapModeR};
		bool isHit = IsHitRects(judgeRect);
		
		if (Mouse::Wheel() != 0)
		{
			if (!rightR.mouseOver())
			{
				scale -= Mouse::Wheel() / 15;
			}
			else
			{
				chipScrollX += Mouse::Wheel() * 15;

				if (chipScrollX > scrollXMax)
				{
					chipScrollX = scrollXMax;
				}
				if (chipScrollX < 0)
				{
					chipScrollX = 0;
				}
			}
		}
		if (MouseM.pressed())
		{
			camPos += Cursor::Delta();
		}
		for (int i = 0; i < cityList.size(); ++i)
		{
			Shape2D h = Shape2D::Hexagon(size, Vec2(rightR.center().x - size, size * 1.5 + 125 * i - chipScrollX));
			if (h.asPolygon().leftClicked() )
			{
				if (selectChip == i)
				{
					isSet = true;
				}
				else
				{
					selectChip = i;
				}
			}
		}
		if (!isHit && MouseL.down())
		{
			for (auto y : step(map.height()))
			{

				for (auto x : step(map.width()))
				{
					Polygon h;
					if (y % 2 == 0)
					{
						h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y)).asPolygon();
					}
					else
					{
						h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y)).asPolygon();
					}
					if (h.mouseOver())
					{
						bool isBuilding = false;
						for (auto&& f : fortList)
						{
							if (f.pos == Point(x, y))
							{
								currentFort = &f;
								isBuilding = true;
								break;
							}
						}

						if (isBuilding || cityList.size() == 0)continue;
						City c = cityList[selectChip];
						if (isEraseMode)
						{
							for (int i = 0;i < cityList.size();i++)
							{
								if (cityList[i].pos == Point(x, y))
								{
									setCities[i] = false;
								}
							}
						}
						else
						{
							isBuilding = false;
							for (auto&& c : cityList)
							{
								if (c.pos == Point(x, y))
								{
									currentCity = &c;
									isBuilding = true;
									break;
								}
							}
							if (isBuilding) continue;
							if (!setCities[selectChip])
							{
								cityList[selectChip].pos = Point(x, y);
								setCities[selectChip] = true;
							}
							
						}
					}
				}
			}
		}
		if (currentCity != nullptr)
		{
			if (MouseL.up())
			{
				for (auto y : step(map.height()))
				{
					for (auto x : step(map.width()))
					{
						Polygon h;
						if (y % 2 == 0)
						{
							h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y)).asPolygon();
						}
						else
						{
							h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y)).asPolygon();
						}
						if (h.mouseOver())
						{
							currentCity->pos = Point(x,y);
						}
					}
				}
				currentCity = nullptr;
			}
		}
		if (currentFort != nullptr)
		{
			if (MouseL.up())
			{
				for (auto y : step(map.height()))
				{
					for (auto x : step(map.width()))
					{
						Polygon h;
						if (y % 2 == 0)
						{
							h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + camPos.x, width * 1.5 * y + camPos.y)).asPolygon();
						}
						else
						{
							h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * y + camPos.y)).asPolygon();
						}
						if (h.mouseOver())
						{
							currentFort->pos = Point(x, y);
							break;
						}
					}
				}
				currentFort = nullptr;

			}
		}
	}
	void BuildGUI()
	{
		Rect underR{ 0,500,550,100 }; underR.draw(ColorF(0.9, 0.9, 0.9, 0.7));
		Rect rightR{ 550,0,250,600 };
		if (SimpleGUI::Button(U"マップモードに変更", Vec2(rightR.pos.x - 220, 0)))
		{
			mode = MapMode;
			isSet = false;
			isEraseMode = false;
			chipScrollX = 0;
			scrollXMax = 75 - 600 + (int)chipdata.size() * 125;
			if (scrollXMax <= 0)
			{
				scrollXMax = 0;
			}
			selectChip = 0;
		}
		String str = isEraseMode == true ? U"OFF" : U"ON";
		if (SimpleGUI::ButtonAt(U"消去モード:" + str, Vec2(underR.centerX() + 120, underR.centerY()), unspecified))
		{
			isEraseMode = !isEraseMode;
		}
		if (SimpleGUI::ButtonAt(U"タイトルへ", Vec2(75, underR.centerY())))
		{
			state = title;
		}
		if (SimpleGUI::ButtonAt(U"セーブ", Vec2(250, underR.centerY())))
		{
			CSV c;
			c.write(Format(mapSize.x));
			c.newLine();
			c.write(Format(mapSize.y));
			c.newLine();
			for (auto y : step(map.height()))
			{
				for (auto x : step(map.width()))
				{
					c.write(Format(map[y][x]));
				}
				c.newLine();
			}
			if (FileSystem::IsFile(U"CSVFile/mapCSV.csv"))
			{
				FileSystem::Remove(U"CSVFile/mapCSV.csv", AllowUndo::Yes);
			}
			c.save(U"CSVFile/mapCSV.csv");
			CSV chipCSV;
			chipCSV.writeRow(U"種類", U"名前", U"色", U"コスト");
			for (int i = 0; i < chipdata.size(); ++i)
			{
				chipCSV.writeRow(i, chipdata[i].name, Format(chipdata[i].color), chipdata[i].cost);
				/*chipCSV[i + 1][0] = Format(i);
				chipCSV[i + 1][1] = Format(chipdata[i].name);
				chipCSV[i + 1][2] = Format(chipdata[i].color);
				chipCSV[i + 1][3] = Format(chipdata[i].cost);*/
			}
			if (FileSystem::IsFile(U"CSVFile/ChipData.csv"))
			{
				FileSystem::Remove(U"CSVFile/ChipData.csv", AllowUndo::Yes);
			}
			chipCSV.save(U"CSVFile/ChipData.csv");
			CSV unitSetCSV;
			unitSetCSV.writeRow(U"チーム", U"種類", U"位置");
			for (auto&& u : unitList)
			{
				unitSetCSV.writeRow(u->color == teamList[Teams::Red].color ? 0 : 1, (int)u->type, u->pos);
			}
			if (FileSystem::IsFile(U"CSVFile/UnitSetData.csv"))
			{
				FileSystem::Remove(U"CSVFile/UnitSetData.csv", AllowUndo::Yes);
			}
			unitSetCSV.save(U"CSVFile/UnitSetData.csv");
			CSV unitDataCSV;
			unitDataCSV.writeRow(U"名前", U"攻撃範囲", U"移動距離", U"有利タイプ");
			for (int i = 0; i < unitDB.size(); ++i)
			{
				unitDataCSV.writeRow(unitDB[(UnitType)i].name, unitDB[(UnitType)i].range, unitDB[(UnitType)i].speed, (int)unitDB[(UnitType)i].advUnit);
			}
			if (FileSystem::IsFile(U"CSVFile/UnitData.csv"))
			{
				FileSystem::Remove(U"CSVFile/UnitData.csv", AllowUndo::Yes);
			}
			unitDataCSV.save(U"CSVFile/UnitData.csv");
			CSV cityCSV;
			cityCSV.writeRow(U"名前",U"チーム",U"お金", U"位置",U"配置するかどうか");
			for (int i = 0; i < cityList.size(); ++i)
			{
				City c = cityList[i];
				cityCSV.writeRow(c.name,(int)c.team,c.plusMoney,c.pos,setCities[i]);
			}
			if (FileSystem::IsFile(U"CSVFile/cityCSV.csv"))
			{
				FileSystem::Remove(U"CSVFile/cityCSV.csv", AllowUndo::Yes);
			}
			cityCSV.save(U"CSVFile/cityCSV.csv");
			CSV fortCSV;
			fortCSV.writeRow(U"チーム",U"位置");
			for (int i = 0; i < fortList.size(); ++i)
			{
				fortCSV.writeRow((int)fortList[i].team,fortList[i].pos);
			}
			if (FileSystem::IsFile(U"CSVFile/fortCSV.csv"))
			{
				FileSystem::Remove(U"CSVFile/fortCSV.csv", AllowUndo::Yes);
			}
			fortCSV.save(U"CSVFile/fortCSV.csv");
		}
	}
	void DrawBuildUI()
	{
		Rect r{ 550,0,250,600 }; r.draw(ColorF(0.9, 0.9, 0.9, 0.7));

		for (int i = 0; i < cityList.size(); ++i)
		{
			Shape2D h;
			Color frameColor;
			if (setCities[i])
			{
				h = Shape2D::Hexagon(size, Vec2(r.center().x - size, size * 1.5 + 125 * i - chipScrollX)).draw(ColorF(1,1,1,0.3)).drawFrame(1, Palette::Black);
				frameColor = Color(255, 0, 0,55);
			}
			else
			{
				h = Shape2D::Hexagon(size, Vec2(r.center().x - size, size * 1.5 + 125 * i - chipScrollX)).draw().drawFrame(1, Palette::Black);
				frameColor = Palette::Red;
			}
			
			font(cityList[i].name).draw(Vec2(h.asPolygon().centroid().x + size, h.asPolygon().centroid().y - font.fontSize() / 2), Palette::Black);

			if (i  == selectChip && !isEraseMode)
			{
				h.drawFrame(3, frameColor);
			}
		}
		if (SimpleGUI::Button(U"-", Vec2(r.centerX() + 80, r.pos.y), unspecified, cityList.size() > 0))
		{

			if (selectChip == chipMax - 1)
			{
				selectChip--;
			}
			chipMax--;

			cityList.pop_back();
			setCities.pop_back();
			scrollXMax = 75 + (cityList.size() - 1) * 125;
			scrollXMax -= 525;
			if (scrollXMax < 0)
			{
				scrollXMax = 0;
			}
		}
		if (SimpleGUI::Button(U"+", Vec2(r.centerX() + 80, r.br().y - 40)))
		{
			City c{ U"空白"};
			c.team = Teams::None;
			c.color = Palette::Black;
			setCities.emplace_back(false);
			cityList.emplace_back(c);
			scrollXMax = 75 + (cityList.size() - 1) * 125;
			scrollXMax -= 525;
			if (scrollXMax < 0)
			{
				scrollXMax = 0;
			}
		}
	}
	void DrawCity()
	{
		for (int i = 0;i < cityList.size();i++)
		{
			City c = cityList[i];
			Shape2D h;
			if (!setCities[i])continue;
			if (c.pos.y % 2 == 0)
			{
				h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * c.pos.x + camPos.x, width * 1.5 * c.pos.y + camPos.y)).draw();
			}
			else
			{
				h = Shape2D::Hexagon(width, Vec2(width * sqrt(3) * c.pos.x + width * sqrt(3) / 2 + camPos.x, width * 1.5 * c.pos.y + camPos.y)).draw();
			}
			h.drawFrame(2, ColorF(c.color.rgba()));
			if (scale >= 0.6)
			{
				nameFont(c.name).draw(Arg::center(h.asPolygon().centroid()),Palette::Black);
			}
		}
	}
	void BuildSetGUI()
	{
		Rect r = { 0,0,800,600 }; r.draw(ColorF(Palette::Black, 0.6));
		Rect guiR = { 100,100,600,400 }; guiR.draw(Palette::Lightgray);
		if (SimpleGUI::Button(U"×", Vec2(guiR.centerX() + 240, guiR.pos.y)))
		{
			isSet = false;
			inputText.clear();
		}
		SimpleGUI::Headline(U"Name", guiR.pos);
		if (SimpleGUI::TextBox(inputText, Vec2(guiR.pos.x, guiR.pos.y + 40)))
		{
			cityList[selectChip].name = inputText.text;
		}
		RectF moneyR = { guiR.leftCenter().x,guiR.center().y - 90,200,120 }; moneyR.draw();
		SimpleGUI::Headline(U"Money", Vec2(moneyR.pos.x, moneyR.pos.y - 35));
		font(cityList[selectChip].plusMoney).drawAt(Vec2(moneyR.center().x, moneyR.center().y), Palette::Black);
		if (SimpleGUI::Button(U"-", Vec2(moneyR.leftCenter().x, moneyR.center().y + 25)) && cityList[selectChip].plusMoney > 0)
		{
			cityList[selectChip].plusMoney -=100;
		}
		if (SimpleGUI::Button(U"+", Vec2(moneyR.center().x + 45, moneyR.center().y + 25)) && cityList[selectChip].plusMoney < 9999)
		{
			cityList[selectChip].plusMoney +=100;
			
		}
		RectF teamR = {guiR.leftCenter().x,moneyR.bottomY() + 45,200,120}; teamR.draw();
		SimpleGUI::Headline(U"Team", Vec2(teamR.pos.x, teamR.pos.y - 35));
		font(teamList[cityList[selectChip].team].name).drawAt(Vec2(teamR.center().x, teamR.center().y), Palette::Black);
		if (SimpleGUI::Button(U"-", Vec2(teamR.leftCenter().x, teamR.center().y + 25)))
		{
			int teamNum = (int)cityList[selectChip].team - 1;
			if (teamNum < 0)
			{
				cityList[selectChip].team = Teams::None;
			}
			else
			{
				cityList[selectChip].team = (Teams)teamNum;
			}
		}
		if (SimpleGUI::Button(U"+", Vec2(teamR.center().x + 45, teamR.center().y + 25)))
		{
			int teamNum = (int)cityList[selectChip].team + 1;
			if (teamNum > (int)Teams::None)
			{
				cityList[selectChip].team = (Teams)0;
			}
			else
			{
				cityList[selectChip].team = (Teams)teamNum;
			}
			cityList[selectChip].color = teamList[cityList[selectChip].team].color;
		}
	}
	void DrawHaveBuiding()
	{
		if (currentCity != nullptr)
		{
			Shape2D h = Shape2D::Hexagon(width, Cursor::PosF()); h.draw();
			h.drawFrame(2, ColorF(currentCity->color.rgba()));
			if (scale >= 0.6)
			{
				nameFont(currentCity->name).draw(Arg::center(h.asPolygon().centroid()), Palette::Black);
			}
		}
		if (currentFort != nullptr)
		{
			Shape2D h = Shape2D::Hexagon(width, Cursor::PosF()); h.draw();
			h.drawFrame(2, ColorF(currentFort->color.rgba()));
			if (scale >= 0.6)
			{
				nameFont(U"要塞").draw(Arg::center(h.asPolygon().centroid()), Palette::Black);
			}
		}
	}
public:
	Edit()
	{
		CSV unitCSV{ U"CSVFile/UnitData.csv" };
		for (int i = 1; i < unitCSV.rows(); ++i)
		{
			UnitData u;
			u.name = unitCSV[i][0];
			u.range = Parse<int>(unitCSV[i][1]);
			u.speed = Parse<int>(unitCSV[i][2]);
			u.advUnit = (UnitType)Parse<int>(unitCSV[i][3]);
			unitDB.emplace((UnitType)(i - 1),u);
		}
		Team red{ U"レッド",Palette::Red,100 };
		Team blue{ U"ブルー",Palette::Blue,100 };
		teamList.emplace(Teams::Red,red);
		teamList.emplace(Teams::Blue,blue);

		mapSize.x = Parse<int32>(mapCSV[0][0]);
		mapSize.y = Parse<int32>(mapCSV[1][0]);
		map = Grid<int32>(mapSize.x,mapSize.y,-1);
		for (int row = 2; row < map.height() + 2; ++row)
		{
			for (int col = 0; col < map.width(); ++col)
			{
				map[row - 2][col] = Parse<int32>(mapCSV[row][col]);
			}
		}
		CSV csv{ U"CSVFile/ChipData.csv"};
		for (int i = 1; i < csv.rows(); ++i)
		{
			Chip c{csv[i][1],Parse<HSV>(csv[i][2]),Parse<int32>(csv[i][3])};
			chipdata.emplace(Parse<int32>(csv[i][0]), c);
			//chipdata.emplace(0,c);
			chipMax++;
		}
		scrollXMax = 75 - 600 + (int)chipdata.size() * 125;
		if (scrollXMax <= 0)
		{
			scrollXMax = 0;
		}
		CSV fortCSV{ U"CSVFile/fortCSV.csv"};
		for (int i = 1; i < fortCSV.rows(); ++i)
		{
			Fort f;
			f.team = (Teams)Parse<int>(fortCSV[i][0]);
			f.color = teamList[f.team].color;
			f.pos = Parse<Point>(fortCSV[i][1]);
			fortList.emplace_back(f);
		}
		CSV cityCSV{ U"CSVFile/cityCSV.csv"};
		for (int i = 1; i < cityCSV.rows(); ++i)
		{
			City c;
			c.name = cityCSV[i][0];
			c.team = (Teams)Parse<int>(cityCSV[i][1]);
			c.color = teamList[c.team].color;
			c.plusMoney = Parse<int>(cityCSV[i][2]);
			c.pos = Parse<Point>(cityCSV[i][3]);
			setCities.emplace_back(Parse<bool>(cityCSV[i][4]));
			cityList.emplace_back(c);
			
		}
		CSV unitSetCSV{ U"CSVFile/UnitSetData.csv" };
		for (int y = 1; y < unitSetCSV.rows(); y++)
		{
			Teams t = (Teams)Parse<int>(unitSetCSV[y][0]);
			Unit* u = new Unit();
			u->type = (UnitType)Parse<int>(unitSetCSV[y][1]);
			u->pos = Parse<Point>(unitSetCSV[y][2]);
			u->team = t;
			u->color = teamList[t].color;
			unitList.emplace_back(u);
		}
		//CSV fortCSV{U"CSVFile/"};
	}
	void Update()
	{
		DrawMap();
		DrawFort();
		DrawUnit();
		DrawCity();
		switch (mode)
		{
		case MapMode:
			DrawChipUI();
			if (isSet)
			{
				SettingGUI();
			}
			else
			{
				EditUpdate();
				DrawGUI();
			}
			break;
		case UnitMode:
			UnitChipUI();
			DrawUnitUI();
			if (isSet)
			{
				UnitSettingGUI();
			}
			else
			{
				UnitEditUpdate();
				UnitGUI();
			}
			break;
		case BuildMode:
			DrawBuildUI();
			
			if (isSet)
			{
				BuildSetGUI();
			}
			else
			{
				BuildUpdate();
				BuildGUI();
				DrawHaveBuiding();
			}
			break;
		}
	}
};
class Title
{
	bool isGameMode = false;
	double posX = { 325 };
public:
	int DrawGUI(int _state,Game*& _gm,Edit*& _e)
	{
		if (!isGameMode)
		{
			if (SimpleGUI::Button(U"ゲーム", Vec2(posX, 350)))
			{
				isGameMode = true;
			}
			if (SimpleGUI::Button(U"エディット", Vec2(posX, 450)))
			{
				_state = edit;
				_e = new Edit();
			}
		}
		else
		{
			if (SimpleGUI::Button(U"CPU対戦", Vec2(posX, 300)))
			{
				_state = game;
				_gm = new Game(2);
				isGameMode = false;
			}
			if (SimpleGUI::Button(U"ローカル対戦", Vec2(posX, 370)))
			{
				_state = game;
				if (_gm != nullptr)
				{
					delete(_gm);
				}
				_gm = new Game(0);
				isGameMode = false;
			}
			if (SimpleGUI::Button(U"ネット対戦", Vec2(posX, 440)))
			{
				isGameMode = false;
			}
			if (SimpleGUI::Button(U"戻る", Vec2(posX, 510)))
			{
				isGameMode = false;
			}
		}
		
		return _state;
	}
	void Update(int& _state,Game*& _gm,Edit*& _e)
	{
		_state = DrawGUI(_state,_gm,_e);
	}
};
void Main()
{
	Game* gm = nullptr;
	Title* t = new Title();
	Edit* e = nullptr;

	TCPServer* server = nullptr;
	TCPClient* client = nullptr;
	while (System::Update())
	{
		switch (state)
		{
		case title:
			t->Update(state,gm,e);
			break;
		case game:
			gm->Update();
			break;
		case edit:
			e->Update();
			break;
			
		}
		
	}
	delete(gm);
	delete(t);
	delete(e);
	delete(server);
	delete(client);
}

//
// - Debug ビルド: プログラムの最適化を減らす代わりに、エラーやクラッシュ時に詳細な情報を得られます。
//
// - Release ビルド: 最大限の最適化でビルドします。
//
// - [デバッグ] メニュー → [デバッグの開始] でプログラムを実行すると、[出力] ウィンドウに詳細なログが表示され、エラーの原因を探せます。
//
// - Visual Studio を更新した直後は、プログラムのリビルド（[ビルド]メニュー → [ソリューションのリビルド]）が必要な場合があります。
//
