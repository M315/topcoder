// C++11
#include <bits/stdc++.h>
#include <time.h>
#include <iostream>
#include <fstream> 
#include <random>

using namespace std;

//ofstream qf; 

int N, C;
static const int dx[] = {1, 0, -1, 0, 0};
static const int dy[] = {0, 1, 0, -1, 0};
static const char dir[] = {'R', 'D', 'L', 'U', 'N'}; 

static const double growthP = 0.05;
static const int K = 3;

int population_size, max_depth, randP;

double lambda[4] = {32., 1.75, 1.25, 1.};

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

// ####################### Greedy #############################

// Returns the best move to make a farmer go to a give position
char goPos(int i, int x, int y) 
{
    char move = 'N';

    int bestDist = N * N;
    for (int j=0;j<4;j++)
    {
        int nx = farmers[i].x + dx[j];
        int ny = farmers[i].y + dy[j];
        int dist = abs(x-nx) + abs(y-ny);
        if (dist<bestDist && nx>=0 && nx<N && ny>=0 && ny<N && grid[nx][ny]!='#')
        {
            move = dir[j];
            bestDist = dist;
        } else if (dist ==  bestDist && nx >= 0 && nx < N && ny >= 0 && ny < N && grid[nx][ny] != '#' && rand() % 2 == 0)
        {
            move = dir[j];
            bestDist = dist;
        }
    }

    return move;
}

// Find the nearest sheep from the farmer
sheep findNearestSheep(int i)
{
    int best_index = 0;
    int best_dist = 100000;

    for(int k = 0; k < sheeps.size(); k++)
    {
        int x = sheeps[k].x;
        int y = sheeps[k].y;

        int dist = abs(x - farmers[i].x) + abs(y - farmers[i].y);
        if(dist < best_dist && sheeps[k].wool > 0)
        {
            best_index = k;
            best_dist = dist;
        } else if (dist == best_dist && sheeps[k].wool > 0 && rand() % 2 == 0)
        {
            best_index = k;
            best_dist = dist;
        }
    }

    return sheeps[best_index];
}

// Find the nearest brane from the farmer
brane findNearestBrane(int i)
{
    int best_index = 0;
    int best_dist = 100000;

    for(int k = 0; k < branes.size(); k++)
    {
        int x = branes[k].x;
        int y = branes[k].y;

        int dist = abs(x - farmers[i].x) + abs(y - farmers[i].y);
        if(dist < best_dist)
        {
            best_index = k;
            best_dist = dist;
        } else if (dist == best_dist && rand() % 2 == 0)
        {
            best_index = k;
            best_dist = dist;
        }
    }

    return branes[best_index];
}

void makeGreedyMoves()
{
    stringstream ss;

    for (int i = 0; i < farmers.size(); i++)
    {
        ss << farmers[i].y << " " << farmers[i].x << " "; 
        if (farmers[i].space == 0)
        {
            brane b = findNearestBrane(i);
            ss << goPos(i, b.x, b.y) << " ";     
        } else
        {
            sheep s = findNearestSheep(i);
            ss << goPos(i, s.x, s.y) << " ";
        }
    }

    cout << ss.str() << endl;
    cout.flush();
}

// ####################### Evolutionary approach #######################

double distanceNearestBrane(int x, int y)
{
    double dist = double(N * N);

    for(int k = 0; k < branes.size(); k++)
    {
        double d = double(abs(x - branes[k].x) + abs(y - branes[k].y));
        if(d < dist)
            dist = d;
    }
    
    return dist;
}

double distanceSheep(int x, int y, int j, ind a)
{

    return lambda[a.g[a.s[j].x][a.s[j].y] - 'A'] * double(abs(x - a.s[j].x) + abs(y - a.s[j].y));
}

double score(ind a)
{
    double s = double(a.newWool) + double(a.collected);

    for(int i = 0; i < a.f.size(); i++)
    {
        double need_home = double(C - a.f[i].space) / double(C);
        need_home = need_home * need_home * need_home;

        s -=  0.25 * need_home * distanceNearestBrane(a.f[i].x, a.f[i].y);
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

    if(moves.size() == 0)
        moves.push_back(4);

    return moves;
}

int randomSheepMove(int i, ind a)
{
    vector<int> moves = validMoves(a.s[i].x, a.s[i].y, a.g);

    return moves[rand() % moves.size()];
}

int randomMove(int i, ind a)
{
    vector<int> moves = validMoves(a.f[i].x, a.f[i].y, a.g);

    return moves[rand() % moves.size()];
}

int greedy(int i, ind a)
{
    vector<int> moves = validMoves(a.f[i].x, a.f[i].y, a.g);

    if(a.f[i].space == 0)
    {
        int best_index = 0;
        double best = double(N * N * N);

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

    int best_index = 0;
    double best = double(N * N * N);

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
            double dist = distanceSheep(nx, ny, j, a);
            if(dist <= best)
            {
                best = dist;
                best_index = k;
            }
        }
    }

    return moves[best_index];
}

int getMove(int i, ind a)
{
    if(rand() % 100 < randP)
        return randomMove(i, a);

    return greedy(i, a);
}

ind simulate(ind ori)
{
    ind a = ori;
    a.d++;

    // Farmers
    for(int i = 0; i < a.f.size(); i++)
    {
        // Choose a move
        int move = getMove(i, a);

        a.f[i].move.push_back(move);

        if(a.d == max_depth)
            continue;
        
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
    for(int i = 0; i < a.s.size() && a.d < max_depth; i++)
    {
        int x = a.s[i].x;
        int y = a.s[i].y;

        // If sheep have been Sheared dont move
        if(a.g[x][y] != ori.g[x][y])
            continue;

        // Make wool grow
        if(a.g[x][y] >= 'A' && a.g[x][y] < 'D')
        {
            double num = 1;//distribution(generator);
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
    for(int k = 0; k < population_size; k++)
        while(population[k].d < max_depth)
            population[k] = simulate(population[k]);
    
    // Pick best
    ind best = population[0];
    double max_score = score(population[0]);

    for(int k = 1; k < population_size; k++)
    {
        double s = score(population[k]);
        if(s >= max_score)
        {
            best = population[k];
            max_score = s;
        }
    }

    /*
    print_individual(best);
    qf << farmers[0].y << " " << farmers[0].x << " "; 
    qf << dir[best.f[0].move[0]] << " " << endl;
    */

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
    branes.clear();
    farmers.clear();
    sheeps.clear();

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

    //qf.open("logs.txt");

    cin >> N >> C;

    population_size = (10 * (1 + int(20 - (2 * int(log2(double(N * N))))))) / 7;
    max_depth = (10 * (1 + int(20 - (2 * int(log2(double(N * N))))))) / 7;

    randP = max(2, ((population_size * max_depth) / 6));

    for(int i = 0; i < N; i++)
    {
        vector<char> g (N, '.');
        grid.push_back(g);
    }
    
    readgrid();
    makemoves();

    for (int i=0;i<1000;i++)
    {
        //qf << "### " << i << " ###" << endl;
        int tm;
        cin >> tm;
        readgrid();
        if(tm > 9800)
        {
            makeGreedyMoves();
        } else
        {
            makemoves();    
        }
    }

    //qf.close();

    return 0;
}
