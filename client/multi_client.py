#!/usr/bin/python

import subprocess as sub
import sys

if len(sys.argv) < 3:
	print "error: not enough arguments"
	print "usage: ./multi_client.py <iterations> <ip_addr> <port>"
	sys.exit(1)

count	= int(sys.argv[1])
ip_addr	= sys.argv[2]
port 	= sys.argv[3]

p = []

try:
	print "starting " + str(count) + " clients..."
	for i in range(0, count):
		p.append(
			sub.Popen(["../bin/client207", ip_addr, port],)
		)
		print "proc #" + str(i+1) + " running"
		p[i].wait()

	print "complete"

except KeyboardInterrupt:
	for i in range(0, count):
		p[i].kill()
		print "proc #" + str(i+1) + " killed"
	sys.exit(0)
finally:
	sys.exit(0)