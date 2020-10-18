#!/usr/bin/python3

import collections
import sys

infile = sys.argv[1]
outfile = sys.argv[2]
#infile = 'Big-ZhuYin.map'
#outfile = 'ZhuYin-Big5.map'

dic = collections.defaultdict(list)
with open(infile, 'r', encoding='BIG5-HKSCS') as f:
    for line in f:
        ch, zhuins = line.split(' ', 1)
        zhuins = zhuins.split('/')
        for zhuin in zhuins:
            dic[zhuin[0]].append(ch)
        
        dic[ch] = [ch]

with open(outfile, 'w', encoding='BIG5-HKSCS') as f:
    for k, v in dic.items():
        f.write(' '.join([k] + v) + '\n')
