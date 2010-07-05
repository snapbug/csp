#!/usr/bin/env python
import subprocess, os

os.chdir(os.path.expanduser('~/Masters/csp-git'))
nsamples = 1

for s in range(nsamples):
	us = file('output/sample.%d.gp.pk.mae.txt' % s, 'r').next().strip().split()
	p = subprocess.Popen(['./bin/csp', '-e', '-pk', '-gg', '-seE'] + us, stdout=subprocess.PIPE).communicate()
	print >> file('output/sample.%d.gg(exhaust).pk.mae.txt' % s, 'w'), p[0]
	print '%s %d %s' % ('-'*15, s, '-'*15)
