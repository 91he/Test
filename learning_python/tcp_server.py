#!/usr/bin/env python
# -*- coding: utf-8 -*-

import socket

fs = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
fs.bind(('127.0.0.1', 5678))
fs.listen(5)

fd, addr = fs.accept()

while True:
	buf = fd.recv(1024)
	if not buf:
		print 'closed'
		fd.close()
		break
	print type(buf), buf
	print 'start', int(buf), 'end'

