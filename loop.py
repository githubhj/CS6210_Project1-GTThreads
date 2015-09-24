import os
import sys
import subprocess

for i in range(1,1000):
	#clean_process = subprocess.Popen("./producer_consumer", cwd="./", shell=True)
    	#clean_process.communicate()
    	#clean_process.wait()
	#print (i,file=sys.stderr)
	os.system("./producer_consumer")
