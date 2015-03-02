#!/usr/bin/python
# Filename: pickling.py

try:
	import cPickle as p
except ImportError:
	import pickle as p

shoplistfile = 'shoplist.data'
# the name of the file where we will store the object

#shoplist = ['apple', 'mango', 'carrot']
#shoplist = {'Name' : 'lilei', 'Age' : 30, 'Class' : 'One'}

class Student:
	def __init__(self, name, age, salary):
		self.name = name
		self.age = age
		self.salary = salary
	def tell(self):
		print 'Name:', self.name, 'Age:', self.age, 'Salary:', self.salary

tmp = {'Name' : 'lilei', 'Age' : 30, 'Class' : 'One'}
shoplist = Student('lilei', 20, tmp)

tmp['nothing'] = 'none'
# Write to the file
f = file(shoplistfile, 'w')
p.dump(shoplist, f) # dump the object to a file
f.close()

del shoplist # remove the shoplist

# Read back from the storage
f = file(shoplistfile)
storedlist = p.load(f)
print storedlist
storedlist.tell()
print storedlist.salary
