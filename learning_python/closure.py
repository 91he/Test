#!/usr/bin/python
# -*- coding: utf-8 -*-

def fun(*args):
	return lambda: args[0] * args[0] + args[1] * args[1]

s = [3, 4]
f = fun(*s);
print f()
