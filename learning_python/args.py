#!/usr/bin/python

def func(a, b, c=0, *args, **kw):
	print 'a =', a, 'b =', b, 'c =', c, 'args =', args, 'kw =', kw

args = [1, 2, '3']
kw = {'lilei' : 100}

func(*args, **kw)
