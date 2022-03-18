// C++11
#include <bits/stdc++.h>

using namespace std;

char grid[32][32];
int N, C;
vector<int> fx, fy, fv;
int bx,by;
static const int dx[] = {1,0,-1,0};
static const int dy[] = {0,1,0,-1};
static const char dir[] = {'R','D','L','U'};

void makemoves()
{
    stringstream ss;
    for (int i=0;i<fx.size();i++)
    {
        ss << fy[i] << " " << fx[i] << " "; 
        if (fv[i]>0)
        {
            // go home
            char move = 'N';
            int bestDist = 10000;
            for (int j=0;j<4;j++)
            {
                int nx = fx[i]+dx[j];
                int ny = fy[i]+dy[j];
                int dist = abs(bx-nx) + abs(by-ny);
                if (dist<bestDist && nx>=0 && nx<N && ny>=0 && ny<N && grid[nx][ny]!='#')
                {
                    move = dir[j];
                    bestDist = dist;
                }
            }        
            ss << move << " ";     
        } else
        {
            bool bFound = false;
            for (int j=0;j<4;j++)
            {
                int nx = fx[i]+dx[j];
                int ny = fy[i]+dy[j];
                if (nx>=0 && nx<N && ny>=0 && ny<N && (grid[nx][ny]=='.' || (grid[nx][ny]>='A' && grid[nx][ny]<'E')))
                {
                    ss << dir[j] << " ";
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
                ss << "N ";
        }
    }
    cout << ss.str() << endl;
    cout.flush();
}

void readgrid()
{
    fx.clear();
    fy.clear();
    fv.clear();
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
        }
    }
}

int main() 
{
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
    return 0;
}