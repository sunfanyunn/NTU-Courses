#!/usr/bin/env python2

import filecmp
import shutil
import os
import time
import stat
import signal
from fnmatch import fnmatch
def _filter(flist, skip):
    return [item for item in flist 
                 if not any(fnmatch(item, pat) for pat in skip)]
filecmp._filter = _filter

MP3Dir = '..'

def diff_dir(dir1,dir2):

	type_wrong = []
	exist_wrong = []
	content_wrong = []
	mode_wrong = []
	time_wrong = []
	right = []
	dcmp = filecmp.dircmp(dir1,dir2,ignore=['.*'])

	# if there is any file in dir1 not in dir2, vice versa
	for x in dcmp.left_list:
		if x not in dcmp.common:
			exist_wrong.append(dir1+'/'+x)
	for x in dcmp.right_list:
		if x not in dcmp.common:
			exist_wrong.append(dir2+'/'+x)

	# compare common files
	for f in dcmp.common_files:
		stat1 = os.lstat(dir1+'/'+f)
		stat2 = os.lstat(dir2+'/'+f)
		if not filecmp.cmp(dir1+'/'+f, dir2+'/'+f,False):
			content_wrong.append(dir1+'/'+f)
		elif stat.S_IFMT(stat1.st_mode) != stat.S_IFMT(stat2.st_mode):
			type_wrong.append(dir1+'/'+f)
		elif stat.S_IMODE(stat1.st_mode) != stat.S_IMODE(stat2.st_mode):
			mode_wrong.append(dir1+'/'+f)
		elif int(stat1.st_mtime) != int(stat2.st_mtime):
			time_wrong.append(dir1+'/'+f)
		else:
			right.append(dir1+'/'+f)
	for dirc in dcmp.common_dirs:
		right.append(dir1+'/'+dirc)
		[a,b,c,d,e,f] = diff_dir(dir1+'/'+dirc,dir2+'/'+dirc)
		type_wrong = type_wrong +a		
		exist_wrong = exist_wrong + b
		content_wrong = content_wrong + c
		mode_wrong = mode_wrong + d
		time_wrong = time_wrong + e		
		right = right + f

	return (type_wrong, exist_wrong, content_wrong, mode_wrong, time_wrong, right)
	
			

cwd = os.getcwd();
if os.path.exists(MP3Dir+'/cdir'):
	shutil.rmtree(MP3Dir+'/cdir')
shutil.copytree('./cdir', MP3Dir+'/cdir',True)
if os.path.exists(MP3Dir+'/sdir'):
	shutil.rmtree(MP3Dir+'/sdir')
os.mkdir(MP3Dir+'/sdir')
if not os.path.exists('./assignment1_test_result'):
	os.mkdir('./assignment1_test_result')
if os.path.exists(MP3Dir+'/config'):
	shutil.rmtree(MP3Dir+'/config')
shutil.copytree('./config',MP3Dir+'/config')
os.chdir(MP3Dir+'/src')
os.system('make clean')
if os.system('make') != 0:
	print 'make fail'
	quit()
os.chdir('../')

pid_port_register = os.fork()
if pid_port_register == 0:
	#child process
	os.chdir('./bin')
	f = open(os.devnull,'w')
	os.dup2(f.fileno(),1)
	os.dup2(f.fileno(),2)
	os.execl('./port_register', 'port_register')
time.sleep(1)
pid_server = os.fork()
if pid_server == 0:
	#child process
	os.chdir('./bin')
	f = open(cwd+'/assignment1_test_result/csiebox_server_stdout','w+')
	os.dup2(f.fileno(),1)
	f = open(cwd+'/assignment1_test_result/csiebox_server_stderr','w+')
	os.dup2(f.fileno(),2)
	os.execl('./csiebox_server', 'server', '../config/server.cfg')
print 'start csiebox_server'
time.sleep(1)
pid_child = os.fork()
if pid_child == 0:
	#child process
	os.chdir('./bin')
	f = open(cwd+'/assignment1_test_result/csiebox_client_stdout','w+')
	os.dup2(f.fileno(),1)
	f = open(cwd+'/assignment1_test_result/csiebox_client_stderr','w+')
	os.dup2(f.fileno(),2)
	os.execl('./csiebox_client', 'client', '../config/client.cfg')
print 'start csiebox_client'
print 'sleep ' + str(60) + 's for your program to synchronize'
time.sleep(60)
print '================================================================='
[type_wrong, exist_wrong, content_wrong, mode_wrong, time_wrong, right]= diff_dir('./cdir','./sdir/user')
print 'result after initial synchronization:'
print 'files that only exist in one side:'
if len(exist_wrong) == 0:
	print 'None'
else:
	print str(exist_wrong).decode('string_escape')
print 'files that have inconsistent type:'
if len(type_wrong) == 0:
	print 'None'
else:
	print str(type_wrong).decode('string_escape')
print 'files that have inconsistent content:'
if len(content_wrong) == 0:
	print 'None'
else:
	print str(content_wrong).decode('string_escape')
print 'files that have inconsistent permission:'
if len(mode_wrong) == 0:
	print 'None'
else:
	print str(mode_wrong).decode('string_escape')
print 'files that have inconsisten modification time:'
if len(time_wrong) == 0:
	print 'None'
else:
	print str(time_wrong).decode('string_escape')
print 'files that are consistent:'
if len(right) == 0:
	print 'None'
else:
	print str(right).decode('string_escape')

print 'start testing inotify'
print 'make new dir cdir/newDir'
os.mkdir('./cdir/newDir')
print 'create new file cdir/test.txt'
os.system('echo This is a pen, that is an apple > ./cdir/test.txt')
print 'create new file cdir/newDir/hahaaa'
os.system('echo This is another test file > ./cdir/newDir/hahaaa')
print 'remove dir cdir/dir_to_rm and files in it'
shutil.rmtree('cdir/dir_to_rm')
print 'modify file cdir/empty_file'
os.system('echo modification Test > ./cdir/empty_file')

print 'sleep ' + str(60) + 's for your program to synchronize'
time.sleep(60)
print '================================================================='
[type_wrong, exist_wrong, content_wrong, mode_wrong, time_wrong, right]= diff_dir('./cdir','./sdir/user')
print 'result after inotify synchronization:'
print 'files that only exist in one side:'
if len(exist_wrong) == 0:
	print 'None'
else:
	print str(exist_wrong).decode('string_escape')
print 'files that have inconsistent type:'
if len(type_wrong) == 0:
	print 'None'
else:
	print str(type_wrong).decode('string_escape')
print 'files that have inconsistent content:'
if len(content_wrong) == 0:
	print 'None'
else:
	print str(content_wrong).decode('string_escape')
print 'files that have inconsistent permission:'
if len(mode_wrong) == 0:
	print 'None'
else:
	print str(mode_wrong).decode('string_escape')
print 'files that have inconsisten modification time:'
if len(time_wrong) == 0:
	print 'None'
else:
	print str(time_wrong).decode('string_escape')
print 'files that are consistent:'
if len(right) == 0:
	print 'None'
else:
	print str(right).decode('string_escape')
os.kill(pid_port_register, signal.SIGKILL)
os.kill(pid_server, signal.SIGKILL)
os.kill(pid_child, signal.SIGKILL)


