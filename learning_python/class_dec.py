#!/usr/bin/env python
# -*- coding: utf-8 -*-

class Router(object):
	def __init__(self):
		pass
	def __call__(self, path):
		def dec(fun):
			self.path = path
			self.fun = fun
			return fun
		return dec

router = Router()

@router("/tmp")
def f():
	print 'hello'

print f()
#print router.path
