import sys
filename = sys.argv[1]
with open(filename) as f:
    lines = []
    for line in f:
        x, y = line.split(',')
        lines.append('{0: 15.10f},{1: 15.10f}'.format(float(x), float(y)))
with open(filename, 'w') as f:
    for line in lines:
        f.write(line + '\n')
