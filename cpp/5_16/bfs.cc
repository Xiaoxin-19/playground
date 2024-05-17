#include <iostream>
#include <cstdio>
#include <queue>
#include <string>
#include <set>
#include <vector>
using namespace std;

vector<string> board;
vector<vector<int>> visited;
vector<int> ans;
int s, n, m;
struct point
{
  int x;
  int y;
  int step;
  point(int x, int y) : x(x), y(y), step(0) {}
  point(int x, int y, int s) : x(x), y(y), step(s) {}
};

bool operator==(const point &lhs, const point &rhs)
{
  return (lhs.x == rhs.x && lhs.y == rhs.y);
}

int knight_x, knight_y, princess_x, princess_y;

bool valid(point p)
{
  if (p.x == 0 || p.y == 0 || p.x == n || p.y == m)
    return false;
  if (board[p.x][p.y] == '#')
    return false;
  if (visited[p.x][p.y] != -1)
    return false;
  return true;
}

int BFS()
{
  queue<point> q;
  visited.clear();
  visited.resize(board.size(), vector<int>(board[0].size(), -1));
  int min_step = 1e9;
  point start(knight_x, knight_y);
  point target(princess_x, princess_y);

  q.push(start);
  visited[start.x][start.y] = 0;

  while (!q.empty())
  {
    int sz = q.size();
    for (int i = 0; i < sz; i++)
    {
      point cur = q.front();
      q.pop();

      if (cur == target)
      {
        // cout << "find princess!!" << endl;
        min_step = min(min_step, cur.step);
      }

      int dirs[4][2]{{1, 0}, {-1, 0}, {0, -1}, {0, 1}};
      for (auto dir : dirs)
      {
        point x(cur.x + dir[0], cur.y + dir[1], cur.step);
        if (valid(x))
        {
          x.step += 1;
          if (board[x.x][x.y] == 'x')
          {
            x.step += 1;
          }
          visited[x.x][x.y] = 0;
          q.push(x);
        }
      }
    }
  }
  return min_step;
}
// 如果走到这里，说明在图中没有找到目标节点
// 公主被恶人抓走，被关押在牢房的某个地方。牢房用N*M (N, M <= 200)的矩阵来表示。矩阵中的每项可以代表道路（@）、墙壁（#）、和守卫（x）。
// 英勇的骑士（r）决定孤身一人去拯救公主（a）。我们假设拯救成功的表示是“骑士到达了公主所在的位置”。由于在通往公主所在位置的道路中可能遇到守卫，骑士一旦遇到守卫，必须杀死守卫才能继续前进。
// 现假设骑士可以向上、下、左、右四个方向移动，每移动一个位置需要1个单位时间，杀死一个守卫需要花费额外的1个单位时间。同时假设骑士足够强壮，有能力杀死所有的守卫。

// 给定牢房矩阵，公主、骑士和守卫在矩阵中的位置，请你计算拯救行动成功需要花费最短时间。
int main()
{
  scanf("%d", &s);
  for (int i = 0; i < s; ++i)
  {
    scanf("%d %d", &n, &m);
    board.clear();
    board.resize(n + 1, string(m + 1, '*'));

    for (int row = 1; row <= n; row++)
    {
      for (int col = 1; col <= m; col++)
      {
        char c;
        cin >> c;
        if (c == 'r')
        {
          knight_x = row;
          knight_y = col;
        }
        else if (c == 'a')
        {
          princess_x = row;
          princess_y = col;
        }
        board[row][col] = c;
      }
    }
    int ret = BFS();
    ans.push_back(ret);
  }
  
  for (auto item : ans)
  {
    if (item == 1e9)
      cout << "Poor ANGEL has to stay in the prison all his life" << endl;
    else
      cout << item << endl;
  }
  return 0;
}