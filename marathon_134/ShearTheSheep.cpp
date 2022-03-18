// C++11
#include <bits/stdc++.h>
#include <time.h>
#include <iostream>
#include <fstream>

using namespace std;

struct sheep {
    int x;
    int y;
    int wool;
};

char grid[32][32];
int N, C;
vector<int> fx, fy, fv;
vector<sheep> sheeps;
int bx,by;
static const int dx[] = {1,0,-1,0};
static const int dy[] = {0,1,0,-1};
static const char dir[] = {'R','D','L','U'};

ofstream qf("logs.txt");
int step = 0;

// Returns the best move to make a farmer go to a give position
char goPos(int i, int x, int y) 
{
    char move = 'N';

    int bestDist = N * N;
    for (int j=0;j<4;j++)
    {
        int nx = fx[i]+dx[j];
        int ny = fy[i]+dy[j];
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

        int dist = abs(x-fx[i]) + abs(y-fy[i]);
        qf << x << " " << y << " " << dist << endl;
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

    qf << "! " << sheeps[best_index].x << " " << sheeps[best_index].y << " " << grid[sheeps[best_index].x][sheeps[best_index].y] << endl;


    return sheeps[best_index];
}

// Returns if the farmer is taking wool or not
int isWorking(int i)
{
    for (int j=0;j<4;j++)
    {
        int nx = fx[i]+dx[j];
        int ny = fy[i]+dy[j];
        if (nx>=0 && nx<N && ny>=0 && ny<N && (grid[nx][ny]>='A' && grid[nx][ny]<'E'))
        {
            return j;
        }
    }

    return -1;
}

vector<char> validMoves(int x, int y)
{
    vector<char> moves {'N'};

    for(int i = 0; i < 4; i++)
    {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if(nx >= 0 && nx < N && ny >=0 && ny < N && grid[nx][ny] != '#')
        {
            moves.push_back(dir[i]);
        }
    }

    return moves;
}

char randomMove(int x, int y)
{
    vector<char> moves = validMoves(x, y);

    return moves[rand() % moves.size()];
}

void makemoves()
{
    stringstream ss;

    for (int i=0;i<fx.size();i++)
    {
        ss << fy[i] << " " << fx[i] << " "; 
        if (fv[i] == C)
        {
            ss << goPos(i, bx, by) << " ";     
        } else
        {
            /*
            int work = isWorking(i);
            if(work >= 0)
            {
                ss << dir[work] << " ";
            } else
            {
                sheep s = findNearestSheep(i);
                ss << goPos(i, s.x, s.y) << " ";
            }
            */
            
            sheep s = findNearestSheep(i);
            ss << goPos(i, s.x, s.y) << " ";
        }
    }

    qf << ss.str() << "\n" << endl;
    cout << ss.str() << endl;
    cout.flush();
}

void readgrid()
{
    fx.clear();
    fy.clear();
    fv.clear();
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
                fx.push_back(x);
                fy.push_back(y);
                fv.push_back(ch-'a');
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
                s.wool = ch - 'A';

                sheeps.push_back(s);
            }
        }
    }
}

int main() 
{
    srand(time(NULL));

    cin >> N >> C;

    readgrid();
    makemoves();

    for (int i=0;i<1000;i++)
    {
        step++;
        qf << step << endl;

        int tm;
        cin >> tm;
        readgrid();
        makemoves();    
    }

    qf.close();

    return 0;
}
