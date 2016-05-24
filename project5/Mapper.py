#!/usr/bin/python
from sys import stdin, stdout

while True:
    str1 = stdin.readline().lower()
    if not str1:
        break
    prev = ""
    for word in str1.split(): 
        if prev:
            stdout.write("%s %s\t1\n" % (prev,word))
        prev = word
