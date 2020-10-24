#! /usr/bin/env python

# Test network throughput.
#
# Usage:
# 1) on host_A: throughput -s [port]                    # start a server
# 2) on host_B: throughput -c time_sec host_A [port]      # start a client
#
# The server will service multiple clients until it is killed.
#
# The client performs one transfer of count*BUFSIZE bytes and
# measures the time it takes (roundtrip!).


import sys, time
from socket import *
from socket import error as socket_error

MY_PORT = 50000 + 42

BUFSIZE = 1024


def server(port=MY_PORT):

    s = socket(AF_INET, SOCK_STREAM)
    s.bind(('', port))
    s.listen(1)
    print 'Server ready...'
    while 1:
        try:
            conn, (host, remoteport) = s.accept()
            while 1:
                data = conn.recv(BUFSIZE)
                if not data:
                    break
                del data
            conn.send('OK\n')
            conn.close()
            print 'Done with', host, 'port', remoteport
        except socket_error as serr:
            print 'pass'
            pass


def client(host, duration, port=MY_PORT):

    testdata = 'x' * (BUFSIZE-1) + '\n'
    t1 = time.time()
    s = socket(AF_INET, SOCK_STREAM)
    t2 = time.time()
    try:
    	s.connect((host, port))
    except:
    	raise socket.error
    t3 = time.time()
    count = 0
    t0 = time.time()
    while time.time() - t0 < duration:
        count += 1
        s.send(testdata)
    # print 'Sent:', count, 'packets'
    # print 'Time used:', time.time() - t0
    s.shutdown(1) # Send EOF
    t4 = time.time()
    data = s.recv(BUFSIZE)
    t5 = time.time()
    # print data
    # print 'Raw timers:', t1, t2, t3, t4, t5
    # print 'Intervals:', t2-t1, t3-t2, t4-t3, t5-t4
    # print 'Total:', t5-t1
    # print 'Throughput:', round((BUFSIZE*count*0.001) / (t5-t1), 3),
    # print 'KB/sec.'
    return round((BUFSIZE*count*0.001) / (t5-t1), 3)

