import subprocess
# Here, we are creating our class, Window, and inheriting from the Frame
# class. Frame is a class from the tkinter module. (see Lib/tkinter/__init__)

import os

# # response = os.system("ping -c 1 192.168.1.220")
# result = subprocess.run('ping -c 1 192.168.1.220',shell=True, stdout=subprocess.PIPE)

# print(result.stdout.decode('utf-8'))
# # if result.stdout.decode('utf-8') == 0:
# # 	print('online')
# # else:
# # 	print('offline')


ipaddress = '192.168.1.120'  # guess who
proc = subprocess.Popen(
	['ping', '-c', '1', ipaddress],
	stdout=subprocess.PIPE)
stdout, stderr = proc.communicate()
if proc.returncode == 0:
	print('{} is UP'.format(ipaddress))
	# print('ping output:')
	# print(stdout.decode('ASCII'))