#!/usr/bin/python

import os, re
from subprocess import *

#corelist = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
corelist = [1,2,3,4]
pagecntlist = [32000, 64000, 128000, 256000]
repeat = 5

def warmup():
	print('Warming up...')
	for i in xrange(3):
		p = Popen('./mmapbench 1', shell=True, stdout=PIPE)
		os.waitpid(p.pid, 0)

def test():
	print('Begin testing...')
	pattern = re.compile(r'usec: (\d+)')
	for n in corelist:
		for pg in pagecntlist:
			total = 0
			for i in xrange(repeat):
				os.system("rm -rf /mnt/pmfs/*")
				p = Popen('./mmapbench %d %d' % (n,  pg), shell=True, stdout=PIPE)
				#p = Popen('./mmapbench %d %d' % pg, shell=True, stdout=PIPE)
				#p=Popen(["./mmapbench",str(n), str(pg)], shell=True, stdout=PIPE)
				#p=Popen(["./mmapbench",str(n), str(pg)])
				os.waitpid(p.pid, 0)
				output = p.stdout.read().strip()
				usec = int(pattern.search(output).group(1))
				total += usec
  			print('Core #%d: Pages #%d:  %d' % (n, pg, total / repeat))
				#print('Core #%d: %d' % (n, usec))

warmup()
test()
