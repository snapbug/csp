import random, subprocess, os

os.chdir('D:/Masters/csp-git')
u = range(429584)
random.shuffle(u)
ssize = 1000
nsamples = 10

for s in range(nsamples):
	us = map(str, sorted(u[s * ssize:(s + 1) * ssize]))
	p = subprocess.Popen(['./bin/csp.exe', '-e', '-pk', '-gp', '-seE'] + us, stdout=subprocess.PIPE).communicate()
	print >> file('output/sample.%d.gp.pk.mae.txt' % s, 'w'), p[0]
	print '%s %d %s' % ('-'*15, s, '-'*15)
