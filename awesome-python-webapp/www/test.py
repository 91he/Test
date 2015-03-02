#!/usr/bin/env python

import math

for i in range(1, 11):
	for j in range(i + 1, 11):
		print 'sqrt(%d) - sqrt(%d) = %.2f' % (j, i, math.sqrt(j) - math.sqrt(i))
