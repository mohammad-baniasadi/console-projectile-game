#include <iostream>
#include <windows.h>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>


    using namespace std;
    
    #define WIN32_LEAN_AND_MEAN
    #define _USE_MATH_DEFINES

#define Num_lev 10

//Data Structures
enum LevelState 
{
    RUNNING,
    WIN,
    LOSE,
};
enum class ShotType
{
    Normal,
    Bomb,
    Pull,
    Bounce,
    Drill,
    Split
};
struct PhysicsConfig
{
    double timeStep = 0.1;  // dt
    double gravity = 2;
    
    int minSpeed = 1;
    int maxSpeed = 25;

    int minAngle = 1;
    int maxAngle = 90;

    int frameDelayMs = 30;
};
struct Vec2
{
    double x;
    double y;
};
struct LaunchInput
{
    double speed;
    double angle;
};
struct VelocityInput
{
    double vx;
    double vy;
};
struct Cell
{
    int x;
    int y;
};
struct Player
{
    Cell pos;
    char symbol = 'P';
};
struct Enemy
{
    Cell pos;
    char symbol = 'E';
    bool alive = true;
};
struct Wall
{
    Cell pos;
    char symbol = '#';
    bool standing = true;
};
struct Level 
{
    Player player;
    vector<Enemy> enemies;
    vector<Wall> walls;
    
    int maxShots = 10;
    int shotsFired = 0;
    int parShots = 3;
    LevelState State = RUNNING ;
};
struct GameProgress
{
    int stars[Num_lev] = {0};
    int scores[Num_lev] = {0};
    bool unlocked[Num_lev] = {true};
};

const PhysicsConfig DefaultPhysics;
PhysicsConfig physics;
GameProgress progress;
vector<string> Grid;
int Row=17;
int Col=62;
const string LINE = "==============================================================";

class Projectile;
//Functions
void clearscreen();
void SetColor(WORD color);
void SaveProgressToFile();
void LoadProgressFromFile();
bool operator==(const Cell& a, const Cell& b);
Cell ToCell(Vec2 v);
void PlayGround ();
void Ground_print();
void placement_char(Cell Pos,char C);
void DrawLevel(Level &lvl);
void RunSingleShot(Level &lvl);
void CheckLevelEnd(Level &lvl);
void LoadLevelData(int levelNumber, Level& lvl);
void StartLevel(int levelNumber);
void RunLevel(Level &lvl, int levelIndex);
void ResetLevel(Level& lvl);
void InitProgress(GameProgress &progress);
int CalculateStars(Level& lvl);
Projectile* CreateProjectile(ShotType type, Vec2 pos, Vec2 vel);
int Get_Speed();
int Get_Angle();
bool IsOutOfBounds(Cell Pos);
void Destroy_Area(Level &lvl, Cell c, int radius);
bool Hit_Enemy(Cell Pos);
void Destroy_Enemy(Level &lvl, Cell Pos);
bool allEnemiesDead(Level &lvl);
bool Hit_Wall(Cell Pos);
void Demolition_Wall(Level &lvl, Cell Pos);
void Welcome_LOGO();
void Win_LOGO();
void Lose_LOGO();
void MainMenu();
void LevelMenu();
void HowToPlayMenu();
void ProgressStarsMenu();
void AboutDeveloperMenu();
void MapMenu();
void SettingsMenu();
void ShowMapOfLevel(int levelNumber);
void ChangeMapSize(string &status);
void ChangeGravity(string &status);
void ChangeTimeStep(string &status);
void Setup_Level_1(Level &lvl);
void Setup_Level_2(Level &lvl);
void Setup_Level_3(Level &lvl);
void Setup_Level_4(Level& lvl);
void Setup_Level_5(Level& lvl);
void Setup_Level_6(Level &lvl);
void Setup_Level_7(Level &lvl);
void Setup_Level_8(Level &lvl);
void Setup_Level_9(Level& lvl);
void Setup_Level_10(Level& lvl);
//OPP
ShotType GetShotType()
{
    int choice;

    cout << "Select shot type:\n";
    cout << "1. Normal\n";
    cout << "2. Bomb\n";
    cout << "3. Pull\n";
    cout << "4. Bounce\n";
    cout << "5. Drill\n";
    cout << "6. Split\n";
    cout << "Choice: ";

    cin >> choice;

    switch (choice)
    {
    case 1: return ShotType::Normal;
    case 2: return ShotType::Bomb;
    case 3: return ShotType::Pull;
    case 4: return ShotType::Bounce;
    case 5: return ShotType::Drill;
    case 6: return ShotType::Split;
    default: return ShotType::Normal;
    }
}
class ProjectileConfig
{
public:
    int explosionRadius = 0;
    bool destroyCenter = true;

    ProjectileConfig() = default;
};
class Projectile
{
public:
    
    virtual ~Projectile() = default;
    
    bool IsActive() const { return active; }

    virtual void Update(Level& lvl)
    {
        if (!active)
            return;

        double dt = physics.timeStep;

        oldPos = pos;

        vel.y += physics.gravity * dt;

        pos.x += vel.x * dt;
        pos.y += vel.y * dt;

        CheckCollision(oldPos, lvl);
    }

    virtual void Draw() const
    {
        if (!active)
            return;

        Cell Pos;
        Pos.x = (int)floor(pos.x);
        Pos.y = (int)floor(pos.y);

        placement_char(Pos, symbol);
    }

    static Vec2 VelocityFromAngle(double speed, double angle_deg)
    {
        double rad = angle_deg * M_PI / 180.0;
        return Vec2{ speed * cos(rad), -speed * sin(rad) };
    }

protected:
    
    Vec2 pos;
    Vec2 vel;
    Vec2 oldPos;

    bool active = true;
    char symbol = '*';

    Projectile() {}

    Projectile(Vec2 position, Vec2 velocity)
    {
        pos = position;
        vel = velocity;
    }

    virtual void OutOfBounds (Level& lvl, Cell cell) 
    {
        active = false;
    }

    virtual void HitEnemy (Level& lvl, Cell cell)
    {
        Destroy_Enemy(lvl, cell);
        active = false;
    }

    virtual void HitWall (Level& lvl, Cell cell) 
    {
        active = false;
    }

private:
    void CheckCollision(Vec2 oldPos, Level& lvl)
    {
        double dx = pos.x - oldPos.x;
        double dy = pos.y - oldPos.y;

        double distance = sqrt(dx * dx + dy * dy);
        double cellSize = 1.0;

        int steps = (int)ceil(distance / cellSize);
        if (steps < 1)
            steps = 1;

        for (int i = 1; i <= steps; i++)
        {
            double t = (double)i / steps;

            double x = oldPos.x + dx * t;
            double y = oldPos.y + dy * t;

            Cell g;
            g.x = (int)floor(x);
            g.y = (int)floor(y);

            if (IsOutOfBounds(g))
            {
                OutOfBounds(lvl, g);
                return;
            }

            if (Hit_Enemy(g))
            {
                HitEnemy (lvl, g);
                return;

            }

            if (Hit_Wall(g))
            {
                HitWall (lvl, g);
                return;
            }
        }
    }

};
class NormalShot : public Projectile
{
public:

    NormalShot(Vec2 position, Vec2 velocity) : Projectile(position, velocity)
    {
        symbol = '*';
    }

};
class BombShot : public Projectile
{
public:
    int radius = 2;

    BombShot(Vec2 position, Vec2 velocity) : Projectile(position, velocity)
    {
        symbol = 'B';
    }

protected:

    void HitWall(Level& lvl, Cell center) override
    {
        for (int dx = -radius; dx <= radius; dx++)
        {
            for (int dy = -radius; dy <= radius; dy++)
            {
                Cell c;
                c.x = center.x + dx;
                c.y = center.y + dy;

                Demolition_Wall(lvl, c);
                Destroy_Enemy(lvl, c);
            }
        }
        active = false;
    }

    void HitEnemy(Level& lvl, Cell center) override
    {
        for (int dx = -radius; dx <= radius; dx++)
        {
            for (int dy = -radius; dy <= radius; dy++)
            {
                Cell c;
                c.x = center.x + dx;
                c.y = center.y + dy;

                Destroy_Enemy(lvl, c);
                Demolition_Wall(lvl, c);
            }
        }
        active = false;
    }

private:

    void Explode(Level& lvl, Cell center)
    {
        cout << "BOOM!\n";
    }

};
class PullShot : public Projectile
{
public:
    int radius = 4;

    PullShot(Vec2 position, Vec2 velocity) : Projectile(position, velocity)
    {
        symbol = 'P';
    }

protected:

    void HitWall(Level& lvl, Cell hitCell) override
    {
        ApplyPull(lvl, hitCell);
        active = false;
    }

    void HitEnemy(Level& lvl, Cell hitCell) override
    {
        ApplyPull(lvl, hitCell);
        active = false;
    }

private:
   
    void ApplyPull(Level& lvl, Cell center)
    {
        for (auto& e : lvl.enemies)
        {
            if (!e.alive) continue;

            int dx = center.x - e.pos.x;
            int dy = center.y - e.pos.y;

            int dist2 = dx * dx + dy * dy;
            if (dist2 > radius * radius)
                continue;

            Cell newPos = e.pos;

            if (dx > 0)      newPos.x++;
            else if (dx < 0) newPos.x--;

            if (dy > 0)      newPos.y++;
            else if (dy < 0) newPos.y--;

            std::cout << "Pull enemy from (" 
                      << e.pos.x << "," << e.pos.y 
                      << ") to (" << newPos.x << "," << newPos.y << ")\n";

            if (!Hit_Wall(newPos))
            {
                e.pos = newPos;
            }
            else
            {
                std::cout << "Blocked by wall, enemy stays at (" 
                          << e.pos.x << "," << e.pos.y << ")\n";
            }
        }
    }

};
class BounceShot : public Projectile
{
public:
    int maxBounces = 7;
    int bounces = 0;

    BounceShot(Vec2 position, Vec2 velocity, int maxB = 5)
        : Projectile(position, velocity), maxBounces(maxB)
    {
        symbol = 'o';
    }

protected:
    void HitWall(Level& lvl, Cell g) override
    {
        Cell oldCell;
        oldCell.x = (int)floor(oldPos.x);
        oldCell.y = (int)floor(oldPos.y);

        pos = oldPos;

        float bounceLoss = 0.85f;
        float wallFriction = 0.95f;

        if (oldCell.x != g.x)
        {
            vel.x *= -bounceLoss;
            vel.y *= wallFriction;
        }

        if (oldCell.y != g.y)
        {
            vel.y *= -bounceLoss;
            vel.x *= wallFriction;
        }

        float randomFactor = 0.05f;

        vel.x += ((rand() % 100) / 100.0f - 0.5f) * randomFactor;
        vel.y += ((rand() % 100) / 100.0f - 0.5f) * randomFactor;


        bounces++;

        if (fabs(vel.x) + fabs(vel.y) < 0.05f)
            active = false;

        if (bounces > maxBounces)
            active = false;
    }
};
class DrillShot : public Projectile
{
public:
    int drills = 0;
    int maxDrills = 3;

    DrillShot(Vec2 position, Vec2 velocity) : Projectile(position, velocity)
    {
        symbol = 'D';
    }

protected:
    void HitWall(Level& lvl, Cell g) override
    {
        drills++;

        float drillLoss = 0.8f;
        vel.x *= drillLoss;
        vel.y *= drillLoss;

        if (drills > maxDrills)
            active = false;
    }

};
class SplitShot : public Projectile
{
public:
    int splitCount = 3;
    bool splitted = false;

    SplitShot(Vec2 position, Vec2 velocity)
        : Projectile(position, velocity)
    {
        symbol = 'S';
    }

protected:

    void HitWall(Level& lvl, Cell g) override
    {
        if (splitted)
            return;

        splitted = true;

        float baseAngle = atan2(vel.y, vel.x);

        double speed = sqrt(vel.x * vel.x + vel.y * vel.y);

        for (int i = 0; i < splitCount; i++)
        {
            float angleOffset = (-0.6f + (1.2f / (splitCount - 1)) * i);
            float angle = baseAngle + angleOffset;

            Vec2 newVel;
            newVel.x = cos(angle) * speed;
            newVel.y = sin(angle) * speed;

            Projectile* child = CreateProjectile(ShotType::Normal, pos, newVel);

            RunSingleProjectile(lvl, child);
        }

        active = false;
    }

private:

    void RunSingleProjectile(Level& lvl, Projectile* p)
    {
        while (p->IsActive())
        {
            clearscreen();
            DrawLevel(lvl);
            p->Update(lvl);
            p->Draw();
            Ground_print();
            Sleep(30);
        }

        delete p;
    }
};

int main ()
{
    Welcome_LOGO();
    LoadProgressFromFile();
    progress.unlocked[0] = true ;
    MainMenu();
    SaveProgressToFile();
    return 0;
}

void clearscreen()
{
    system("cls");
}
void SetColor(WORD color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
//Files
void LoadProgressFromFile()
{
    ifstream file("progress.dat",ios::binary);
    if (file.is_open())
    {
        file.read((char*)&progress,sizeof(GameProgress));
        file.close();
    }
}
void SaveProgressToFile()
{
    ofstream file("progress.dat",ios::binary);
    if (file.is_open())
    {
        file.write((char*)&progress,sizeof(GameProgress));
        file.close();
    }
}
//Data Structure functions
bool operator==(const Cell& a, const Cell& b)
{
    return a.x == b.x && a.y == b.y;
}
Cell ToCell(Vec2 v)
{
    return { (int)floor(v.x), (int)floor(v.y) };
}
//Grid
void PlayGround()
{
    Grid.clear();
    Grid.resize(Row, string(Col, ' '));

    for (int i = 0; i < Row; i++)
    {
        for (int j = 0; j < Col; j++)
        {
            if (i == 0 || i == Row - 1 || j == 0 || j == Col - 1)
                Grid[i][j] = '#';
        }
    }
}
void Ground_print()
{
    for (int i = 0; i < Row; i++)
    {
        cout << Grid[i] << endl;
    }
}
void placement_char(Cell Pos, char C)
{
    if (Pos.x >= 0 && Pos.x < Col && Pos.y >= 0 && Pos.y < Row)
        Grid[Pos.y][Pos.x] = C;
}
//Levels Execution
void DrawLevel(Level &lvl)
{
    PlayGround();

    // Player
    placement_char(lvl.player.pos , lvl.player.symbol);

    // Enemies
    for (const auto& enemy : lvl.enemies)
    {
        if (enemy.alive)
            placement_char(enemy.pos , enemy.symbol);
    }

    // Walls
    for (const auto& wall : lvl.walls)
    {
        if (wall.standing)
            placement_char(wall.pos , wall.symbol);
    }
}
void RunSingleShot(Level &lvl)
{
    ShotType shot = GetShotType();
    int power = Get_Speed();
    int angle = Get_Angle();

    Vec2 startPos = {lvl.player.pos.x + 0.5 , lvl.player.pos.y + 0.5};
    Vec2 velocity = Projectile::VelocityFromAngle(power, angle);
    
    Projectile* p = CreateProjectile(shot, startPos, velocity);


    while (p->IsActive())
    {
        clearscreen();
        DrawLevel(lvl);
        p->Update(lvl);
        p->Draw();
        Ground_print();
        Sleep(30);
    }

    delete p;
    p = nullptr; 

    lvl.shotsFired++;
    CheckLevelEnd(lvl);
}
void CheckLevelEnd(Level &lvl)
{
    if (allEnemiesDead(lvl))
        lvl.State = WIN;
    else if (lvl.shotsFired >= lvl.maxShots)
        lvl.State = LOSE;
}
void LoadLevelData(int levelNumber, Level& lvl)
{
    switch (levelNumber)
    {
        case 1:
            Setup_Level_1(lvl);
            break;
        case 2:
            Setup_Level_2(lvl);
            break;
        case 3:
            Setup_Level_3(lvl);
            break;
        case 4:
            Setup_Level_4(lvl);
            break;
        case 5:
            Setup_Level_5(lvl);
            break;
        case 6:
            Setup_Level_6(lvl);
            break;
        case 7:
            Setup_Level_7(lvl);
            break;
        case 8:
            Setup_Level_8(lvl);
            break;
        case 9:
            Setup_Level_9(lvl);
            break;
        case 10:
            Setup_Level_10(lvl);
            break;
        default:
            ResetLevel(lvl);
            break;
    }
}
void StartLevel(int levelNumber)
{
    Level lvl;
    clearscreen();
    LoadLevelData(levelNumber, lvl);
    RunLevel(lvl, levelNumber - 1);
}
void RunLevel(Level &lvl, int levelIndex)
{
    while (lvl.State == RUNNING)
    {
        clearscreen();
        DrawLevel(lvl);
        Ground_print();
        RunSingleShot(lvl);
    }

    clearscreen();

    if(lvl.State == WIN)
    {
        int stars = CalculateStars(lvl);

        progress.stars[levelIndex] = stars;
        progress.scores[levelIndex] = stars * 100;

        if(levelIndex + 1 < Num_lev)
            progress.unlocked[levelIndex + 1] = true;
        
        SaveProgressToFile();
        Win_LOGO();

    }
    if(lvl.State == LOSE)
    {
        Lose_LOGO();
    }

    LevelMenu();
}
void ResetLevel(Level& lvl)
{
    lvl.player.pos = {2, Row - 2};
    lvl.player.symbol = 'P';

    lvl.enemies.clear();
    lvl.walls.clear();

    lvl.maxShots = 10;
    lvl.shotsFired = 0;
    lvl.parShots = 3;
    lvl.State = RUNNING;
}
//progress
void InitProgress(GameProgress &progress)
{
    for(int i=0;i<Num_lev;i++)
    {
        progress.stars[i] = 0;
        progress.scores[i] = 0;
        progress.unlocked[i] = false;
    }

    progress.unlocked[0] = true; // Level 1 unlocked
}
int CalculateStars(Level& lvl)
{
    if(lvl.shotsFired <= lvl.parShots)
        return 3;

    if(lvl.shotsFired <= lvl.parShots + 2)
        return 2;

    return 1;
}
//Projectile
Projectile* CreateProjectile(ShotType type, Vec2 pos, Vec2 vel)
{
    switch (type)
    {
    case ShotType::Normal:
        return new NormalShot(pos, vel);

    case ShotType::Bomb:
        return new BombShot(pos, vel);

    case ShotType::Split:
        return new SplitShot(pos, vel);

    case ShotType::Bounce:
        return new BounceShot(pos, vel);

    case ShotType::Drill:
        return new DrillShot(pos, vel);

    case ShotType::Pull:
        return new PullShot(pos, vel);
    default:
        return nullptr;
    }
}
int Get_Speed()
{
    int speed;

    while (true)
    {
        cout << "Enter Speed (" << physics.minSpeed << "-" << physics.maxSpeed << "): ";

        cin >> speed;

        if (speed >= physics.minSpeed && speed <= physics.maxSpeed)
            return speed;

        cout << "Invalid speed!\n";
    }
}
int Get_Angle()
{
    int angle;

    while (true)
    {
        cout << "Enter Angle (" << physics.minAngle << "-" << physics.maxAngle << "): ";

        cin >> angle;

        if (angle >= physics.minAngle && angle <= physics.maxAngle)
            return angle;

        cout << "Invalid angle!\n";
    }
}
bool IsOutOfBounds(Cell Pos)
{
    return (Pos.x < 0 || Pos.x >= Col || Pos.y < 0 || Pos.y >= Row);
}
void Destroy_Area(Level &lvl, Cell c, int radius)
{
    for (int dx = -radius; dx <= radius; dx++)
    {
        for (int dy = -radius; dy <= radius; dy++)
        {
            Cell t{ c.x + dx, c.y + dy };
            
            if (Hit_Enemy(t))
                Destroy_Enemy(lvl, t);

            if (IsOutOfBounds(t))
                continue;

            if (Hit_Wall(t))
                Demolition_Wall(lvl, t);
        }
    }
}
//Enemy
bool Hit_Enemy(Cell Pos)
{
    return Grid[Pos.y][Pos.x] == 'E';
}
void Destroy_Enemy(Level &lvl, Cell Pos)
{
    for (int i = 0; i < lvl.enemies.size(); i++)
    {
        if (lvl.enemies[i].alive && lvl.enemies[i].pos == Pos )
        {
            lvl.enemies[i].alive = false;
            Grid[Pos.y][Pos.x] = ' ';
            break;
        }
    }
}
bool allEnemiesDead(Level &lvl)
{
    for (int i = 0; i < lvl.enemies.size(); i++)
    {
        if(lvl.enemies[i].alive)
            return false;
    }
    return true;
}
//Wall
bool Hit_Wall(Cell Pos)
{
    return Grid[Pos.y][Pos.x] == '#';
}
void Demolition_Wall(Level &lvl, Cell Pos)
{
    for (int i = 0; i < lvl.walls.size(); i++)
    {
        if (lvl.walls[i].standing && lvl.walls[i].pos.x == Pos.x && lvl.walls[i].pos.y == Pos.y)
        {
            lvl.walls[i].standing = false;
            break;
        }
    }
}
//Graphics
void Welcome_LOGO()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    SetColor(15);

    cout <<
R"(==============================================================)";

    SetColor(14);
    cout <<
R"(
            ____   ___  _   _ ____   ___  _     _____
           / ___| / _ \| \ | / ___| / _ \| |   | ____|
          | |    | | | |  \| \___ \| | | | |   |  _|
          | |___ | |_| | |\  |___) | |_| | |___| |___
           \____| \___/|_| \_|____/ \___/|_____|_____|
   ____  ____   ___      _ _____ ____ _____ ___ _     _____
  |  _ \|  _ \ / _ \    | | ____/ ___|_   _|_ _| |   | ____|
  | |_) | |_) | | | |_  | |  _|| |     | |  | || |   |  _|
  |  __/|  _ <| |_| | |_| | |__| |___  | |  | || |___| |___
  |_|   |_| \_\\___/ \___/|_____\____| |_| |___|_____|_____|
                    ____    _    __  __ _____
                   / ___|  / \  |  \/  | ____|
                  | |  _  / _ \ | |\/| |  _|
                  | |_| |/ ___ \| |  | | |___
                   \____/_/   \_\_|  |_|_____|
)";

    SetColor(15);
    cout <<
R"(==============================================================
)";

    SetColor(10);
    cout <<
R"(               Welcome to ANGRY BIRDS TERMINAL
)";


    SetColor(11);
    cout <<
R"(                   Press ENTER to Start
)";


    SetColor(15);
    cout <<
R"(==============================================================)";


    SetColor(15);

    cin.get();
}
void Win_LOGO()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    auto SetColor = [&](int c){ SetConsoleTextAttribute(hConsole, c); };

    SetColor(15);
    cout << "==============================================================\n";

    SetColor(14);
    cout <<
R"(     __   __  ___   _   _  __        _____ _   _     _
     \ \ / / / _ \ | | | | \ \      / /_ _| \ | |   | |
      \ V / | | | || | | |  \ \ /\ / / | ||  \| |   | |
       | |  | |_| || |_| |   \ V  V /  | || |\  |   |_|
       |_|   \___/  \___/     \_/\_/  |___|_| \_|   (_)
)";

    SetColor(10);
    cout << "\n                    Y O U   W I N !\n\n";

    SetColor(15);
    cout << "                 The enemies are defeated\n";

    cout << "==============================================================\n";

    SetColor(11);
    cout << "              Press ENTER to return to menu\n";

    SetColor(15);
    cout << "==============================================================";

    SetColor(15);
    cin.ignore();
    cin.get();
}
void Lose_LOGO()
{
    SetColor(15);
    cout << R"(==============================================================)";

    SetColor(12);
    cout << R"(
   ____    _    __  __ _____    _____     _______ ____
  / ___|  / \  |  \/  | ____|  / _ \ \   / / ____|  _ \
 | |  _  / _ \ | |\/| |  _|   | | | \ \ / /|  _| | |_) |
 | |_| |/ ___ \| |  | | |___  | |_| |\ V / | |___|  _ <
  \____/_/   \_\_|  |_|_____|  \___/  \_/  |_____|_| \_\

)";

    SetColor(4);
    cout << R"(                    G A M E   O V E R

)";

    SetColor(7);
    cout << R"(               The enemies are still alive
)";


    SetColor(15);
    cout << R"(==============================================================
    )";


    SetColor(14);
    cout << R"(              Press ENTER to try again
)";

    SetColor(15);
    cout << R"(==============================================================
)";

    SetColor(15);

    cin.ignore();
    cin.get();
}
//Menus
void MainMenu()
{
    
    int choice;
    string status = "Welcome!";
    while(true)
    {
        clearscreen();

        SetColor(14);
        cout << LINE << "\n";
        cout << "                    CONSOLE PROJECTILE GAME                    \n";
        cout << LINE << "\n";

        SetColor(15);
        cout << "   [1] Start Game           \n";
        cout << "   [2] How To Play          \n";
        cout << "   [3] Progress / Stars     \n";
        cout << "   [4] About The Developer  \n";
        cout << "   [5] Show Map             \n";
        cout << "   [6] Setting            \n\n";
        cout << "   [0] Exit               \n\n";
        cout << LINE << "\n";

        if(status.find("Invalid") != string::npos)
            SetColor(12);
        else
            SetColor(10);
        cout << "   Status: " << status << "\n";
        
        SetColor(15);
        cout << "   Select an option by entering its number                     \n";
        cout << LINE << "\n";
        
        SetColor(11);
        cout << "   Your Choice: ";
        cin>>choice;
        
        if(cin.fail())
        {
            cin.clear();
            cin.ignore(1000,'\n');
            status = "Invalid input! Please enter a number.";
            continue;
        }

        switch(choice)
        {
            case 1:
            {
                LevelMenu();
                status = "Returned from Start Game.";
                break;
            }
            case 2:
            {   
                HowToPlayMenu();
                status = "Returned from How To Play.";
                break;
            }
            case 3:
            {   
                ProgressStarsMenu();
                status = "Returned from Progress.";
                break;
            }
            case 4:
            {   
                AboutDeveloperMenu();
                status = "Returned from About Developer.";
                break;
            }
            case 5:
            {   
                MapMenu();
                status = "Map not implemented yet.";
                break;
            }
            case 6:
            {
                SettingsMenu();
                status = "Returned from Settings.";
                break;
            }
            case 0:
                exit(0);

            default:
                status = "Invalid choice! Please select 0 - 6.";
        }
    }
    SetColor(15);
}
void LevelMenu()
{
    int page = 1;
    const int perPage = 5;
    int totalPages = (Num_lev + perPage - 1) / perPage;

    string choice;
    string status = "Select a level.";

    while(true)
    {
        clearscreen();
        
        SetColor(14);
        cout << LINE << "\n";
        cout << "                          LEVEL SELECT                        \n";
        cout << LINE << "\n";

        SetColor(11);
        cout << "Page " << page << "/" << totalPages << "\n";
        
        SetColor(14);
        cout << LINE << "\n";
        
        int start = (page - 1) * perPage + 1;
        int end = start + perPage - 1;

        if(end > Num_lev)
            end = Num_lev;

        for(int i = start; i <= end; i++)
        {
            SetColor(15);
            cout << i << ". Level " << i << " ";

            if(progress.unlocked[i-1])
            {
                SetColor(14);
                cout << "[Unlocked]\n";
            }
            else
            {
                SetColor(12);
                cout << "[LOCKED]\n";
            }
        }

        SetColor(14);
        cout << LINE << "\n";

        if(status.find("Invalid") != string::npos || status.find("LOCKED")  != string::npos)
            SetColor(12);
        else
            SetColor(10);
        cout << "Status: " << status << "\n";

        SetColor(15);
        cout << "--------------------------------------------------------------\n";
        cout << "Enter Level Number | N - Next | P - Previous | B - Back\n";
        
        SetColor(14);
        cout << LINE << "\n";;
        
        SetColor(11);
        cout << "Your Choice: ";

        cin >> choice;

        if(choice == "n" || choice == "N")
        {
            if(page < totalPages)
            {
                page++;
                status = "Moved to next page.";
            }
            else
                status = "Already on last page.";
        }
        else if(choice == "p" || choice == "P")
        {
            if(page > 1)
            {
                page--;
                status = "Moved to previous page.";
            }
            else
                status = "Already on first page.";
        }
        else if(choice == "b" || choice == "B")
            MainMenu();
        else
        {
            bool isNumber = all_of(choice.begin(), choice.end(), ::isdigit);

            if(isNumber)
            {
                int level = stoi(choice);

                if(level >= 1 && level <= Num_lev)
                {
                    if(progress.unlocked[level-1])
                    {
                        status = "Starting Level " + to_string(level);
                        StartLevel(level);
                    }
                    else
                        status = "Level " + choice + " is LOCKED.";
                }
                else
                    status = "Invalid level number.";
            }
            else
                status = "Invalid input. Enter 1-" + to_string(Num_lev) + " or N/P/B.";
        }
    }
}
void HowToPlayMenu()
{
    int page = 1;
    char choice;
    string status = "Use N/P to navigate pages.";

    while(true)
    {
        clearscreen();
        cout << LINE << "\n";
        cout << "                          HOW TO PLAY                         \n";
        cout << LINE << "\n";

        switch (page)
        {
            case 1:
            {
                cout << "Page 1/3 - Controls:\n";
                cout << LINE << "\n";
                cout << "1. Enter Angle (0-90 degrees)\n";
                cout << "2. Enter Speed (1-12)\n";
                cout << "3. Press Enter to simulate each step\n";
                cout << "4. Press Q to quit level\n";
                break;
            }
            case 2:
            {
                cout << "Page 2/3 - Game Rules:\n";
                cout << LINE << "\n";
                cout << "1- Destroy all enemies (E)\n";
                cout << "2- Avoid hitting walls (#)\n";
                cout << "3- Limited shots per level\n";
                cout << "4- Get 3 stars by using fewer shots\n";
                break;
            }
            case 3:
            {
                cout << "Page 3/3 - Special Shots:\n";
                cout << LINE << "\n";
                cout << "1. Explosive: Destroys nearby targets\n";
                cout << "2. Bouncy: Bounces 3 times off walls\n";
                cout << "3. Dropper: Drops straight down\n";
                cout << "4. Boomerang: Returns after distance\n";
            }
        }

        cout << LINE << "\n";
        cout << "N - Next Page  |  P - Previous Page  |  B - Back to Menu\n";
        cout << LINE << "\n";
        cout << "Your Choice: ";
        cin >> choice;

        switch(choice)
        {
            case 'N':
            case 'n':
            {    
                if(page < 3)
                {
                    page++;
                    status = "Moved to next page.";
                }
                else
                    status = "You are on the last page.";
                break;
            }
            case 'P':
            case 'p':
            {
                if(page > 1)
                {
                    page--;
                    status = "Moved to previous page.";
                }
                else
                    status = "You are on the first page.";
                break;
            }
            case 'B':
            case 'b':
                return;

            default:
                status = "Invalid input. Use N / P / B.";
        }
       
    }

}
void ProgressStarsMenu()
{
    char choice;
    string status = "View your progress.";

    while(true)
    {
        clearscreen();

        cout << LINE << "\n";
        cout << "                      PROGRESS & STARS                        \n";
        cout << LINE << "\n\n";

        for(int i = 0; i < Num_lev ; i++)
        {
            cout << "Level " << i + 1 << ": ";

            if(progress.unlocked[i])
            {
                cout << "[";

                for(int j = 0; j < 3; j++)
                {
                    if(j < progress.stars[i])
                        cout << "*";
                    else
                        cout << " ";
                }

                cout << "] ";
                cout << "Score: " << progress.scores[i];

                if(progress.stars[i] == 0)
                    cout << " (Not completed)";
                else if(progress.stars[i] == 3)
                    cout << " PERFECT!\n";

                cout << endl;
            }
            else
                cout << "LOCKED (Complete Level " << i << " first)\n";
        }

        cout << "\n" << LINE << "\n";
        cout << "Status: " << status << "\n";
        cout << LINE << "\n";
        cout << "R - Reset Progress\n";
        cout << "B - Back to Main Menu\n";
        cout << LINE << "\n";
        cout << "Your Choice: ";
        cin >> choice;
        switch(choice)
        {
            case 'r':
            case 'R':
            {
                for(int i = 0; i < Num_lev ; i++)
                {
                    progress.stars[i] = 0;
                    progress.scores[i] = 0;
                    progress.unlocked[i] = (i == 0);
                }

                SaveProgressToFile();
                status = "Progress reset successfully.";
                break;
            }

            case 'B':
            case 'b':
                return;

            default:
                status = "Invalid input. Choose 1 or 2.";
        }
    }
}
void AboutDeveloperMenu()
{
    char choice;

    while(true)
    {
        clearscreen();

        cout << LINE << "\n";
        cout << "                     ABOUT THE DEVELOPER                      \n";
        cout << LINE << "\n\n";

        cout << "Name       : Mohammad BaniAsadi\n";
        cout << "Field      : Computer Engineering\n";
        cout << "University : University of Birjand\n\n";

        cout << "This console game was originally developed as a project\n";
        cout << "for the 'Fundamentals of Computer Programming' course.\n";
        cout << "I am continuing to develop and improve this project\n";
        cout << "during my free time between academic semesters.\n\n";

        cout << "GitHub : [ Your GitHub Link Here ]\n";

        cout <<"\n"<< LINE << "\n";
        cout << "Press B to go back\n";
        cout << LINE << "\n";

        cin >> choice;

        if(choice == 'B' || choice == 'b')
            return;
    }
}
void MapMenu()
{
    int page = 1;
    const int perPage = 5;
    int totalPages = (Num_lev + perPage - 1) / perPage;

    string choice;
    string status = "Select a level to view map.";

    while(true)
    {
        clearscreen();

        cout << LINE << endl;
        cout << "                        MAP MENU\n";
        cout << LINE << endl;

        cout << "Page " << page << "/" << totalPages << endl;
        cout << LINE << endl;

        int start = (page - 1) * perPage + 1;
        int end = start + perPage - 1;

        if(end > Num_lev)
            end = Num_lev;

        for(int i = start; i <= end; i++)
        {
            cout << i << ". Level " << i << " ";

            if(progress.unlocked[i-1])
                cout << "[Unlocked]\n";
            else
                cout << "[Locked]\n";
        }

        cout << LINE << endl;
        cout << "Status: " << status << endl;
        cout << "-------------------------------------------------\n";
        cout << "Enter Level Number | N-next | P-previous | B-back\n";
        cout << LINE << endl;
        cout << "Your Choice: ";

        cin >> choice;

        if(choice == "n" || choice == "N")
        {
            if(page < totalPages)
            {
                page++;
                status = "Next page.";
            }
            else
                status = "Already last page.";
        }
        else if(choice == "p" || choice == "P")
        {
            if(page > 1)
            {
                page--;
                status = "Previous page.";
            }
            else
                status = "Already first page.";
        }
        else if(choice == "b" || choice == "B")
        {
            return;
        }
        else
        {
            bool isNumber = true;

            for(char c : choice)
            {
                if(!isdigit(c))
                {
                    isNumber = false;
                    break;
                }
            }

            if(isNumber)
            {
                int level = stoi(choice);

                if(level >= 1 && level <= Num_lev)
                {
                    if(progress.unlocked[level-1])
                    {
                        ShowMapOfLevel(level);
                    }
                    else
                    {
                        status = "Level is locked.";
                    }
                }
                else
                {
                    status = "Invalid level.";
                }
            }
            else
            {
                status = "Invalid input.";
            }
        }

    }
}
void SettingsMenu()
{
    char choice;
    string status = "Select a setting to change.";

    while(true)
    {
        clearscreen();
        cout << LINE << "\n";
        cout << "========================== SETTINGS ==========================\n";
        cout << LINE << "\n\n";
        cout << "1. Map Size           : " << Row << " x " << Col << endl;
        cout << "2. TimeStep           : " << physics.timeStep << endl;
        cout << "3. Gravity            : " << physics.gravity << endl;

        cout << "\n" << LINE << "\n";
        cout << "Status: " << status << endl;
        cout << LINE << "\n";
        cout << "Enter Number of a setting option | \n";
        cout << "B - Back to Main Menu | R - Returnto default sttings\n";
        cout << LINE << "\n";;
        cout << "Your Choice: ";
        
        cin >> choice;

        switch (choice)
        {
            case '1':
            {
                ChangeMapSize(status);
                break;
            }

            case '2':
            {
                ChangeTimeStep(status);
                break;
            }
            
            case '3':
            {
                ChangeGravity(status);
                break;
            }
            
            case 'r':
            case 'R':
            {
                physics = DefaultPhysics;
                status = "Physics settings reset.";
                break;
            }
            
            case 'b':
            case 'B':
                return;
            
            default:
                status = "Invalid input.";
                break;
        }
    }
}
//Menu subset
void ShowMapOfLevel(int levelNumber)
{
    Level lvl;

    clearscreen();
    LoadLevelData(levelNumber, lvl);
    DrawLevel(lvl);
    Ground_print();

    cout << "\nPress Enter to return...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}
void ChangeMapSize(string &status)
{
    int newRow, newCol;

    cout << "Current Map Size: " << Row << " x " << Col << endl;

    cout << "Enter Height (10 - 40): ";
    cin >> newRow;

    if(newRow < 10 || newRow > 40)
    {
        status = "Invalid height. Must be between 10 and 40.";
        return;
    }

    cout << "Enter Width (30 - 120): ";
    cin >> newCol;

    if(newCol < 30 || newCol > 120)
    {
        status = "Invalid width. Must be between 30 and 120.";
        return;
    }

    Row = newRow;
    Col = newCol;

    status = "Map size updated successfully.";
}
void ChangeGravity(string &status)
{
    double newGravity;

    cout << "Current Gravity: " << physics.gravity << endl;
    cout << "Enter new Gravity (0.1 - 5): ";

    cin >> newGravity;

    if(newGravity < 0.1 || newGravity > 5)
    {
        status = "Invalid gravity value.";
        return;
    }

    physics.gravity = newGravity;

    status = "Gravity updated successfully.";
}
void ChangeTimeStep(string &status)
{
    double newDt;

    cout << "Current Time Step: " << physics.timeStep << endl;
    cout << "Enter new Time Step (0.01 - 0.20): ";

    cin >> newDt;

    if(newDt < 0.01 || newDt > 0.20)
    {
        status = "Invalid time step. Must be between 0.01 and 0.20.";
        return;
    }

    physics.timeStep = newDt;

    status = "Time step updated successfully.";
}
//Setup_Level
void Setup_Level_1(Level& lvl)   
{
    ResetLevel(lvl);

    lvl.maxShots = 10;
    lvl.parShots = 3;

    lvl.player.pos = {2, Row - 2};

    int wallX      = (Col * 6) / 7;
    int upperWallY = (Row * 3) / 4;
    int lowerWallY = Row / 4;

    for (int y = Row - 1; y >= upperWallY; --y)
    {
        lvl.walls.push_back({ {wallX, y}, '#', true });
    }

    for (int x = wallX; x < Col; ++x)
    {
        lvl.walls.push_back({ {x, upperWallY}, '#', true });
    }

    for (int x = wallX; x < Col; ++x)
    {
        lvl.walls.push_back({ {x, lowerWallY}, '#', true });
    }

    int e1x = (Col * 13) / 14;
    int e1y = upperWallY - 1;

    int e2x = (Col * 19) / 21;
    int e2y = lowerWallY - 1;

    int e3x = (Col * 20) / 21;
    int e3y = lowerWallY - 1;

    e1x = min(e1x, Col - 2);
    e2x = min(e2x, Col - 2);
    e3x = min(e3x, Col - 2);

    e1y = max(e1y, 1);
    e2y = max(e2y, 1);
    e3y = max(e3y, 1);

    lvl.enemies.push_back({ {e1x, e1y}, 'E', true });
    lvl.enemies.push_back({ {e2x, e2y}, 'E', true });
    lvl.enemies.push_back({ {e3x, e3y}, 'E', true });
}
void Setup_Level_2(Level& lvl)   
{
    ResetLevel(lvl);

    lvl.maxShots = 10;
    lvl.parShots = 3;

    lvl.player.pos = {2, Row - 2};

    int w = Col / 10;
    
    // platform 1
    int y1 = (Row * 4) / 5;
    int x1 = (Col * 7) / 10;

    for(int x = x1; x <= x1 + w && x < Col-1; x++)
        lvl.walls.push_back({{x, y1}, '#', true});

    int y2 = (Row * 3) / 5;
    int x2 = (Col * 5) / 10;

    for(int x = x2; x <= x2 + w && x < Col-1; x++)
        lvl.walls.push_back({{x, y2}, '#', true});

    int y3 = (Row * 2) / 5;
    int x3 = (Col * 8) / 10;


    for(int x = x3; x <= x3 + w && x < Col-1; x++)
        lvl.walls.push_back({{x, y3}, '#', true});

    int y4 = (Row * 1) / 4;
    int x4 = (Col * 4) / 10;

    for(int x = x4; x <= x4 + w && x < Col-1; x++)
        lvl.walls.push_back({{x, y4}, '#', true});


    lvl.enemies.push_back({{x1 + w/2, y1 - 1}, 'E', true});
    lvl.enemies.push_back({{x2 + w/2, y2 - 1}, 'E', true});
    lvl.enemies.push_back({{x3 + w/2, y3 - 1}, 'E', true});
    lvl.enemies.push_back({{x4 + w/2, y4 - 1}, 'E', true});

}
void Setup_Level_3(Level& lvl)   
{
    ResetLevel(lvl);

    lvl.maxShots = 10;
    lvl.parShots = 3;
    lvl.player.pos = {2, Row - 2};

    int x1 = (Col * 3) / 5;
    int y1 = static_cast<int>(Row / 1.6);

    int x2 = (Col * 4) / 5;
    int y2 = static_cast<int>(Row / 2.4);

    int x3 = (Col * 9) / 10;
    int y3 = Row / 4;

    for (int y = Row - 1; y >= y1; --y)
        lvl.walls.push_back({{x1, y}, '#', true});

    for (int x = x1; x < Col; ++x)
        lvl.walls.push_back({{x, y1}, '#', true});

    for (int y = y1; y >= y2; --y)
        lvl.walls.push_back({{x2, y}, '#', true});

    for (int x = x2; x < Col; ++x)
        lvl.walls.push_back({{x, y2}, '#', true});

    for (int y = y2; y >= y3; --y)
        lvl.walls.push_back({{x3, y}, '#', true});

    for (int x = x3; x < Col; ++x)
        lvl.walls.push_back({{x, y3}, '#', true});

    lvl.enemies.push_back({{(Col * 7) / 10, y1 - 1}, 'E', true});
    lvl.enemies.push_back({{(Col * 17) / 20, y2 - 1}, 'E', true});
    lvl.enemies.push_back({{(Col * 19) / 20, y3 - 1}, 'E', true});
}
void Setup_Level_4(Level& lvl)   
{
    ResetLevel(lvl);

    lvl.maxShots = 10;
    lvl.parShots = 3;

    lvl.player.pos = {2, Row - 2};

    int w = Col / 12;
    int h = Row / 7;

    int x = (Col * 3) / 4;
    int floors = 3;
    int gap = 1;                    
    int yStart = Row / 5;
    
    auto MakeRoom = [&](int x, int y)
    {
        for(int i = 0; i < w; i++)
            lvl.walls.push_back({{x + i, y}, '#', true});

        for(int i = 0; i < w; i++)
            lvl.walls.push_back({{x + i, y + h}, '#', true});

        for(int j = 0; j <= h; j++)
            lvl.walls.push_back({{x + w - 1, y + j}, '#', true});

        lvl.enemies.push_back({{x + w/2, y + h - 1}, 'E', true});
    };

    int mid = x + w / 2;
    for(int yy = 1; yy < yStart; yy++)
        lvl.walls.push_back({{mid, yy}, '#', true});

    for(int f = 0; f < floors; f++)
    {
        int y = yStart + f * (h + gap);
        MakeRoom(x, y);
    }
}
void Setup_Level_5(Level& lvl)   
{
    ResetLevel(lvl);

    lvl.maxShots = 10;
    lvl.parShots = 3;

    lvl.player.pos = {2, Row - 2};

    int w = Col / 12;
    int h = Row / 6;

    int x1 = (Col * 3) / 5;
    int x2 = (Col * 7) / 10;
    int x3 = (Col * 4) / 5;

    int yTop = Row / 5;

    auto MakeRoom = [&](int x, int y)
    {
        int mid = x + w/2;  


        for(int yy = 1; yy < y; yy++)
            lvl.walls.push_back({{mid, yy}, '#', true});


        for(int i = 0; i < w; i++)
            lvl.walls.push_back({{x + i, y}, '#', true});


        for(int i = 0; i < w; i++)
            lvl.walls.push_back({{x + i, y + h}, '#', true});


        for(int j = 0; j <= h; j++)
            lvl.walls.push_back({{x + w - 1, y + j}, '#', true});

            lvl.enemies.push_back({{x + w/2, y + h - 1}, 'E', true});
    };

    MakeRoom(x1, yTop);
    MakeRoom(x2, yTop + h + Row/12);
    MakeRoom(x3, yTop + 2*h + Row/10);
}
void Setup_Level_6(Level& lvl)   
{
    ResetLevel(lvl);

    lvl.maxShots = 10;
    lvl.parShots = 5;
    lvl.player.pos = {2, Row - 2};

    int x1 = (Col * 2) / 3;
    int x2 = (Col * 5) / 6;
    int x4Start = (Col * 3) / 6;
    int x5End   = (Col * 7) / 12;

    int yTop    = (Row * 5) / 6;
    int yMid    = (Row * 7) / 12;
    int yBottom = (Row * 2) / 6;

    for (int y = Row - 1; y >= yTop; --y)
        lvl.walls.push_back({{x1, y}, '#', true});

    for (int y = yTop; y >= yBottom; --y)
        lvl.walls.push_back({{x2, y}, '#', true});

    for (int x = x1; x <= x2; ++x)
        lvl.walls.push_back({{x, yTop}, '#', true});

    for (int x = x2; x >= x4Start; --x)
        lvl.walls.push_back({{x, yMid}, '#', true});

    for (int x = x2; x >= x5End; --x)
        lvl.walls.push_back({{x, yBottom}, '#', true});

    lvl.enemies.push_back({{(Col * 9) / 12,  yTop - 1}, 'E', true});
    lvl.enemies.push_back({{(Col * 7) / 12,  yMid - 1}, 'E', true});
    lvl.enemies.push_back({{(Col * 8) / 12,  yMid - 1}, 'E', true});
    lvl.enemies.push_back({{(Col * 9) / 12,  yMid - 1}, 'E', true});
    lvl.enemies.push_back({{(Col * 17) / 24, yBottom - 1}, 'E', true});
}
void Setup_Level_7(Level& lvl)   
{
    ResetLevel(lvl);

    lvl.maxShots = 10;
    lvl.parShots = 5;
    lvl.player.pos = {2, Row - 2};

    int xLeft   = (Col * 2) / 3;
    int xRight  = (Col * 5) / 6;
    int xMid    = (Col * 9) / 12;
    int xUpper2 = (Col * 11) / 12;

    int yTopBox    = (Row * 6) / 8;
    int yUpperLine = Row / 8;
    int yMidLine   = (Row * 3) / 8;

    for (int y = Row - 1; y >= yTopBox; --y)
        lvl.walls.push_back({{xLeft, y}, '#', true});

    for (int y = Row - 1; y >= yTopBox; --y)
        lvl.walls.push_back({{xRight, y}, '#', true});

    for (int x = xLeft; x <= xRight; ++x)
        lvl.walls.push_back({{x, yTopBox}, '#', true});

    for (int x = Col - 1; x >= xMid; --x)
        lvl.walls.push_back({{x, yUpperLine}, '#', true});

    for (int x = xMid; x <= xUpper2; ++x)
        lvl.walls.push_back({{x, yMidLine}, '#', true});

    for (int y = yUpperLine; y <= yMidLine; ++y)
        lvl.walls.push_back({{xMid, y}, '#', true});

    lvl.enemies.push_back({{(Col * 13) / 18, yTopBox - 1}, 'E', true});
    lvl.enemies.push_back({{(Col * 14) / 18, yTopBox - 1}, 'E', true});
    //lvl.enemies.push_back({{xMid, Row - 2}, 'E', true});
    lvl.enemies.push_back({{(Col * 7) / 8, yUpperLine - 1}, 'E', true});
    lvl.enemies.push_back({{(Col * 10) / 12, yMidLine - 1}, 'E', true});
}
void Setup_Level_8(Level& lvl)   
{
    ResetLevel(lvl);

    lvl.maxShots = 10;
    lvl.parShots = 7;
    lvl.player.pos = {2, Row - 2};

    int x1 = (Col * 2) / 5;
    int x2 = (Col * 16) / 25;
    int x3 = (Col * 19) / 25;
    int x4 = (Col * 41) / 50;

    int yTop    = (Row * 3) / 4;
    int yMid    = (Row * 2) / 4;
    int yBottom = Row / 4;

    for (int y = Row - 1; y >= yTop; --y)
        lvl.walls.push_back({{x1, y}, '#', true});

    for (int y = Row - 1; y >= yMid; --y)
        lvl.walls.push_back({{x2, y}, '#', true});

    for (int y = Row - 1; y >= yMid; --y)
        lvl.walls.push_back({{x3, y}, '#', true});

    for (int x = x1; x <= x2; ++x)
        lvl.walls.push_back({{x, yTop}, '#', true});

    for (int x = x2; x <= x3; ++x)
        lvl.walls.push_back({{x, yMid}, '#', true});

    for (int x = x3; x < Col; ++x)
        lvl.walls.push_back({{x, yTop}, '#', true});

    for (int y = 0; y <= yBottom; ++y)
        lvl.walls.push_back({{x4, y}, '#', true});

    for (int x = x4; x < Col; ++x)
        lvl.walls.push_back({{x, yBottom}, '#', true});

    lvl.enemies.push_back({{(Col * 13) / 25, yTop - 1}, 'E', true});

    lvl.enemies.push_back({{(Col * 35) / 50, yMid - 1}, 'E', true});

    lvl.enemies.push_back({{(Col * 22) / 25, yTop - 1}, 'E', true});

    lvl.enemies.push_back({{(Col * 91) / 100, yBottom - 1}, 'E', true});
}
void Setup_Level_9(Level &lvl)  
{
    ResetLevel(lvl);

    lvl.maxShots = 3;
    lvl.parShots = 2;

    lvl.player.pos = {2, Row - 2};

    // anchor ساختمان (کمی متمایل به سمت راست)
    int anchorX = (Col * 2) / 3;

    int baseTop = (Row * 3) / 4;
    int baseBottom = Row - 1;

    int spacing = Col / 24;

    int c1 = anchorX - spacing * 3;
    int c2 = anchorX - spacing;
    int c3 = anchorX + spacing;
    int c4 = anchorX + spacing * 3;

    for (int y = baseTop; y <= baseBottom; y++)
        lvl.walls.push_back({{c1, y}, '#', true});

    for (int y = baseTop; y <= baseBottom; y++)
        lvl.walls.push_back({{c2, y}, '#', true});

    for (int y = baseTop; y <= baseBottom; y++)
        lvl.walls.push_back({{c3, y}, '#', true});

    for (int y = baseTop; y <= baseBottom; y++)
        lvl.walls.push_back({{c4, y}, '#', true});
    
    int platformY = baseTop;

    for (int x = c1; x <= c4; x++)
        lvl.walls.push_back({{x, platformY}, '#', true});

    int dist = c2 - c1;

    int upperHeight = (dist - 1) / 2;

    int u1 = (c1 + c2) / 2;
    int u2 = (c2 + c3) / 2;
    int u3 = (c3 + c4) / 2;

    for (int y = platformY - 1; y >= platformY - upperHeight; y--)
        lvl.walls.push_back({{u1, y}, '#', true});

    for (int y = platformY - 1; y >= platformY - upperHeight; y--)
        lvl.walls.push_back({{u2, y}, '#', true});

    for (int y = platformY - 1; y >= platformY - upperHeight; y--)
        lvl.walls.push_back({{u3, y}, '#', true});

    int upperPlatformY = platformY - upperHeight - 1;

    for (int x = u1; x <= u3; x++)
        lvl.walls.push_back({{x, upperPlatformY}, '#', true});

    int uDist = u2 - u1;
    int upper2Height = (uDist - 1) / 2;

    int uu1 = (u1 + u2) / 2;
    int uu2 = (u2 + u3) / 2;

    for (int y = upperPlatformY - 1; y >= upperPlatformY - upper2Height; y--)
        lvl.walls.push_back({{uu1, y}, '#', true});

    for (int y = upperPlatformY - 1; y >= upperPlatformY - upper2Height; y--)
        lvl.walls.push_back({{uu2, y}, '#', true});

    int topPlatformY = upperPlatformY - upper2Height - 1;

    for (int x = uu1; x <= uu2; x++)
        lvl.walls.push_back({{x, topPlatformY}, '#', true});

    int startLeft = uu1 + 1;
    int startRight = uu2 - 1;

    int width = startRight - startLeft + 1;
    int layers = (width + 1) / 2;   // تا برسیم به عرض 1

    for (int i = 0; i < layers; i++)
    {
        int y = topPlatformY - 1 - i;
        int left = startLeft + i;
        int right = startRight - i;

        for (int x = left; x <= right; x++)
            lvl.walls.push_back({{x, y}, '#', true});
    }


    int enemyY = platformY - 1;

    int e1 = (u1 + u2) / 2;
    int e2 = (u2 + u3) / 2;

    lvl.enemies.push_back({{e1, enemyY}, 'E', true});
    lvl.enemies.push_back({{e2, enemyY}, 'E', true});

    int enemy2X = (uu1 + uu2) / 2;
    int enemy2Y = upperPlatformY - 1;

    lvl.enemies.push_back({{enemy2X, enemy2Y}, 'E', true});

    int enemyTopX = (uu1 + uu2) / 2;
    int enemyTopY = topPlatformY - layers - 1;

    lvl.enemies.push_back({{enemyTopX, enemyTopY}, 'E', true});


}
void Setup_Level_10(Level& lvl)   
{
    ResetLevel(lvl);

    lvl.maxShots = 10;
    lvl.parShots = 7;

    lvl.player.pos = {2, Row - 2};

    int offset = Row / 8;   // مقدار پایین آوردن آدمک (فقط Y)

    int headY = Row * 3 / 14 + offset;
    for(int x = Col*29/54; x <= Col*35/54; x++)
        lvl.walls.push_back({{x,headY},'#',true});

    int neckY = Row * 5 / 14 + offset;

    for(int x = Col*29/54; x <= Col*35/54; x++)
        lvl.walls.push_back({{x,neckY},'#',true});

    for(int y = headY; y <= neckY; y++)
    {
        lvl.walls.push_back({{Col*29/54,y},'#',true});
        lvl.walls.push_back({{Col*35/54,y},'#',true});
    }

    int armY = Row * 6 / 14 + offset;
    for(int x = Col*16/54; x <= Col*20/54; x++)
        lvl.walls.push_back({{x,armY},'#',true});

    int rightY = Row * 5 / 14 + offset;
    for(int x = Col*42/54; x <= Col*48/54; x++)
        lvl.walls.push_back({{x,rightY},'#',true});

    int armsY = Row * 7 / 14 + offset;
    for(int x = Col*20/54; x <= Col*42/54; x++)
        lvl.walls.push_back({{x,armsY},'#',true});


    for(int y = armY; y <= armsY; y++)
        lvl.walls.push_back({{Col*20/54,y},'#',true});

    for(int y = rightY; y <= armsY; y++)
        lvl.walls.push_back({{Col*42/54,y},'#',true});

    int legsY = Row * 10 / 14 + offset;
    int spineX = Col * 32 / 54;

    for(int y = legsY; y >= neckY; y--)
        lvl.walls.push_back({{spineX,y},'#',true});

    for(int x = Col*26/54; x <= Col*38/54; x++)
        lvl.walls.push_back({{x,legsY},'#',true});

    for(int y = legsY; y <= Row*12/14 + offset; y++)
    {
        lvl.walls.push_back({{Col*26/54,y},'#',true});
        lvl.walls.push_back({{Col*38/54,y},'#',true});
    }

    lvl.enemies.push_back({{Col*30/54, headY-1}, 'E', true});
    lvl.enemies.push_back({{Col*34/54, headY-1}, 'E', true});
    //lvl.enemies.push_back({{Col*32/54, neckY-1}, 'E', true});
    lvl.enemies.push_back({{Col*48/54, rightY-1}, 'E', true});
    lvl.enemies.push_back({{Col*18/54, armY-1}, 'E', true});
    lvl.enemies.push_back({{Col*24/54, armsY-1}, 'E', true});
    lvl.enemies.push_back({{Col*32/54, legsY-1}, 'E', true});
    lvl.enemies.push_back({{Col*28/54, legsY-1}, 'E', true});

}
