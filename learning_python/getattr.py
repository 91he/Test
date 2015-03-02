#!/usr/bin/env python
# -*- coding: utf-8 -*-

class Chain(object):
	def __init__(self, path = ''):
		self.path = path
	def __getattr__(self, path):
		return Chain('%s/%s' % (self.path, path))
	def __str__(self):
		return self.path
	__repr__ = __str__
	def __iter__(self):
		return iter(self.path[1:].split('/'))
	def __call__(self, path):
#		print 'there'
		return Chain('%s/%s' % (self.path, path))
#	def users(self, name):
#		print 'here'
#		return Chain('%s/users/%s' % (self.path, name))

print Chain().hello.world.nihao.haha
print Chain().users('Michael').repos

for i in Chain().users('Michael').repos:
	print i,

print

print max(Chain().users('Michael').repos)
