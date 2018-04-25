import os
import sys
import time
from subprocess import *

nodes = [128] #128, 256] # [16, 32, 64, 128, 256, 512]  # [sys.argv[1]] 

def runcmd (node, iter):

	script = './runknl.sh ' + str(node) 
	cmd = 'qsub -A datascience -t 00:59:00 -n '+str(node)+' --mode script '+script + ' ' + str(iter)  
#	cmd = 'qsub -q R.perf_test -A Performance -t 00:30:00 -n ' + str(node) +  ' --attrs="location=3130-3139,3160-3167,3170-3177,3180-3189,3192-3193,3198-3200,3264-3265,3268-3269,3273,3276,3281,3283-3285,3299,3310-3311,3313,3318-3319,3332,3364,3369,3402-3407,3438,3442-3445,3447-3449,3464-3469,3479,3482-3487,3491,3527,3540-3548,3610-3611,3615-3617,3640-3642,3648-3649,3651,3670-3677,3720-3723,3726-3727,3747,3765,3780,3785,3842"' + ' --mode script ' + script 

	print 'Executing ' + cmd
	jobid = Popen(cmd, shell=True, stdout=PIPE).communicate()[0]
	print 'Jobid : ' + jobid
	
	while True:
		cmd = 'qstat ' + jobid.strip() + ' | grep preeti | awk \'{print $1}\''
		jobrun = Popen(cmd, shell=True, stdout=PIPE).communicate()[0]
		if jobrun == '':
			break
		time.sleep(30)

	return jobid.strip()

for iter in range (1, 10):
 for node in nodes:
		print '\nStarting on ' + str(node) + ' nodes' #+ str(rank) + ' ranks per node'
		jobid = runcmd(node, iter)
		filename = 'rt_'+str(node)+'_'+str(iter)
		print filename + ' ' + jobid
		continue
		cmd = 'mv ' + jobid.strip() + '.output ' + filename + '.output'
		Popen(cmd, shell=True, stdout=PIPE).communicate()[0]
		cmd = 'mv ' + jobid.strip() + '.error ' + filename + '.error'
		Popen(cmd, shell=True, stdout=PIPE).communicate()[0]
		cmd = 'mv ' + jobid.strip() + '.cobaltlog ' + filename + '.cobaltlog'
		Popen(cmd, shell=True, stdout=PIPE).communicate()[0]


