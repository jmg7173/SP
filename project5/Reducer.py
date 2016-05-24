#!/usr/bin/python
from sys import stdin, stdout

words = {}
while True:
    str1 = stdin.readline()
    if not str1:
        break
    splited = str1.split()
    key, word, count = splited[0], splited[1], splited[2]
    if key not in words:
        words[key] = {}
    
    if word in words[key]:
        words[key][word] += 1
    else:
        words[key][word] = 1

for keys in sorted(words):
    for word in sorted(words[keys]):
        stdout.write("%s %s\t%d\n" % (keys, word, words[keys][word]))    
