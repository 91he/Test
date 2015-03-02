#!/usr/bin/python
# -*- coding: utf-8 -*-

class Stu:
	'a test class'
	def __init__(self):
		print 'new Stu'
	def call(self):
		print self.age

s = Stu()
s.age = 30
print s.age
s.call()
