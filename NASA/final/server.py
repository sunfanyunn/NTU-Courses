#! /usr/bin/env python

from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
from sys import argv
import SocketServer
import smtplib
import os.path

PORT=9091

# modify this area
path = "/var/www/html/nasa/ui/"
metric_filename = {}
metric_filename[('client1','client2')] = path + "metric0.in"
metric_filename[('client1','client4')] = path + "metric1.in"
metric_filename[('client1','server')] = path + "metric2.in"
metric_filename[('client2','client4')] = path + "metric3.in"
metric_filename[('client2','server')] = path + "metric4.in"
metric_filename[('client3','client4')] = path + "metric5.in"
metric_filename[('client3','server')] = path + "metric6.in"
metric_filename[('client4','server')] = path + "metric7.in"
metric_filename_header = "src,dst,time,packetloss,latency,throughput\n"
sented = dict.fromkeys(metric_filename, False)

class handler(BaseHTTPRequestHandler):	
	
	def send_mail(self, src, dst):
		mail_user = 'katmao1009@yandex.com'  
		mail_password = 'slgofjebgvwbhyob'
		sent_from = mail_user  
		to = ['hiwang123@gmail.com']

		email_text = "\r\n".join([
		"From: " + sent_from,
		"To: " + ",".join(to),
		"Subject: [NASA] Network metric error",
		"",
		"Connectivity between " + src + " and " + dst + " is poor or dead. Please Check it."
		])
		
		try:  
			server = smtplib.SMTP_SSL('smtp.yandex.com', 465)
			server.ehlo()
			server.login(mail_user, mail_password)
			server.sendmail(sent_from, to, email_text)
			server.close()
			print 'Email sent!'
			sented[(src, dst)] = True
		except:  
			print 'Something went wrong...'
	

	def do_POST(self):
		self.send_response(200)
		self.end_headers()
		content_length = int(self.headers['Content-Length'])
		post_data = self.rfile.read(content_length) # src,dst,pktloss,avg_rtt,throuhgput
		data_arr = post_data.split(',')
		src, dst = data_arr[0:2]
		pktloss = data_arr[3]
		throughput = data_arr[5]
		
		if (src, dst) not in metric_filename:
			return 
		
		#check metric between client link
		if (float(throughput) < 10):
			if not sented[(src, dst)]:
				self.send_mail(src, dst)
		else:
			sented[(src, dst)] = False
		
		#write to csv
		if not os.path.isfile(metric_filename[(src, dst)]):
			metric_file = open(metric_filename[(src, dst)], "a+")
			metric_file.write(metric_filename_header)
		else:
			metric_file = open(metric_filename[(src, dst)], "r")
			lines = metric_file.readlines()
			metric_file.close()
			metric_file = open(metric_filename[(src, dst)], "w")
			print len(lines)
			if len(lines) < 30 :
				metric_file.write("".join(lines))
			else:
				metric_file.write(lines[0])
				metric_file.write("".join(lines[2:]))
		metric_file.write(post_data+"\n")
		metric_file.close()
		

if __name__ == "__main__":

	try:
		server = HTTPServer(('', PORT), handler)
		print 'start server'

		server.serve_forever()

	except:
		server.socket.close()
