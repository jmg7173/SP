#!/usr/bin/python
from sys import stdin, stdout

while True:
    str1 = stdin.readline()
    if not str1:
        break
    for word in str1.split():
        stdout.write("%s\t1\n" % word)
