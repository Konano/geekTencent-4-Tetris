import sys

mx = 0

try:
    f = open(f'./backup/{sys.argv[1]}', 'r')
    s = f.readlines()
    s = [int(x.split(' ')[0]) for x in s]
    for x in s:
        mx = max(mx, x)
except FileNotFoundError:
    pass

print(mx)
