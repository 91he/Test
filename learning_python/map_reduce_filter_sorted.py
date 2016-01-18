#!/usr/bin/python
# -*- coding: utf-8 -*-

import math

list = ['adam', 'LIST', 'barT']

def fun(name):
	return name[0].upper() + name[1:].lower()
	tmp = name.lower()
	return tmp[0].upper() + tmp[1:]

print map(fun, list)

values = (1, 2, 3, 4, 5, 6)

print reduce(lambda x, y: x * y, values)

def judge(val):
	for i in range(2, int(math.sqrt(val)) + 1):
		if val % i == 0:
			return True
	return False

print filter(judge, range(1, 101))

print sorted(values, lambda x, y: y - x)
