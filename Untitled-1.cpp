#include <iostream>
#include <windows.h>
#include <cmath>
#include <fstream> //فایل
//#include <rang.hpp>

    using namespace std;
    #define WIN32_LEAN_AND_MEAN
    #define _USE_MATH_DEFINES
    //using namespace rang;
#define Row 13
#define col 45
#define STEP 0.2
#define G 0.5

//Data Structures
enum LevelState
{
    RUNNING,
    WIN,
    LOSE,
};
struct Position //مختصات
{
    int x;
    int y;
};
struct Player //پلیر
{
    Position pos;
    char symbol = 'P';
};
struct Enemy
{
    Position pos;
    char symbol = 'E';
    bool alive = true;
};
struct Wall
{
    Position pos;
    char symbol = '#';
    bool standing = true;
};
struct Projectile
{
    Position pos;
    Position v; // سرعت
    bool active=true;
    char symbol = '*';
};
struct Level 
{
    Player player;
    Enemy enemies[5];
    Wall walls[5];
    int enemyCount;
    int WallCount;
    int maxShots=10;
    int shotsFired = 0;
    int parShots;    // شلیک استاندارد برای 3 ستاره
    LevelState State;
};
struct GameProgress
{
    int stars[3]={0,0,0};
    int scores[3]={0,0,0};
    bool unlocked[3]={true,false,false};
};

//متغیرها
GameProgress progress;
char Grid[Row][col];

//توابع
void clearscreen();
void SaveProgressToFile();
void LoadProgressFromFile();
void PlayGround ();
void Ground_print();
void placement_char(int x,int y,char C);
void DrawLevel(Level &lvl);
int Get_Speed();
int Get_Angle();
Position V (int speed,int angle);
void Move_Projectile(Projectile &P_star);
bool IsOutOfBounds(int x, int y);
bool Hit_Enemy(int x, int y);
bool Hit_Wall(int x,int y);
void Draw_Projectile(Projectile &p);
void RunSingleShot(Level &lvl);
void CheckLevelEnd(Level &lvl);
void RunLevel(Level &lvl, int levelIndex);
void Projectile_Update(Projectile &p, Level &lvl);
void Kill_Enemy(Level &lvl, int x, int y);
bool allEnemiesDead(Level &lvl);
void MainMenu();//منوی اصلی بازی 
void LevelMenu();// منوی انتخاب لول بازی
void HowToPlayMenu();
void ProgressStarsMenu();
void Setup_Level_1(Level &lvl);
void Setup_Level_2(Level &lvl);
void Setup_Level_3(Level &lvl);
void Level_1();
void Level_2();
void Level_3();


int main ()
{
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
void PlayGround ()  //زمین خام بازی
{
    for (int i=0;i<Row;i++)
    {   
        for (int j=0;j<col;j++)
        {
            if ( i==0 || i==Row-1 || j==0 || j==col-1 )
                Grid[i][j]='#';
            else
                Grid[i][j]=' ';
        }
    }

}
void Ground_print()
{
    for (int i=0;i<Row;i++)
    {
        for (int j=0;j<col;++j)
            cout<<Grid[i][j];
        cout<<endl;
    }
}
void placement_char(int x,int y,char C)
{
    Grid[y][x]=C;
}

void DrawLevel(Level &lvl)
{
    PlayGround();

    // Player
    placement_char(lvl.player.pos.x,lvl.player.pos.y,lvl.player.symbol);

    // Enemies
    for (int i = 0; i < lvl.enemyCount; i++)
        if (lvl.enemies[i].alive)
            placement_char(lvl.enemies[i].pos.x,lvl.enemies[i].pos.y,lvl.enemies[i].symbol);
    // Walls
    for (int i = 0; i < lvl.WallCount; i++)
        if (lvl.walls[i].standing)
            placement_char(lvl.walls[i].pos.x,lvl.walls[i].pos.y,lvl.walls[i].symbol);
}
void RunSingleShot(Level &lvl)
{
    Projectile p;

    int speed = Get_Speed();
    int angle = Get_Angle();

    p.pos = lvl.player.pos;
    p.v = V(speed, angle);
    p.active = true;

    while (p.active)
    {
        clearscreen();
        DrawLevel(lvl);
        Projectile_Update(p, lvl);
        Ground_print();
        Sleep(30);
    }

    lvl.shotsFired++;
    CheckLevelEnd(lvl);
}
void CheckLevelEnd(Level &lvl)
{
    if (allEnemiesDead(lvl))
    {
        lvl.State = WIN;
    }
    else if (lvl.shotsFired >= lvl.maxShots)
    {
        lvl.State = LOSE;
    }
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

    if (lvl.State == WIN)
    {
        cout << "YOU WIN!\n";

        // محاسبه ستاره   
        int stars = 1;
        if (lvl.shotsFired <= lvl.parShots)
            stars = 3;
        else if (lvl.shotsFired <= lvl.parShots + 2)
            stars = 2;

        progress.stars[levelIndex] = stars;
        progress.scores[levelIndex] = (lvl.maxShots - lvl.shotsFired + 1) * 100;

        // باز کردن لول بعدی
        if (levelIndex < 2)
            progress.unlocked[levelIndex + 1] = true;
    }
    else
    {
        cout << "YOU LOSE!\n";
    }

    SaveProgressToFile();
    cout << "\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}

//Projectile
int Get_Speed()
{
    int Speed;
    cout<<"Speed=";
    cin >>Speed;
    if (Speed < 0 || Speed > 12)
    {
        cout << "\n    The number is out of range!    ";
        cout <<"\n         Please enter 1-12.         ";
        cout <<"\n     Press Enter to continue...     ";
        cin.ignore();
        cin.get();
        return Get_Speed();
    }
    return Speed;
}
int Get_Angle()
{
    int Angle;
    cout<<"Angle=";
    cin >>Angle;
    if (Angle<=0 || Angle>=90)
    {
        cout << "\n    The number is out of range!    ";
        cout <<"\n         Please enter 0-90.         ";
        cout <<"\n     Press Enter to continue...     ";
        cin.ignore();
        cin.get();
        return Get_Angle();
    }
    return Angle;
}
Position V(int speed, int angle)
{
    Position v;
    double rad = angle * M_PI / 180.0;   //M_PI عدد پی است
    v.x = speed * cos(rad);
    v.y = -speed * sin(rad); // منفی چون محور y رو به پایین است
    return v;
}
void Move_Projectile(Projectile &P) 
{
    P.pos.x+=(P.v.x * STEP);
    P.pos.y+=(P.v.y * STEP);
    P.v.y +=(G * STEP);
}
bool IsOutOfBounds(int x, int y)
{
    return (x < 0 || x >= col || y < 0 || y >= Row);
}
bool Hit_Enemy(int x, int y)
{
    return Grid[y][x] == 'E';
}
bool Hit_Wall(int x,int y)
{
    return Grid[y][x] == '#';
}
void Draw_Projectile(Projectile &p)
{
    placement_char((int)p.pos.x, (int)p.pos.y, p.symbol);
}
void Projectile_Update(Projectile &p, Level &lvl)
{
    if (!p.active)
        return;

    Move_Projectile(p);

    int x = (int)p.pos.x;
    int y = (int)p.pos.y;

    if (IsOutOfBounds(x, y))
    {
        p.active = false;
        return;
    }
    if (Hit_Wall(x, y))
    {
        p.active = false;
        return;
    }
    if (Hit_Enemy(x, y))
    {
        p.active = false;
        Kill_Enemy(lvl, x, y);
        return;
    }
    Draw_Projectile(p);
}

//Enemy
void Kill_Enemy(Level &lvl, int x, int y)
{
    for (int i = 0; i < lvl.enemyCount; i++)
    {
        if (lvl.enemies[i].alive && lvl.enemies[i].pos.x == x && lvl.enemies[i].pos.y == y)
        {
            lvl.enemies[i].alive = false;
            Grid[y][x] = ' ';
            break;
        }
    }
}
bool allEnemiesDead(Level &lvl)
{
    for(int i = 0; i < lvl.enemyCount; i++)
    {
        if(lvl.enemies[i].alive)
            return false;
    }
    return true;
}


//Menus
void MainMenu()//منوی اصلی بازی 
{
    clearscreen();
    int choice;
    cout << "===================================\n";//35
    cout << "        ANGRY BIRDS CONSOLE        \n";
    cout << "===================================\n";
    cout << "1. Start                           \n";
    cout << "2. How to Play                     \n";
    cout << "3. Progress / Stars                \n";
    cout << "4. Exit                          \n\n";
    cout << "===================================\n";
    cout << "Your Choice:";
    cin>>choice;
    switch(choice)
    {
        case 1:
            LevelMenu(); 
            break;
        case 2:
            HowToPlayMenu();  
            break;
        case 3:
            ProgressStarsMenu();
            break;
        case 4:
            exit(0);
            break;
        default:
            cout << "\nInvalid choice! Please enter 1-4.\n";
            cout << "Press Enter to continue...";
            cin.ignore();
            cin.get();
            MainMenu();
    }
}
void LevelMenu()// منوی انتخاب لول بازی
{
    clearscreen();
    int choice;
    cout << "===================================\n";
    cout << "           CHOOSE  LEVEL           \n";
    cout << "===================================\n";
    cout << "1. Level 1 ";
    cout << "[Unlocked]\n";
    cout << "2. Level 2 ";
    if (progress.unlocked[1])
        cout << "[Unlocked]\n";
    else
        cout << "[LOCKED]\n";
    cout << "3. Level 3 ";
    if (progress.unlocked[2])
        cout << "[Unlocked]\n";
    else
        cout << "[LOCKED]\n";
    cout << "4. Back to Main Menu";
    cout << "\n===================================\n";
    cout << "Your Choice: ";
    cin >> choice;
     switch(choice)
     {
        case 1:
            Level_1();
            break;
        case 2:
            Level_2();  
            break;
        case 3:
            Level_3();
            break;
        case 4:
             MainMenu();
             break;
         default:
             cout << "\nInvalid choice! Please enter 1-4.\n";
             cout << "Press Enter to continue...";
             cin.ignore();
             cin.get();
             LevelMenu();
    }
}
void HowToPlayMenu()  // این تابع رو باید دوباره بعد از ساخته شدن برنامه برسی و تصحیح کرد.
{
    clearscreen();
    int page = 1;
    char choice;
    
    do {
        clearscreen();
        cout << "===================================\n";
        cout << "            HOW TO PLAY            \n";
        cout << "===================================\n";
        
        if(page == 1) {
            cout << "Page 1/3 - Controls:\n";
            cout << "===================================\n";
            cout << "1. Enter Angle (0-90 degrees)\n";
            cout << "2. Enter Speed (1-12)\n";
            cout << "3. Press Enter to simulate each step\n";
            cout << "4. Press Q to quit level\n";
        }
        else if(page == 2) {
            cout << "Page 2/3 - Game Rules:\n";
            cout << "===================================\n";
            cout << "- Destroy all enemies (E)\n";
            cout << "- Avoid hitting walls (#)\n";
            cout << "- Limited shots per level\n";
            cout << "- Get 3 stars by using fewer shots\n";
        }
        else if(page == 3) {
            cout << "Page 3/3 - Special Shots:\n";
            cout << "===================================\n";
            cout << "1. Explosive: Destroys nearby targets\n";
            cout << "2. Bouncy: Bounces 3 times off walls\n";
            cout << "3. Dropper: Drops straight down\n";
            cout << "4. Boomerang: Returns after distance\n";
        }
        
        cout << "\n===================================\n";
        cout << "N - Next Page  |  P - Previous Page\n";
        cout << "B - Back to Menu\n";
        cout << "===================================\n";
        cout << "Your Choice: ";
        cin >> choice;
        
        if((choice == 'N' || choice == 'n') && page < 3)
            page++;
        else if((choice == 'P' || choice == 'p') && page > 1)
            page--;
        else if(choice == 'B' || choice == 'b')
            MainMenu();
        
    } while(true);

}
void ProgressStarsMenu()
{
    char choice;
    
    do {
        clearscreen();
        cout << "===================================\n";
        cout << "          PROGRESS & STARS         \n";
        cout << "===================================\n\n";
        
        for(int i = 0; i < 3; i++)
        {
            cout << "Level " << i+1 << ": ";
            if(progress.unlocked[i])
            {
                // نمایش ستاره‌ها
                cout << "[";
                for(int j=0;j<3;j++)
                {
                    if(j<progress.stars[i])
                        cout << "*";
                    else
                        break;
                }
                cout << "] ";
                cout << "Score: " << progress.scores[i];
                
                if(i == 0 && progress.stars[i] == 0)
                    cout << " (Not completed)";
                else if(progress.stars[i] == 3)
                    cout << "PERFECT!";
                    
                cout << endl;
            }
            else
            {
                cout << "LOCKED (Complete Level " << i << " first)" << endl;
            }
        }
        
        cout << "\n===================================\n";
        cout << "1. Reset Progress\n";
        cout << "2. Back to Main Menu\n";
        cout << "===================================\n";
        cout << "Your Choice: ";
        cin >> choice;
        
        if(choice == '1')
        {
            for(int i=0;i<3;i++)
            {
                progress.stars[i] = 0;
                progress.scores[i] = 0;
                progress.unlocked[i] = (i == 0); // فقط لول ۱ باز باشد
            }
            cout << "\nProgress reset! (Press Enter)";
            cin.ignore();
            cin.get();
        }
        else if(choice == '2')
        {
            MainMenu();
            return;
        }
        
    } while(true);
}

void Setup_Level_1(Level &lvl)
{
    lvl.State = RUNNING;
    lvl.shotsFired = 0;
    lvl.maxShots = 10;
    lvl.parShots = 3;

    lvl.player.pos = {2, Row - 2};

    lvl.enemyCount = 3;
    lvl.enemies[0] = {{20,2}, 'E', true};
    lvl.enemies[1] = {{20,4}, 'E', true};
    lvl.enemies[2] = {{20,6}, 'E', true};

    lvl.WallCount = 3;
    lvl.walls[0] = {{20,3}, '#', true};
    lvl.walls[1] = {{20,5}, '#', true};
    lvl.walls[2] = {{20,7}, '#', true};
}
void Setup_Level_2(Level &lvl)
{
    lvl.State = RUNNING;
    lvl.shotsFired = 0;
    lvl.maxShots = 10;
    lvl.parShots = 3;

    lvl.player.pos = {2, Row - 2};

    lvl.enemyCount = 3;
    lvl.enemies[0] = {{22,2}, 'E', true};
    lvl.enemies[1] = {{20,6}, 'E', true};
    lvl.enemies[2] = {{24,9}, 'E', true};

    lvl.WallCount = 3; 
    lvl.walls[0] = {{22,3}, '#', true};
    lvl.walls[1] = {{20,7}, '#', true};
    lvl.walls[2] = {{24,10}, '#', true};
}
void Setup_Level_3(Level &lvl)
{
    lvl.State = RUNNING;
    lvl.shotsFired = 0;
    lvl.maxShots = 12;
    lvl.parShots = 4;
    
    lvl.player.pos = {2, Row - 2};
    
    lvl.enemyCount = 4;
    lvl.enemies[0] = {{00,00}, 'E', true};
    lvl.enemies[1] = {{00,00}, 'E', true};
    lvl.enemies[2] = {{00,00}, 'E', true};
    lvl.enemies[3] = {{00,00}, 'E', true};

    lvl.WallCount = 5;
    lvl.walls[0] = {{00,00}, '#', true};
    lvl.walls[1] = {{00,00}, '#', true};
    lvl.walls[2] = {{00,00}, '#', true};
    lvl.walls[3] = {{00,00}, '#', true};
    lvl.walls[4] = {{00,00}, '#', true};
}


void Level_1 ()
{
    Level lev_1;
    clearscreen();
    Setup_Level_1(lev_1);
    RunLevel(lev_1, 0);
}
void Level_2()
{
    Level lev_2;
    clearscreen();
    Setup_Level_2(lev_2);
    RunLevel(lev_2, 1);
}
void Level_3()
{
    Level lev_3;
    clearscreen();
    Setup_Level_3(lev_3);
    RunLevel(lev_3, 2);
}
