import sys
with open(sys.argv[2], 'r') as f: ans = f.readlines()
with open(sys.argv[1], 'r') as f: pred = f.readlines()
assert(len(ans) == len(pred))
print(sum([float(a.split()[0]==p.split()[0]) for a,p in zip(ans, pred)])/len(ans))
