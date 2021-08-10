import sys

def translate(a):
    c = []
    for x in a:
        tmp = []
        while x:
            tmp.append(x&3)
            x >>= 2
        tmp[-1] = 4
        c += tmp[::-1]

    C = 'DLRC'
    s = []
    d = -1
    t = 0
    for x in c:
        if x != d:
            if t:
                if d == 4:
                    s.append('N')
                else:
                    s.append(f'{C[d]}{t}')
            d = x
            t = 1
        else:
            t += 1

    if t:
        if d == 4:
            s.append('N')
        else:
            s.append(f'{C[d]}{t}')

    print(','.join(s))

mx = 0

try:
    f = open(f'./backup/{sys.argv[1]}', 'r')
    s = f.readlines()
    s = [int(x.split(' ')[0]) for x in s]
    for x in s:
        mx = max(mx, x)
except FileNotFoundError:
    pass

try:
    f = open(f'./backup/{sys.argv[1]}', 'r')
    s = f.readlines()
    s = [[int(x.replace('[','').replace(']','').replace(',','')) for x in sub.strip().split(' ')] for sub in s]
    for x in s:
        if x[0] == mx:
            translate(x[22:])
            print(x[0])
            exit(0)
except FileNotFoundError:
    pass
