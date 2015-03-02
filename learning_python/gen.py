#!/usr/bin/python
# -*- coding: utf-8 -*-

def gen():
	for i in range(1, 101):
		yield i

g = gen()
for i in g:
	print i,
