// C++11
#include <bits/stdc++.h>
#include <time.h>
#include <iostream>
#include <fstream> 
#include <random>

using namespace std;

ofstream qf; int N, C;
static const int dx[] = {1, 0, -1, 0, 0};
static const int dy[] = {0, 1, 0, -1, 0};
static const char dir[] = {'R', 'D', 'L', 'U', 'N'}; 

static const double growthP = 0.07;
static const int K = 3;

static const int population_size = 20;
static const int max_depth = 10;

default_random_engine generator;
uniform_real_distribution<double> distribution(0.0,1.0);

struct brane {
    int x;
    int y;
};

struct sheep {
    int x;
    int y;
    int wool;
};

struct farmer {
    int x;
    int y;
    int space;
    vector<int> move;
};

vector<vector<char>> grid;
vector<brane> branes;
vector<farmer> farmers;
vector<sheep> sheeps;

struct ind {
    vector<vector<char>> g;
    vector<farmer> f;
    vector<sheep> s;
    int d;
    int newWool;
    int collected;
};

vector<ind> parents;
vector<ind> children;

// A* to go home?
double distanceNearestBrane(int x, int y)
{
    double dist = double(N * N);

    for(int k = 0; k < branes.size(); k++)
    {
        double d = abs(x - branes[k].x) + abs(y - branes[k].y);
        if(d < dist)
            dist = d;
    }
    
    return dist;
}

double score(ind a)
{
    double s = double(a.newWool) + double(a.collected);

    for(int i = 0; i < a.f.size(); i++)
    {
        double need_home = double(C - a.f[i].space) / double(C);
        need_home = need_home * need_home * need_home;
        s -=  need_home * distanceNearestBrane(a.f[i].x, a.f[i].y);
    }

    return s;
}

vector<int> validMoves(int x, int y, vector<vector<char>> g)
{
    vector<int> moves;

    for(int i = 0; i < 4; i++) {
        int nx = x + dx[i]; 
        int ny = y + dy[i];

        if(nx >= 0 && nx < N && ny >=0 && ny < N && g[nx][ny] != '#' && g[nx][ny] != 'A' && !(g[nx][ny] >= 'a' && g[nx][ny] <= 'z'))
        {
            moves.push_back(i);
        }
    }

    moves.push_back(4);

    return moves;
}

int randomSheepMove(int i, ind a)
{
    vector<int> moves = validMoves(a.s[i].x, a.s[i].y, a.g);

    return moves[rand() % moves.size()];
}

int randomMove(vector<int> moves, vector<double> p)
{
    double num = distribution(generator);
    int n = moves.size();

    for(int i = 0; i < n; i++)
        if(num <= p[i])
            return moves[i];
    
    return moves[n - 1];
}

double closeness(int x1, int y1, int x2, int y2)
{
    double dist = double(abs(x1 - x2) + abs(y1 - y2));
    return 100. / (dist * dist);
}

vector<double> heuristic(int index, ind a, vector<int> moves)
{
    farmer f = a.f[index];
    // Go towards a sheeps with wool but try keep close to home

    // As more close to full more probability to go home
    double need_home = double(C - f.space) / double(C);
    need_home *= need_home;

    int n = moves.size();
    vector<double> p (n, 0.0);

    double total = 0.;
    for(int k = 0; k < n; k++)
    {
        int nx = f.x + dx[moves[k]];
        int ny = f.y + dy[moves[k]];

        if(a.g[nx][ny] == 'X')
        {
            nx = f.x;
            ny = f.y;
            p[k] += 300. * need_home;
        }

        if(a.g[nx][ny] > 'A' && a.g[nx][ny] <= 'D')
        {
            p[k] += 200. * double(a.g[nx][ny] - 'A') * (1 - need_home);
            nx = f.x;
            ny = f.y;
        }

        // Sheeps
        for(int i = 0; i < a.s.size(); i++)
        {
            p[k] += max((1. - need_home) * (double(a.s[i].wool) / 3.) * closeness(nx, ny, a.s[i].x, a.s[i].y), 0.);
        }

        // Home
        for(int i = 0; i < branes.size(); i++)
        {
            p[k] += max(need_home * closeness(nx, ny, branes[k].x, branes[k].y), 0.);
        }

        // Farmers
        for(int i = 0; i < a.f.size(); i++)
        {
            if(i == index)
                continue;

            p[k] -= max(0.25 * (1. - need_home) * closeness(nx, ny, a.f[i].x, a.f[i].y), 0.);
        }

        if(moves[k] == 4)
            p[k] *= 0.5;

        p[k] = max(p[k], 0.0);
        total += p[k];
    }

    /*
    qf << "Probability:" << endl;
    for(int k = 0; k < n; k++)
        qf << dir[moves[k]] << " " << p[k] << endl;
    qf << endl;
    */

    // Normalize the probability vector
    for(int k = 0; k < n; k++)
        p[k] /= total;

    return p;
}

int getMove(int i, ind a)
{
    vector<int> moves = validMoves(a.f[i].x, a.f[i].y, a.g);
    vector<double> p = heuristic(i, a, moves);

    return randomMove(moves, p);
}

int greedy(int i, ind a)
{
    vector<int> moves = validMoves(a.f[i].x, a.f[i].y, a.g);

    if(a.f[i].space == 0)
    {
        int best_index = 4;
        int best = N * N * N;

        for(int k = 0; k < moves.size(); k++)
        {
            int nx = a.f[i].x + dx[moves[k]];
            int ny = a.f[i].y + dy[moves[k]];

            if(a.g[nx][ny] >= 'A' && a.g[nx][ny] <= 'D')
            {
                nx = a.f[i].x;
                ny = a.f[i].y;
            }

            double dist = distanceNearestBrane(nx, ny);
            if(dist < best)
            {
                best = dist;
                best_index = k;
            }
        }

        return moves[best_index];
    }

    int best_index = 4;
    int best = N * N * N;

    for(int k = 0; k < moves.size(); k++)
    {
        int nx = a.f[i].x + dx[moves[k]];
        int ny = a.f[i].y + dy[moves[k]];

        if(a.g[nx][ny] == 'X')
        {
            nx = a.f[i].x;
            ny = a.f[i].y;
        }

        for(int j = 0; j < a.s.size(); j++)
        {
            if(abs(nx - a.s[j].x) + abs(ny - a.s[j].y) <= best)
            {
                best = abs(nx - a.s[j].x) + abs(ny - a.s[j].y);
                best_index = k;
            }
        }
    }

    return moves[best_index];
}

ind simulate(ind ori, bool greed)
{
    ind a = ori;
    a.d++;

    // Farmers
    for(int i = 0; i < a.f.size(); i++)
    {
        int move = 4;
        // Choose a move
        if(greed)
        {
            qf << "####   " << a.f[i].x << " " << a.f[i].y << endl;
            move = greedy(i, a);
            qf << move << endl;
        } else 
        {
            move = getMove(i, a);
        }

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
            continue;

        // Make wool grow
        if(a.g[x][y] >= 'A' && a.g[x][y] < 'D')
        {
            double num = distribution(generator);
            if(num <= growthP)
                a.g[x][y]++;
        }

        // Choose a move
        int move = randomSheepMove(i, a);

        int nx = x + dx[move];
        int ny = y + dy[move];

        if(a.g[nx][ny] == '.')
        {
            a.g[nx][ny] = a.g[x][y];
            a.g[x][y] = '.';

            a.s[i].x = nx;
            a.s[i].y = ny;
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

        a.g = grid;
        a.f = farmers;
        a.s = sheeps;
        a.d = 0;
        a.newWool = 0;
        a.collected = 0;

        population.push_back(a);
    }

    // Simulate each individual
    population[0] = simulate(population[0], true);
    for(int k = 1; k < population_size; k++)
        while(population[k].d < max_depth)
            population[k] = simulate(population[k], false);
    
    // Pick best
    ind best = population[0];
    double max_score = -1;

    for(int k = 0; k < population_size; k++)
    {
        double s = score(population[k]);
        if(s >= max_score)
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
    grid.clear();
    branes.clear();
    farmers.clear();
    sheeps.clear();

    for(int i = 0; i < N; i++)
    {
        vector<char> g (N);
        grid.push_back(g);
    }

    for (int y=0;y<N;y++)
    {
        vector<char> g;
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
                f.space = C - (ch - 'a');

                farmers.push_back(f);
            }
            if (ch=='X')
            {
                brane b;
                b.x = x;
                b.y = y;

                branes.push_back(b);
            }
            if (ch >= 'A' && ch <= 'E')
            {
                sheep s;
                s.x = x;
                s.y = y;
                s.wool = ch - 'A';

                sheeps.push_back(s);
            }
        }
    }
}

int main() 
{
    srand(time(NULL));

    qf.open("logs.txt");

    cin >> N >> C;

    readgrid();
    makemoves();

    for (int i=0;i<1000;i++)
    {
        qf << "### " << i << " ###" << endl;
        int tm;
        cin >> tm;
        readgrid();
        makemoves();    
    }

    qf.close();

    return 0;
}
