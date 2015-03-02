#!/usr/bin/python
# -*- coding: utf-8 -*-

import functools

def log(*ags):
	def decorator(fun):
		@functools.wraps(fun)
		def tmp(*args, **kw):
			print '%s %s' % (ags, fun.__name__)
			fun(*args, **kw)
		return tmp
	return decorator

def dec(fun):
	@functools.wraps(fun)
	def tmp(*args, **kw):
		print 'test %s' % fun.__name__
		fun(*args, **kw)
	return tmp

def wrap(text):
	if isinstance(text, str):
		def drt(fun):
			@functools.wraps(fun)
			def tmp(*args, **kw):
				print 'before: %s %s' % (text, fun.__name__)
				fun(*args, **kw)
				print 'after: %s %s' % (text, fun.__name__)
			return tmp
		return drt
	else:
		def tmp(*args, **kw):
			print 'before: call %s' % text.__name__
			text(*args, **kw)
			print 'after: call %s' % text.__name__
		return tmp

#@log('execute')
@wrap
#@dec
def f():
	pass

f()

@wrap('call')
def f1():
	pass

f1()
