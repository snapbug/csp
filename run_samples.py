#!/usr/bin/env python
import random, subprocess, os

os.chdir(os.path.expanduser('~/Masters/csp-git'))
u = range(429584)
random.shuffle(u)

ssize = 20
nsamples = 1

for s in range(nsamples):
	us = map(str, sorted(u[s * ssize:(s + 1) * ssize]))
	p = subprocess.Popen(['./bin/csp', '-e', '-pk', '-gp', '-seE'] + us, stdout=subprocess.PIPE).communicate()
	print >> file('output/sample.%d.gp.pk.mae.txt' % s, 'w'), p[0]
	print
	print '%s %d %s' % ('-'*15, s, '-'*15)
