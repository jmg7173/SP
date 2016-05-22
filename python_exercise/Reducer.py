#!/usr/bin/python
from sys import stdin, stdout

words = {}
while True:
    str1 = stdin.readline()
    if not str1:
        break
    word, count = str1.split()[0], str1.split()[1]
    if word in words:
        words[word] += 1
    else:
        words[word] = 1

for word in words:
    stdout.write("%s\t%d\n" % (word, words[word]))
