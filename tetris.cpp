/*
* @Author: NanoApe
* @Version: 8.1 (DFS)
*/

#include <set>
#include <map>
#include <cmath>
#include <ctime>
#include <queue>
#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

using namespace std;

#define rp(i, n) for (int i = 0; i < n; i++) 
#define rep(i, l, r) for (int i = l; i <= r; i++)
#define dw(i, n) for (int i = n - 1; i >= 0; i--)
#define dow(i, l, r) for (int i = l; i >= r; i--)
#define fi first
#define se second
#define pb push_back
#define mp make_pair
#define clr(x, c) memset(x, c, sizeof(x))
#define all(x) (x).begin(), (x).end()
#define lowbit(x) ((x) & (-(x)))

typedef long long ll;
typedef long double ld;
typedef unsigned long long ull;
typedef pair<int, int> Pii;
typedef pair<ll, int> Pli;
typedef pair<double, int> Pdi;
typedef pair<ull, int> Pui;
typedef pair<pair<int, int>, pair<int, int>> PPP;

#define MAXLAYER 50    // 最多搜索层数，一层代表一回合 // TODO
#define MAXROUND 10000 // 最多回合数，也是允许出现的方块总数，超过此值后结束游戏
#define MAPWIDTH 10    // 场地宽度
#define MAPHEIGHT 22   // 场地高度
#define LIMITHEIGHT 20 // 方块限制最高高度
#define EMPTYLINE ((1 << 0) + (1 << (MAPWIDTH + 1))) // 空行的二进制状态
#define FULLLINE ((1 << (MAPWIDTH + 2)) - 1)         // 满行的二进制状态
// x 的范围是[1,MAPWIDTH]，y 的范围是[1,MAPHEIGHT]，坐标系原点在左下角

int nextType[MAXROUND];                      // 方块序列

int nowGame = 0;                      // 当前搜索分支游戏局面的下标，范围 0~4
int gridInfo[5][MAPHEIGHT + 2] = {0}; // 记录当前局面的场地二进制状态，第二个维度为行
int nowRound[5];                      // 记录当前局面已进行的轮数
int nowPoint[5];                      // 记录当前局面已获得的分数
ull addCommand[5];                    // 记录当前局面的移动序列，用 unsigned long long 储存一个四进制数

#define grid(x, y) ((gridInfo[nowGame][y] >> (x)) & 1)                    // 判断场地上某位置是否被占用
#define setgrid(x, y) (gridInfo[nowGame][y] |= (1 << (x)))                // 占用场地上某位置
#define unsetgrid(x, y) (gridInfo[nowGame][y] &= (FULLLINE - (1 << (x)))) // 取消占用场地上某位置

inline int bit(int x) // 计算二进制中 1 的个数
{
    x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x & 0x0F0F0F0F) + ((x >> 4) & 0x0F0F0F0F);
    x = (x & 0x00FF00FF) + ((x >> 8) & 0x00FF00FF);
    x = (x & 0x0000FFFF) + ((x >> 16) & 0x0000FFFF);
    return x;
}

// 方块姿态
// 7 种形状：I,L,J,T,O,S,Z
// 4 种朝向：左下右上
// 4 对坐标：(x,y)
const int blockShape[7][4][8] = {
    {{0, 0, 0, -1, 0, 1, 0, 2}, {0, 0, -1, 0, 1, 0, 2, 0}, {0, 0, 0, -1, 0, 1, 0, 2}, {0, 0, -1, 0, 1, 0, 2, 0}},
    {{0, 0, 1, 0, 0, 1, 0, 2}, {0, 0, 1, 0, 2, 0, 0, -1}, {0, 0, -1, 0, 0, -1, 0, -2}, {0, 0, -1, 0, -2, 0, 0, 1}},
    {{0, 0, -1, 0, 0, 1, 0, 2}, {0, 0, 1, 0, 2, 0, 0, 1}, {0, 0, 1, 0, 0, -1, 0, -2}, {0, 0, -1, 0, -2, 0, 0, -1}},
    {{0, 0, 1, 0, -1, 0, 0, -1}, {0, 0, -1, 0, 0, 1, 0, -1}, {0, 0, 1, 0, -1, 0, 0, 1}, {0, 0, 1, 0, 0, 1, 0, -1}},
    {{0, 0, 1, 0, 0, 1, 1, 1}, {0, 0, 1, 0, 0, 1, 1, 1}, {0, 0, 1, 0, 0, 1, 1, 1}, {0, 0, 1, 0, 0, 1, 1, 1}},
    {{0, 0, -1, 0, 0, 1, 1, 1}, {0, 0, 0, -1, -1, 0, -1, 1}, {0, 0, -1, 0, 0, 1, 1, 1}, {0, 0, 0, -1, -1, 0, -1, 1}},
    {{0, 0, 1, 0, 0, 1, -1, 1}, {0, 0, 0, 1, -1, 0, -1, -1}, {0, 0, 1, 0, 0, 1, -1, 1}, {0, 0, 0, 1, -1, 0, -1, -1}},
};
class Tetris
{
public:
    const int blockType;   // 标记方块类型的序号，范围 0~6
    int blockX;            // 旋转中心的 x 轴坐标
    int blockY;            // 旋转中心的 y 轴坐标
    int orientation;       // 标记方块的朝向，范围 0~3
    const int (*shape)[8]; // 当前类型方块的姿态定义

    Tetris(int t) : blockType(t), shape(blockShape[t]) {}

    // 放置方块
    inline Tetris &set(int x = -1, int y = -1, int o = -1)
    {
        blockX = (x == -1 ? blockX : x);
        blockY = (y == -1 ? blockY : y);
        orientation = (o == -1 ? orientation : o);
        return *this;
    }

    // 判断当前位置是否合法
    inline bool isValid(int x = -1, int y = -1, int o = -1)
    {
        x = x == -1 ? blockX : x;
        y = y == -1 ? blockY : y;
        o = o == -1 ? orientation : o;
        if (o < 0 || o > 3)
            return false;

        int i, tmpX, tmpY;
        for (i = 0; i < 4; i++) // 检查方块中四个格子是否合法
        {
            tmpX = x + shape[o][2 * i];
            tmpY = y + shape[o][2 * i + 1];
            if (tmpX < 1 || tmpX > MAPWIDTH ||
                tmpY < 1 || tmpY > MAPHEIGHT ||
                grid(tmpX, tmpY))
                return false;
        }
        return true;
    }

    // 判断是否触顶
    inline bool onTop(int x = -1, int y = -1, int o = -1)
    {
        x = x == -1 ? blockX : x;
        y = y == -1 ? blockY : y;
        o = o == -1 ? orientation : o;
        if (o < 0 || o > 3)
            return false;

        for (int i = 0; i < 4; i++)
            if (y + shape[o][2 * i + 1] >= LIMITHEIGHT)
                return true;
        return false;
    }

    // 判断是否落地
    inline bool onGround()
    {
        return isValid() && !isValid(-1, blockY - 1); // 合法 且 往下移动一格后不合法 即可视为落地
    }

    // 将方块放置在场地上
    inline bool place()
    {
        if (onTop())
            return false; // 触顶则不合法

        int i, tmpX, tmpY;
        for (i = 0; i < 4; i++)
        {
            tmpX = blockX + shape[orientation][2 * i];
            tmpY = blockY + shape[orientation][2 * i + 1];
            setgrid(tmpX, tmpY);
        }
        return true;
    }

    // 将方块从场地上移除
    inline void remove()
    {
        int i, tmpX, tmpY;
        for (i = 0; i < 4; i++)
        {
            tmpX = blockX + shape[orientation][2 * i];
            tmpY = blockY + shape[orientation][2 * i + 1];
            unsetgrid(tmpX, tmpY);
        }
    }
};

int eliminateRow; // 记录消行行数

namespace Util
{

    // 随机数发生器
    class randomGenerator
    {
        int a = 27073;        // 乘子
        int M = 32749;        // 模数
        int C = 17713;        // 增量
        int v = 12358;        // 随机数种子
        int curRandomNum = v; // 当前随机数值
    public:
        inline int getRandomNum() // 获取随机值
        {
            return curRandomNum = (curRandomNum * a + C) % M;
        }
    } randomGen;

    // I,L,J,T,O,S,Z 型方块的概率权重分别为：2,3,3,4,5,6,6（和为29）
    inline int getShapeInfo(int randomNum) 
    {
        int weightIndex = randomNum % 29;
        if (weightIndex >= 0 && weightIndex <= 1)
        {
            return 0; // I
        }
        else if (weightIndex > 1 && weightIndex <= 4)
        {
            return 1; // L
        }
        else if (weightIndex > 4 && weightIndex <= 7)
        {
            return 2; // J
        }
        else if (weightIndex > 7 && weightIndex <= 11)
        {
            return 3; // T
        }
        else if (weightIndex > 11 && weightIndex <= 16)
        {
            return 4; // O
        }
        else if (weightIndex > 16 && weightIndex <= 22)
        {
            return 5; // S
        }
        else if (weightIndex > 22)
        {
            return 6; // Z
        }
        return 114514; // 臭
    }

    // 初始化，给场地围一圈护城河，同时计算整局游戏的方块序列
    inline void init()
    {
        int i;
        for (i = 0; i < MAPHEIGHT + 2; i++)
        {
            setgrid(0, i);
            setgrid(MAPWIDTH + 1, i);
        }
        for (i = 0; i < MAPWIDTH + 2; i++)
        {
            setgrid(i, 0);
            setgrid(i, MAPHEIGHT + 1);
        }
        for (i = 0; i < MAXROUND; i++)
        {
            nextType[i] = getShapeInfo(randomGen.getRandomNum());
        }
    }

    // 打印场地用于调试
    inline void printField()
    {
        static const char *i2s[] = {
            "  ",
            "[]"};
        for (int y = LIMITHEIGHT + 1; y >= 0; y--)
        {
            for (int x = 0; x <= MAPWIDTH + 1; x++)
                cout << i2s[grid(x, y)];
            cout << endl;
        }
        cout << endl;
    }

    // 复制当前游戏局面
    inline void fork()
    {
        rep(i, 0, MAPHEIGHT + 1) gridInfo[nowGame + 1][i] = gridInfo[nowGame][i];
        nowRound[nowGame + 1] = nowRound[nowGame];
        nowPoint[nowGame + 1] = nowPoint[nowGame];
        ++nowGame;
    }

    // 游戏回退到复制前的局面
    inline void unfork()
    {
        --nowGame;
    }

    // 消行逻辑实现
    inline void eliminate()
    {
        eliminateRow = 0;
        for (int i = 1; i <= MAPHEIGHT; i++)
        {
            if (gridInfo[nowGame][i] == FULLLINE)
            {
                eliminateRow += 1;
                gridInfo[nowGame][i] = EMPTYLINE;
            }
            else if (eliminateRow)
            {
                gridInfo[nowGame][i - eliminateRow] = gridInfo[nowGame][i];
                gridInfo[nowGame][i] = EMPTYLINE;
            }
        }
    }

    // 获取当前回合的方块
    inline Tetris getNowBlock()
    {
        Tetris block(nextType[nowRound[nowGame]]);
        switch (nextType[nowRound[nowGame]])
        {
        case 1:
        case 2:
        case 3:
            block.set(MAPWIDTH / 2, LIMITHEIGHT, nowRound[nowGame] & 3);
            break;
        case 0:
        case 5:
        case 6:
            block.set(MAPWIDTH / 2, LIMITHEIGHT, nowRound[nowGame] & 1);
            break;
        case 4:
            block.set(MAPWIDTH / 2, LIMITHEIGHT, 0);
            break;
        }
        return block;
    }
}

namespace Feature
{
    int boardRowTransitions;
    int boardColTransitions;
    int boradGrids;
    int y;

    // 返回场面上已被占用格子数
    inline int getGrids()
    {
        boradGrids = 0;
        for (int y = 1; y <= LIMITHEIGHT; y++)
            boradGrids += bit(gridInfo[nowGame][y]) - 2;
        return boradGrids;
    }

    // 返回场面上纵行方向上方块从有到无的变化次数，越少场面越平整
    inline int getTransitions(bool recount = true)
    {
        if (recount) // 重新计算该值
        {
            boardRowTransitions = 0;
            boardColTransitions = 0;
            for (int y = 1; y <= LIMITHEIGHT; y++)
                boardRowTransitions += bit(gridInfo[nowGame][y] ^ (gridInfo[nowGame][y] >> 1)) - 1, // 各行中变换之和
                boardColTransitions += bit(gridInfo[nowGame][y] ^ gridInfo[nowGame][y - 1]);        // 各列中变换之和
        }
        return boardRowTransitions + boardColTransitions;
    }
}

namespace Game
{
    // 优化思路和想法：这里为了节约空间和降低常数采用了回收机制，同时搜索树的操作序列也用一棵树进行储存

    int num = 0;                    // 游戏存档数量
    int latestGameID = 0;           // 最近一次载入的 gameID
    map<ull, int> mapHash2Game;
    vector<ull> mapGame2Hash;
    struct gameNode
    {
        int grid[LIMITHEIGHT], round, point, actIndex;
        gameNode()
        {
            rp(i, LIMITHEIGHT) grid[i] = gridInfo[nowGame][i + 1];
            round = nowRound[nowGame];
            point = nowPoint[nowGame];
            actIndex = -1;
        }
        inline void load()
        {
            rp(i, LIMITHEIGHT) gridInfo[nowGame][i + 1] = grid[i];
            nowRound[nowGame] = round;
            nowPoint[nowGame] = point;
        }
    };
    vector<gameNode> storage;       // 存放游戏存档
    queue<int> trashbin;            // gameID 回收队列

    int actNum = 0;                 // 操作序列总数
    vector<ull> actStorage[10000];
    queue<int> actTrashbin;         // actIndex 回收队列

    // 优化思路和想法：这里为了节约空间和降低常数，搜索树的操作序列用一棵树进行储存
    vector<Pui> actTreeVec;         // actTree 结点，pair<unsigned long long, int> 类型，前者为单轮操作序列，后者为结点父亲下标
    vector<int> actTreeHead;        // 指向该搜索树结点的操作序列最后一轮的 actTree

    // 对于游戏局面的哈希函数，参与计算的对象包括当前轮数和场地情况
    inline ull Hash()
    {
        ull hashVal = nowRound[nowGame];
        rep(y, 1, LIMITHEIGHT) hashVal = hashVal * 233 + gridInfo[nowGame][y];
        return hashVal;
    }

    // 初始化
    inline void init()
    {
        num = 0;
        mapHash2Game.clear();
        storage.clear();
        actTreeVec.clear();
    }

    // 删除游戏局面存档
    inline void close(const int gameID)
    {
        trashbin.push(gameID); // 将该存档所对应的 gameID 放入回收队列
        mapHash2Game.erase(mapGame2Hash[gameID]);
        if (storage[gameID].actIndex != -1)
            actTrashbin.push(storage[gameID].actIndex); // 将该存档所对应的 actIndex 放入回收队列
    }

    // 载入游戏局面存档
    inline void load(const int gameID)
    {
        storage[gameID].load();
        latestGameID = gameID;
    }

    // 保存游戏局面作为存档
    inline int save(const bool root = false)
    {
        ull hash = Hash();
        if (mapHash2Game.count(hash)) // 若之前保存过则不保存
            return -1;

        int gameID;
        if (trashbin.empty()) // 若下标回收队列为空则新建下标，否则从回收队列中取出一个下标准备二次队列
        {
            gameID = num++;
            storage.push_back(gameNode());  // 将场地情况、当前轮数和分数保存在 storage 中
            mapGame2Hash.push_back(hash);
            if (root)                       // 若不为搜索树的根则保存单轮的操作序列
            {
                actTreeHead.push_back(-1);
            }
            else
            {
                actTreeHead.push_back((int)actTreeVec.size());
                actTreeVec.push_back(mp(addCommand[nowGame], actTreeHead[latestGameID]));
            }
        }
        else
        {
            gameID = trashbin.front();
            trashbin.pop();
            storage[gameID] = gameNode();   // 将场地情况、当前轮数和分数保存在 storage 中
            mapGame2Hash[gameID] = hash;
            if (root)                       // 若不为搜索树的根则保存单轮的操作序列
            {
                actTreeHead[gameID] = -1;
            }
            else
            {
                actTreeHead[gameID] = (int)actTreeVec.size();
                actTreeVec.push_back(mp(addCommand[nowGame], actTreeHead[latestGameID]));
            }
        }
        mapHash2Game[hash] = gameID;
        return gameID;
    }

     // 保存游戏局面操作序列，这里的游戏局面操作序列指的是从搜索树的根到该叶子的操作序列
    inline void saveAct(const int layer)
    {
        // 若下标回收队列为空则新建下标，否则从回收队列中取出一个下标准备二次队列
        if (actTrashbin.empty())
            actTrashbin.push(actNum++);
        int actIndex = actTrashbin.front();
        actTrashbin.pop();
        // 获取游戏 ID
        int gameID = mapHash2Game[Hash()];
        // 保存从搜索树的根到该叶子的操作序列
        actStorage[actIndex].clear();
        rp(i, layer) actStorage[actIndex].push_back(0);
        int now = actTreeHead[gameID];
        dw(i, layer) actStorage[actIndex][i] = actTreeVec[now].first, now = actTreeVec[now].second;
        // 记录下 actScorage 的下标
        storage[gameID].actIndex = actIndex;
    }

    // 存放操作序列的变量
    int command[1000000], commandLen;

    // 翻译单轮操作序列并 reverse 后存进 command。将数转为四进制数，其中最高位的 1 为 New(4) 的意思，其他位按照 0D,1L,2R,3C 转化
    inline void translateCommand(ull c)
    {
        vector<int> tmp;
        while (c)
            tmp.push_back(c & 3), c >>= 2;
        for (int i = 0; i < (int)tmp.size(); i++)
            command[commandLen++] = tmp[i];
        command[commandLen - 1] = 4;
    }

    // 打印游戏局面的操作序列（字符串版）
    inline void printCommand(const int gameID)
    {
        commandLen = 0;
        int actIndex = storage[gameID].actIndex;
        int actLen = (int)actStorage[actIndex].size();
        dw(i, actLen) translateCommand(actStorage[actIndex][i]);

        const char *type = "DLRC";
        int nowCommand = -1, times = 0;
        for (int i = commandLen - 1; i >= 0; i--)
        {
            if (nowCommand != command[i])
            {
                if (times)
                {
                    if (nowCommand == 4)
                        printf("N,");
                    else
                        printf("%c%d,", type[nowCommand], times);
                }
                nowCommand = command[i], times = 1;
            }
            else
                times += 1;
        }
        if (times)
        {
            if (nowCommand == 4)
                printf("N");
            else
                printf("%c%d", type[nowCommand], times);
        }
        puts("");
    }

    // 打印游戏局面的操作序列（数字版）
    inline void printCommandInt(const int gameID)
    {
        int actIndex = storage[gameID].actIndex;
        int actLen = (int)actStorage[actIndex].size();
        rp(i, actLen) printf("%llu ", actStorage[actIndex][i]);
        puts("");
    }
}

struct layerNode { int gameID, transition; };   // gameID 为游戏局面 ID，transition 为 场地行列变换数
bool operator<(const layerNode &a, const layerNode &b) { return a.transition < b.transition; }
priority_queue<layerNode> layerPQ[MAXLAYER];    // 保存搜索树非叶子结点的优先队列

struct bucketNode { int gameID, point, transition; };   // gameID 为游戏局面 ID，point 为分数，transition 为 场地行列变换数
bool operator<(const bucketNode &a, const bucketNode &b) {return (a.point != b.point) ? a.point > b.point : a.point < b.point; }
priority_queue<bucketNode> bucketPQ[MAXROUND + 1][100]; // 保存搜索树叶子的桶，第一个维度为游戏轮数，第二个维度为场地格子数量 / 2（为了节约内存空间）

#define MAXLAYERPQSIZE 200000                   // 搜索树单层结点数量上限
#define MAXBUCKETPQSIZE 10                      // 每个桶的大小上限

ull posValid[MAPWIDTH + 2][MAPHEIGHT + 2][4];   // 若为 0 则为不合法状态，若不为 0 则为合法状态，此时值为从起始状态到该状态的操作序列
struct posNode { int x, y, o; };                // x 和 y 为坐标，o 为方块形态
queue<posNode> posQueue;                        // BFS 搜索队列

bucketNode varTop; // 临时变量

// 搜索树单层拓展搜索
inline void search(const int layer)
{
    Tetris block = Util::getNowBlock();
    if (!block.isValid())
        return; // 若当前局面的初始方块无法被放置则中止

    // 初始化
    memset(posValid, 0, sizeof(posValid));
    posValid[MAPWIDTH / 2][LIMITHEIGHT][block.orientation] = 1;
    posQueue.push((posNode){MAPWIDTH / 2, LIMITHEIGHT, block.orientation});

    // 临时变量
    int x, y, o, gameID;
    bool onGround;

    // 使用 BFS 搜索方块所有可放置的位置
    while (!posQueue.empty())
    {
        x = posQueue.front().x;
        y = posQueue.front().y;
        o = posQueue.front().o;
        posQueue.pop();

        block.set(x, y, o);
        if (!posValid[x - 1][y][o] && block.isValid(x - 1, y, o)) // 判断方块是否可以向左移动
            posValid[x - 1][y][o] = (posValid[x][y][o] << 2) + 1, posQueue.push((posNode){x - 1, y, o});
        if (!posValid[x + 1][y][o] && block.isValid(x + 1, y, o)) // 判断方块是否可以向右移动
            posValid[x + 1][y][o] = (posValid[x][y][o] << 2) + 2, posQueue.push((posNode){x + 1, y, o});
        if (!posValid[x][y - 1][o] && block.isValid(x, y - 1, o)) // 判断方块是否可以向下移动
            posValid[x][y - 1][o] = (posValid[x][y][o] << 2) + 0, posQueue.push((posNode){x, y - 1, o});

        switch (nextType[nowRound[nowGame]])
        { // 判断是否可以旋转
        case 1:
        case 2:
        case 3:
            if (!posValid[x][y][(o + 1) & 3] && block.isValid(x, y, (o + 1) & 3)) // L J T 有四种不同姿态
                posValid[x][y][(o + 1) & 3] = (posValid[x][y][o] << 2) + 3, posQueue.push((posNode){x, y, (o + 1) & 3});
            break;
        case 0:
        case 5:
        case 6:
            if (!posValid[x][y][(o + 1) & 1] && block.isValid(x, y, (o + 1) & 1)) // I S Z 有两种不同姿态
                posValid[x][y][(o + 1) & 1] = (posValid[x][y][o] << 2) + 3, posQueue.push((posNode){x, y, (o + 1) & 1});
            break;
        }

        // 如果方块不触顶则为可能的落点，无需落地（即可以悬空）
        if (!block.onTop())
        {
            onGround = block.onGround();
            Util::fork();      // 接下来的操作会对游戏局面进行修改，为了方便撤销，故复制游戏局面
            block.place();     // 放置方块
            Util::eliminate(); // 计算消行
            nowRound[nowGame] += 1;
            nowPoint[nowGame] += (Feature::getGrids() + 10 * eliminateRow) * (1 + eliminateRow) * eliminateRow / 2;
            addCommand[nowGame] = posValid[x][y][o]; // 记录最近一次的单轮操作序列
            if (eliminateRow || nowRound[nowGame] >= MAXROUND) // 如果消行数量大于 0 或到达游戏轮数上限，则将当前游戏局面作为搜索树的叶子进行保存，不再进行拓展
            {
                int i = Feature::getGrids() / 2; // 为了节省不必要的的空间，将必定是偶数的格子总数除以二
                if (i >= 50) // 若消行后当前场地格子数不少于 100 则保存，否则舍弃。优化思路和想法：一个优秀的操作序列是不应该为了消行把场地格子数下降到过低
                {
                    // bucketPQ 只保留 MAXBUCKETPQSIZE 个搜索树叶子，其第一优先级是分数越高越好，第二优先级是场地行列变换数越低越好
                    if (bucketPQ[nowRound[nowGame]][i].size() < MAXBUCKETPQSIZE || nowPoint[nowGame] > (varTop = bucketPQ[nowRound[nowGame]][i].top()).point || (nowPoint[nowGame] == varTop.point || Feature::getTransitions() < varTop.transition))
                    {
                        // 保存游戏局面
                        if ((gameID = Game::save()) >= 0) // 若该游戏局面已被保存过，则不进行保存
                        {
                            if (bucketPQ[nowRound[nowGame]][i].size() == MAXBUCKETPQSIZE)
                                Game::close(varTop.gameID), bucketPQ[nowRound[nowGame]][i].pop();
                            bucketPQ[nowRound[nowGame]][i].push((bucketNode){gameID, nowPoint[nowGame], Feature::getTransitions()});
                            Game::saveAct(layer); // 保存游戏局面操作序列，这里的游戏局面操作序列指的是从搜索树的根到该叶子的操作序列
                        }
                    }
                }
            }
            else if (onGround) // 若无消行且落地则保存游戏局面到下一层搜索树中，等待下一次拓展。优化思路和想法：虽然说方块可以悬空，但这样会大大增加状态数量，故在此添加了一个人为的限定条件，即能带来消行的方块可以悬空，同时不能消行的方块不能悬空
            {
                // 同一层的搜索树只保留 MAXLAYERPQSIZE 个搜索树结点，其优先级是场地行列变换数越低越好
                if (layerPQ[layer].size() < MAXLAYERPQSIZE || Feature::getTransitions() < layerPQ[layer].top().transition)
                {
                    // 保存游戏局面
                    if ((gameID = Game::save()) >= 0) // 若该游戏局面已被保存过，则不进行保存
                    {
                        if (layerPQ[layer].size() == MAXLAYERPQSIZE)
                            Game::close(layerPQ[layer].top().gameID), layerPQ[layer].pop();
                        layerPQ[layer].push((layerNode){gameID, Feature::getTransitions(false)});
                    }
                }
            }
            Util::unfork(); // 恢复复制前的游戏局面
        }
    }
}

int main(int argc, char *argv[])
{
    Util::init();
    nowPoint[nowGame] = atoi(argv[1]); // 读取第一个命令行参数作为初始局面的分数
    nowRound[nowGame] = atoi(argv[2]); // 读取第二个命令行参数作为初始局面的轮数
    rep(i, 1, 20) gridInfo[nowGame][21 - i] = atoi(argv[i + 2]); // 读取后续的命令行参数作为初始局面的场地情况

    Game::init();
    layerPQ[0].push((layerNode){Game::save(true), Feature::getTransitions()}); // 将初始局面保存到搜索树的第 0 层作为搜索起点

    // 开始 BFS 广度优先搜索
    int gameID;
    rp(i, MAXLAYER - 1)
    {
        while (!layerPQ[i].empty()) // 若该层搜索树存在待拓展的游戏局面
        {
            gameID = layerPQ[i].top().gameID;
            Game::load(gameID); // 读取游戏局面
            layerPQ[i].pop();
            search(i + 1); // 进行第 i+1 层的搜索
            Game::close(gameID);
        }
    }
    
    // 输出搜索结果
    rep(i, 0, MAXROUND) rp(j, 100) while (!bucketPQ[i][j].empty())
    {
        int gameID = bucketPQ[i][j].top().gameID;
        Game::load(gameID);
        printf("%d %d ", i, nowPoint[nowGame]);
        dow(i, 20, 1) printf("%d ", gridInfo[nowGame][i]);
        Game::printCommandInt(gameID);
        bucketPQ[i][j].pop();
    }

    return 0;
}
