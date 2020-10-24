#! /usr/bin/env python

# client collect metrics and report back to server

import sys, time
import subprocess
import urllib2
import datetime
import throughput as th

# modify this area
myname = 'client1'
names = ['server','client2','client4'] 
ips = ['139.162.41.153','192.168.31.192','140.112.250.25'] 
server_http = 'http://139.162.41.153:9091'



while True:

	for i in range(len(names)):
		ip = ips[i]
		name = names[i]
	
		ping = subprocess.Popen(["ping", "-c", "5", "-w", "5", ip], stdout=subprocess.PIPE)
		lines = ping.stdout.readlines()
		ping.stdout.close()
	
		if '100% packet loss' not in lines[-2]:	
			# reachability(%)
			pktloss = lines[-2].split(",")[2].split("%")[0]
	
			#latency(ms)
			avg_rtt = lines[-1].split("/")[4]
		else:
			pktloss = '100'
			avg_rtt = '-1'
	
		try:
			#throughput(KB/s), server has to run ./server
			throuhgput = str(th.client(ip, 10))
		except:
			throuhgput = '-1'

		print name, pktloss, avg_rtt, throuhgput
	
		# send metric to server
		data = myname+','+name+','+str(datetime.datetime.now()).split('.')[0]+','+pktloss+','+avg_rtt+','+throuhgput
		req = urllib2.Request(server_http, data)
		response = urllib2.urlopen(req)
        	
	time.sleep(40)
