import os
import gc
from multiprocessing import Pool
from functools import total_ordering

MAXROUND = 10000

@total_ordering
class Game:
    def __init__(self, round, point, grid, act):
        self.round = round
        self.point = point
        self.grid = grid
        self.act = act
        self.num = 0
        self.trans = 0
        self.hash = point * 10007 + round
        for x in grid:
            self.num += bin(x).count('1') - 2
            self.trans += bin(x ^ (x >> 1)).count('1') - 1
            self.hash = self.hash * 10007 + x
        for x, y in zip(grid[1:]+[4095], grid):
            self.trans += bin(x ^ y).count('1')

    def __eq__(self, other):
        return self.hash == other.hash

    def __lt__(self, other):
        return self.point < other.point if self.point != other.point else self.trans > other.trans

    def __hash__(self):
        return self.hash


def toGame(s, g):
    s = [int(x) for x in s.strip().split(' ')]
    return Game(s[0], s[1], s[2:22], g.act+s[22:])


# 调用外部程序 tetris 进行单步搜索（这里的单步指的是从一次消行到另一次消行的过程）
def subtask(g, id):
    os.system(f'./tetris {g.point} {g.round} {" ".join([str(x) for x in g.grid])} > ./result/{id}')
    try:
        ret = [toGame(s, g) for s in open(f'./result/{id}', 'r').readlines()]
    except:
        ret = []
        print(f'{id} NO OUTPUT')
    return ret


MAXSIZE = 10 # 每个桶的大小上限
all = [{} for _ in range(MAXROUND+1)]


# 将游戏局面加入到桶
def insert(x):
    global all
    if x.num not in all[x.round].keys():
        all[x.round][x.num] = set([])

    if len(all[x.round][x.num]) == MAXSIZE:
        if min(all[x.round][x.num]) < x:
            all[x.round][x.num].remove(min(all[x.round][x.num]))
            all[x.round][x.num].add(x)
    else:
        all[x.round][x.num].add(x)


# 读取储存的备份数据
def getPreResult(st):
    try:
        f = open(f'./backup/{st}', 'r')
        for s in f.readlines():
            s = [int(x.replace('[','').replace(']','').replace(',','')) for x in s.strip().split(' ')]
            insert(Game(st, s[0], s[2:22], s[22:]))
        f.close()
    except FileNotFoundError:
        pass


if __name__ == '__main__':

    # 从初始局面开始搜索。手玩的初始局面，并不是最完美的。
    # start = Game(26, 0, [2049,2049,2049,2049,2049,2049,2049,2081,3697,4063,4091,4061,4063,4091,4091,3839,4087,4087,3071,3839], [117097988358144,274877906944,76209899700224,32074037303705600,78365973282816,20583630766080,103079215104,4759645847552,485868175360,331853332480,5268448477184,1735653544951808,4831838208,455317913600,5438090736828416,18442354688,7805599744,1298128699392,310378496,483164160,20007444063373868,84934656,87243996978126848,1296379904,69869568,1884160])
    # insert(start)

    # 或者从某一轮开始搜索
    startRound = 9998

    for round in range(startRound, MAXROUND):
        pl = Pool(32)   # 线程池中线程数量
        maxpoint = 0    # 当前轮中能达到的最高的分数
        result = []
        id = 0
        getPreResult(round) # 读取当前轮储存的备份数据

        # 开始调用外部程序 tetris 进行搜索
        for num in all[round].keys():
            for g in all[round][num]:
                maxpoint = max(maxpoint, g.point)
                result.append(pl.apply_async(subtask, args=(g, id,)))
                id += 1
        pl.close()
        pl.join()

        # 读取外部程序 tetris 的运行结果，并加入到桶
        result = [x.get() for x in result]
        for _ in result:
            for x in _:
                insert(x)

        # 输出一些调试信息
        print(f'round {round}: {maxpoint}')
        for x in range(round, min(round+50, MAXROUND+1)):
            for num in all[x].keys():
                maxpoint = 0
                for g in all[x][num]:
                    maxpoint = max(maxpoint, g.point)
                print(f'{x}-{num}-{maxpoint}', end=' ')
            if len(all[x]):
                print()

        # 保存当前轮的数据到文件中
        if len(all[round]):
            f = open(f'./backup/{round}', 'w')
            for num in all[round].keys():
                for g in all[round][num]:
                    f.write(f'{g.point} {g.num} {g.grid} {g.act}\n')
            f.close()

        # 清空当前轮的桶
        all[round] = {}
        gc.collect()

    f = open(f'./backup/{MAXROUND}', 'w')
    for num in all[MAXROUND].keys():
        for g in all[MAXROUND][num]:
            f.write(f'{g.point} {g.num} {g.grid} {g.act}\n')
    f.close()