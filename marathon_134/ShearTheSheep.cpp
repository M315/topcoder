// C++11
#include <bits/stdc++.h>
#include <time.h>
#include <iostream>
#include <fstream> 

using namespace std;

//ofstream qf;

int N, C;
int bx,by;
static const int dx[] = {1, 0, -1, 0, 0};
static const int dy[] = {0, 1, 0, -1, 0};
static const char dir[] = {'R', 'D', 'L', 'U', 'N'};

struct sheep {
    int x;
    int y;
};

struct farmer {
    int x;
    int y;
    int space;
    vector<int> move;
};

char grid[32][32];

vector<farmer> farmers;
vector<sheep> sheeps;

static const int population_size = 20;
static const int max_depth = 10;

struct ind {
    char g[32][32];
    vector<farmer> f;
    vector<sheep> s;
    int d;
    int newWool;
    int collected;
};

vector<ind> parents;
vector<ind> children;

// A* to go home?
double score(ind a)
{
    return a.newWool + a.collected;
}

vector<int> validMoves(int x, int y)
{
    vector<int> moves;

    for(int i = 0; i < 4; i++) {
        int nx = x + dx[i]; 
        int ny = y + dy[i];

        if(nx >= 0 && nx < N && ny >=0 && ny < N && grid[nx][ny] != '#' && grid[nx][ny] != 'A' && !(grid[nx][ny] >= 'a' && grid[nx][ny] <= 'z'))
        {
            moves.push_back(i);
        }
    }

    moves.push_back(4);

    return moves;
}

int randomMove(int x, int y)
{
    vector<int> moves = validMoves(x, y);

    return moves[rand() % moves.size()];
}

int getMove(farmer f)
{
    return randomMove(f.x, f.y);
}

ind simulate(ind ori)
{
    ind a = ori;
    a.d++;

    // Farmers
    for(int i = 0; i < a.f.size(); i++)
    {
        // Choose a move
        int move = getMove(a.f[i]);
        a.f[i].move.push_back(move);
        
        int nx = a.f[i].x + dx[move];
        int ny = a.f[i].y + dy[move];

        if(a.g[nx][ny] > 'A' && a.g[nx][ny] <= 'D' && a.f[i].space > 0)
        {
            a.f[i].space--;
            a.g[nx][ny]--;

            a.newWool++;
        } else if(a.g[nx][ny] == 'X')
        {
            a.collected += C - a.f[i].space;
            a.f[i].space = C;
        } else
        {
            a.g[a.f[i].x][a.f[i].y] = '.';
            a.g[nx][ny] = ori.g[a.f[i].x][a.f[i].y];

            a.f[i].x = nx;
            a.f[i].y = ny;
        }
    }

    // Sheeps
    for(int i = 0; i < a.s.size(); i++)
    {
        int x = a.s[i].x;
        int y = a.s[i].y;

        // If sheep have been Sheared dont move
        if(a.g[x][y] != ori.g[x][y])
        {
            continue;
        }

        // Choose a move
        int move = randomMove(x, y);

        int nx = x + dx[move];
        int ny = y + dy[move];

        if(a.g[nx][ny] < 'A' && a.g[nx][ny] > 'D')
        {
            a.g[nx][ny] = a.g[x][y];
            a.g[x][y] = '.';
        }
    }

    return a;
}

/*
void print_farmer(farmer f)
{
    qf << "Pos: " << f.x << " " << f.y << endl;
    qf << "Space: " << f.space << endl;

    qf << "Moves: ";
    for(int i = 0; i < f.move.size(); i++)
        qf << dir[f.move[i]] << " - ";
    qf << endl;
}

void print_sheep(sheep s)
{
    qf << "Pos: " << s.x << " " << s.y << endl;
}

void print_individual(ind a)
{
    qf << "\n Step: " << a.d << endl;

    qf << "Grid:" << endl;
    for(int j = 0; j < 10; j++)
    {
        for(int i = 0; i < 10; i++)
            qf << a.g[i][j] << " ";
        qf << endl;
    }

    qf << "\nFarmers: " << endl;
    for(int i = 0; i < a.f.size(); i++)
        print_farmer(a.f[i]);

    qf << "\nSheeps: " << endl;
    for(int i = 0; i < a.s.size(); i++)
        print_sheep(a.s[i]);

    qf << "Wool: " << a.newWool << endl;
    qf << "Collected: " << a.collected << endl;
}
*/

void makemoves()
{
    stringstream ss;

    // Initialize population
    vector<ind> population;

    for(int k = 0; k < population_size; k++)
    {
        ind a;

        for(int i = 0; i < 32; i++)
            for(int j = 0; j < 32; j++)
                a.g[i][j] = grid[i][j];
        a.f = farmers;
        a.s = sheeps;
        a.d = 0;
        a.newWool = 0;
        a.collected = 0;

        population.push_back(a);
    }

    // Simulate each individual

    for(int k = 0; k < population_size; k++)
    {
        while(population[k].d < max_depth)
        {
            population[k] = simulate(population[k]);
        }
    }
    
    // Pick best
    ind best;
    int max_score = -1;

    for(int k = 0; k < population_size; k++)
    {
        int s = score(population[k]);
        if(max_score < s)
        {
            best = population[k];
            max_score = s;
        }
    }

    for (int i = 0; i < farmers.size(); i++)
    {
        ss << farmers[i].y << " " << farmers[i].x << " "; 
        
        ss << dir[best.f[i].move[0]] << " ";
    }

    cout << ss.str() << endl;
    cout.flush();
}

void readgrid()
{
    farmers.clear();
    sheeps.clear();
    for (int y=0;y<N;y++)
    {
        for (int x=0;x<N;x++)
        {
            char ch;
            cin >> ch;
            grid[x][y] = ch;
            if (ch>='a' && ch<='z')
            {
                farmer f;
                f.x = x;
                f.y = y;
                f.space = 2;//C - (ch - 'a');

                farmers.push_back(f);
            }
            if (ch=='X')
            {
                bx = x;
                by = y;
            }
            if (ch >= 'A' && ch <= 'E')
            {
                sheep s;
                s.x = x;
                s.y = y;

                sheeps.push_back(s);
            }
        }
    }
}

int main() 
{
    srand(time(NULL));

    //qf.open("logs.txt");

    cin >> N >> C;

    readgrid();
    makemoves();

    for (int i=0;i<1000;i++)
    {
        int tm;
        cin >> tm;
        readgrid();
        makemoves();    
    }

    //qf.close();

    return 0;
}
