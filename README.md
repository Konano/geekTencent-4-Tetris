# 腾讯极客挑战赛第四期：鹅罗斯方块

写得很 OI 风格的一个算法实现。

参赛成绩为外网赛道第一，分数为 1413876。

具体代码见 `tetris.cpp` 和 `multi.py`。

要是跑太慢的话可以增加 `multi.py` 中的线程数量，或者减小 `tetris.cpp` 中的 `MAXLAYERPQSIZE` 和 `MAXBUCKETPQSIZE` 这两个变量。

复盘：【施工中】

## Usage

```
$ g++ tetris.cpp -o tetris -O3
$ mkdir result backup
$ python3 multi.py
$ # wait for a long time....
$ python3 check.py 10000
$ python3 choose.py 10000
```
