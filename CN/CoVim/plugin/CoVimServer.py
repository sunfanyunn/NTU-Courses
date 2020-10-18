import os
import json
import sys
import warnings
from twisted.internet.protocol import ClientFactory, Protocol
from twisted.internet import reactor
from threading import Thread
from time import sleep


import re
import json
import argparse
import socket

from twisted.internet.protocol import Factory, Protocol
from twisted.internet import reactor

import binascii
import random
import socket
import struct
from threading import Event

# Ignore Warnings
warnings.filterwarnings('ignore', '.*', UserWarning)
warnings.filterwarnings('ignore', '.*', DeprecationWarning)

STOP = Event()

def send_msg(sock, msg):
    # Prefix each message with a 4-byte length (network byte order)
    msg = struct.pack('>I', len(msg)) + msg
    sock.sendall(msg)

def connect(local_addr, addr):
    print('waiting to connecting...\n')
    print(local_addr, addr)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    s.bind(local_addr)
    while not STOP.is_set():
        try:
            s.connect(addr)
        except socket.error as e:
            continue
        else:
            return s

def accept(port):
    print('Listening at', port)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    s.bind(('', port))
    s.listen(1)
    s.settimeout(5)
    while not STOP.is_set():
        try:
            conn, addr = s.accept()
        except socket.timeout as e:
            continue
        else:
            return conn

stun_servers_list= [
    ('stun.stunprotocol.org', 3478)
]

# stun attributes
MappedAddress = '0001'
SourceAddress = '0004'
ChangedAddress = '0005'

# types for a stun message
BindRequestMsg = '0001'
BindResponseMsg = '0101'

dictAttrToVal = {'MappedAddress': MappedAddress,
                 'SourceAddress': SourceAddress,
                 'ChangedAddress': ChangedAddress}

dictMsgTypeToVal = {
    'BindRequestMsg': BindRequestMsg,
    'BindResponseMsg': BindResponseMsg}

dictValToMsgType = {}

dictValToAttr = {}

def _initialize():
    items = dictAttrToVal.items()
    for i in range(len(items)):
        dictValToAttr.update({items[i][1]: items[i][0]})
    items = dictMsgTypeToVal.items()
    for i in range(len(items)):
        dictValToMsgType.update({items[i][1]: items[i][0]})


def gen_tran_id():
    a = ''.join(random.choice('0123456789ABCDEF') for i in range(32))
    return a


def stun_test(sock, host, port, send_data=""):
    retVal = {'Resp': False, 'ExternalIP': None, 'ExternalPort': None,
              'SourceIP': None, 'SourcePort': None, 'ChangedIP': None,
              'ChangedPort': None}
    str_len = "%#04d" % (len(send_data) / 2)
    tranid = gen_tran_id()
    str_data = ''.join([BindRequestMsg, str_len, tranid, send_data])
    data = binascii.a2b_hex(str_data)
    recvCorr = False
    while not recvCorr:
        recieved = False
        count = 3
        while not recieved:
            try:
                sock.send(data)
            except socket.gaierror:
                retVal['Resp'] = False
                return retVal
            try:
                buf = sock.recv(2048)
                recieved = True
            except Exception, e:
                recieved = False
                if count > 0:
                    count -= 1
                else:
                    retVal['Resp'] = False
                    return retVal
        if dictValToMsgType[binascii.b2a_hex(buf[0:2])] == "BindResponseMsg" and \
                                         tranid.upper() == binascii.b2a_hex(buf[4:20]).upper():
            recvCorr = True
            retVal['Resp'] = True
            len_message = int(binascii.b2a_hex(buf[2:4]), 16)
            len_remain = len_message
            base = 20
            while len_remain:
                attr_type = binascii.b2a_hex(buf[base:(base + 2)])
                attr_len = int(binascii.b2a_hex(buf[(base + 2):(base + 4)]), 16)
                if attr_type == MappedAddress:
                    port = int(binascii.b2a_hex(buf[base + 6:base + 8]), 16)
                    ip = ".".join([
                        str(int(binascii.b2a_hex(buf[base + 8:base + 9]), 16)),
                        str(int(binascii.b2a_hex(buf[base + 9:base + 10]), 16)),
                        str(int(binascii.b2a_hex(buf[base + 10:base + 11]), 16)),
                        str(int(binascii.b2a_hex(buf[base + 11:base + 12]), 16))
                    ])
                    retVal['ExternalIP'] = ip
                    retVal['ExternalPort'] = port
                if attr_type == SourceAddress:
                    port = int(binascii.b2a_hex(buf[base + 6:base + 8]), 16)
                    ip = ".".join([
                        str(int(binascii.b2a_hex(buf[base + 8:base + 9]), 16)),
                        str(int(binascii.b2a_hex(buf[base + 9:base + 10]), 16)),
                        str(int(binascii.b2a_hex(buf[base + 10:base + 11]), 16)),
                        str(int(binascii.b2a_hex(buf[base + 11:base + 12]), 16))
                    ])
                    retVal['SourceIP'] = ip
                    retVal['SourcePort'] = port
                if attr_type == ChangedAddress:
                    port = int(binascii.b2a_hex(buf[base + 6:base + 8]), 16)
                    ip = ".".join([
                        str(int(binascii.b2a_hex(buf[base + 8:base + 9]), 16)),
                        str(int(binascii.b2a_hex(buf[base + 9:base + 10]), 16)),
                        str(int(binascii.b2a_hex(buf[base + 10:base + 11]), 16)),
                        str(int(binascii.b2a_hex(buf[base + 11:base + 12]), 16))
                    ])
                    retVal['ChangedIP'] = ip
                    retVal['ChangedPort'] = port

                base = base + 4 + attr_len
                len_remain = len_remain - (4 + attr_len)

    return retVal


def get_ip_info(stun_host=None, stun_port=3478):

    host = '59.124.162.92'
    port = 5005
    sa = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sa.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sa.connect((host, port))
    priv_addr = sa.getsockname()

    send_msg(sa, addr_to_msg(priv_addr))
    data = recv_msg(sa)
    #logger.info("client %s %s - received data: %s", priv_addr[0], priv_addr[1], data)
    pub_addr = msg_to_addr(data)
    send_msg(sa, addr_to_msg(pub_addr))
    return pub_addr[0].encode('utf-8'), pub_addr[1], priv_addr[0].encode('utf-8'), priv_addr[1], sa

import struct
from collections import namedtuple

def addr_from_args(args, host='127.0.0.1', port=9999):
    if len(args) >= 3:
        host, port = args[1], int(args[2])
    elif len(args) == 2:
        host, port = host, int(args[1])
    else:
        host, port = host, port
    return host, port


def msg_to_addr(data):
    ip, port = data.decode('utf-8').strip().split(':')
    return (ip, int(port))


def addr_to_msg(addr):
    return '{}:{}'.format(addr[0], str(addr[1])).encode('utf-8')


def send_msg(sock, msg):
    # Prefix each message with a 4-byte length (network byte order)
    msg = struct.pack('>I', len(msg)) + msg
    sock.sendall(msg)


def recvall(sock, n):
    # Helper function to recv n bytes or return None if EOF is hit
    data = b''
    while len(data) < n:
        packet = sock.recv(n - len(data))
        if not packet:
            return None
        data += packet
    return data


def recv_msg(sock):
    # Read message length and unpack it into an integer
    raw_msglen = recvall(sock, 4)
    if not raw_msglen:
        return None
    msglen = struct.unpack('>I', raw_msglen)[0]
    # Read the message data
    return recvall(sock, msglen)

def main(mykey, key):
  host = '59.124.162.92'
  port = 5005
  sa = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  sa.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  sa.connect((host, port))
  priv_addr = sa.getsockname()

  send_msg(sa, addr_to_msg(priv_addr))
  data = recv_msg(sa)
  #logger.info("client %s %s - received data: %s", priv_addr[0], priv_addr[1], data)
  pub_addr = msg_to_addr(data)
  send_msg(sa, addr_to_msg(pub_addr))
  return pub_addr[0], pub_addr[1], priv_addr[0], priv_addr[1], sa

  data = recv_msg(sa)
  pubdata, privdata = data.split(b'|')
  client_pub_addr = msg_to_addr(pubdata)
  client_priv_addr = msg_to_addr(privdata)
  #logger.info(
  #    "client public is %s and private is %s, peer public is %s private is %s",
  #    pub_addr, priv_addr, client_pub_addr, client_priv_addr,
  #)
  return connect(priv_addr, client_priv_addr)

def encode(pub_addr, pub_port, priv_addr, priv_port):
  return ','.join([pub_addr, str(pub_port), priv_addr, str(priv_port)])

def decode(s):
  x = s.split(',')
  return x[0], int(x[1]), x[2], int(x[3])

def name_validate(strg, search=re.compile(r'[^0-9a-zA-Z\-\_]').search):
  return not bool(search(strg))

class React(Protocol):

  def __init__(self, factory):
    self.factory = factory
    self.state = "GETNAME"

  def dataReceived(self, data):
    print(data)
    if data.startswith('add'):
        idx = data.find('{')
        if idx == -1:
            server.add_connection(data[3:])
        else:
            server.add_connection(data[3:idx])
            self.handle_BUFF(data[idx:])
    elif self.state == "GETNAME":
      self.handle_GETNAME(data)
    else:
      self.handle_BUFF(data)

  def handle_GETNAME(self, name):
    # Handle duplicate name
    if userManager.has_user(name):
      d = {
        'packet_type': 'message',
        'data': {
          'message_type': 'error_newname_taken'
        }
      }
      self.transport.write(json.dumps(d))
      return

    # Handle spaces in name
    if not name_validate(name):
      d = {
        'packet_type': 'message',
        'data': {
          'message_type': 'error_newname_invalid'
        }
      }
      self.transport.write(json.dumps(d))
      return

    # Name is Valid, Add to Document
    self.user = User(name, self)
    userManager.add_user(self.user)
    self.state = "CHAT"
    d = {
      'packet_type': 'message',
      'data': {
        'message_type': 'connect_success',
        'name': name,
        'collaborators': userManager.all_users_to_json()
      }
    }

    if userManager.is_multi():
      d['data']['buffer'] = self.factory.buff
    self.transport.write(json.dumps(d))
    print 'User "{user_name}" Connected'.format(user_name=self.user.name)

    # Alert other Collaborators of new user
    d = {
      'packet_type': 'message',
      'data': {
        'message_type': 'user_connected',
        'user': self.user.to_json()
      }
    }
    self.user.broadcast_packet(d)

  def handle_BUFF(self, data_string):
    def to_utf8(d):
      if isinstance(d, dict):
        # no dict comprehension in python2.5/2.6
        d2 = {}
        for key, value in d.iteritems():
          d2[to_utf8(key)] = to_utf8(value)
        return d2
      elif isinstance(d, list):
        return map(to_utf8, d)
      elif isinstance(d, unicode):
        return d.encode('utf-8')
      else:
        return d

    def clean_data_string(d_s):
      bad_data = d_s.find("}{")
      if bad_data > -1:
        d_s = d_s[:bad_data+1]
      return d_s

    data_string = clean_data_string(data_string)
    d = to_utf8(json.loads(data_string))
    data = d['data']
    update_self = False

    if 'cursor' in data.keys():
      user = userManager.get_user(data['name'])
      user.update_cursor(data['cursor']['x'], data['cursor']['y'])
      d['data']['updated_cursors'] = [user.to_json()]
      del d['data']['cursor']

    if 'buffer' in data.keys():
      b_data = data['buffer']
      #TODO: Improve Speed: If change_y = 0, just replace that one line
      #print ' \\n '.join(self.factory.buff[:b_data['start']])
      #print ' \\n '.join(b_data['buffer'])
      #print ' \\n '.join(self.factory.buff[b_data['end']-b_data['change_y']+1:])
      self.factory.buff = self.factory.buff[:b_data['start']]   \
                          + b_data['buffer']                    \
                          + self.factory.buff[b_data['end']-b_data['change_y']+1:]
      d['data']['updated_cursors'] += userManager.update_cursors(b_data, user)
      update_self = True
    self.user.broadcast_packet(d, update_self)

  def connectionLost(self, reason):
    if hasattr(self, 'user'):
      userManager.rem_user(self.user)
      if userManager.is_empty():
        print 'All users disconnected. Shutting down...'
        reactor.stop()


class ReactFactory(Factory):

  def __init__(self):
    self.buff = []

  def buildProtocol(self, addr):
    return React(self)


class Cursor:
  def __init__(self):
    self.x = 1
    self.y = 1

  def to_json(self):
    return {
      'x': self.x,
      'y': self.y
    }


class User:
  def __init__(self, name, protocol):
    self.name = name
    self.protocol = protocol
    self.cursor = Cursor()

  def to_json(self):
    return {
      'name': self.name,
      'cursor': self.cursor.to_json()
    }

  def broadcast_packet(self, obj, send_to_self=False):
    obj_json = json.dumps(obj)
    #print obj_json
    for name, user in userManager.users.iteritems():
      if user.name != self.name or send_to_self:
        user.protocol.transport.write(obj_json)
        #TODO: don't send yourself your own buffer, but del on a copy doesn't work

  def update_cursor(self, x, y):
    self.cursor.x = x
    self.cursor.y = y


class UserManager:

  def __init__(self):
    self.users = {}

  def is_empty(self):
    return not self.users

  def is_multi(self):
    return len(self.users) > 1

  def has_user(self, search_name):
    return self.users.get(search_name)

  def add_user(self, u):
    self.users[u.name] = u

  def get_user(self, u_name):
    try:
      return self.users[u_name]
    except KeyError:
      raise Exception('user doesnt exist')

  def rem_user(self, user):
    if self.users.get(user.name):
      d = {
        'packet_type': 'message',
        'data': {
          'message_type': 'user_disconnected',
          'name': user.name
        }
      }
      user.broadcast_packet(d)
      print 'User "{user_name}" Disconnected'.format(user_name=user.name)
      del self.users[user.name]

  def all_users_to_json(self):
    return [user.to_json() for user in userManager.users.values()]

  def update_cursors(self, buffer_data, u):
    return_arr = []
    y_target = u.cursor.y
    x_target = u.cursor.x

    for user in userManager.users.values():
      updated = False
      if user != u:
        if user.cursor.y > y_target:
          user.cursor.y += buffer_data['change_y']
          updated = True
        if user.cursor.y == y_target and user.cursor.x > x_target:
          user.cursor.x = max(1, user.cursor.x + buffer_data['change_x'])
          updated = True
        if user.cursor.y == y_target - 1 and user.cursor.x > x_target \
           and buffer_data['change_y'] == 1:
          user.cursor.y += 1
          user.cursor.x = max(1, user.cursor.x + buffer_data['change_x'])
          updated = True
        #TODO: If the line was just split?
        if updated:
          return_arr.append(user.to_json())
    return return_arr

class Server:

  def __init__(self, port=None):
    # self.priv_port = port
    self.connections = []
    self.pub_addr, self.pub_port, self.priv_addr, self.priv_port, self.ss = get_ip_info()
    self.key = encode(self.pub_addr, self.pub_port, self.priv_addr, self.priv_port)
    print('server_key {}'.format(self.key))

  def add_connection(self, client_key, local=False):

    try:
        peer_pub_addr, peer_pub_port, peer_priv_addr, peer_priv_port = decode(client_key)
    except Exception as e:
        print(e)
        print('Bad format!')
        print(client_key)
        return

    if self.priv_addr != self.pub_addr and self.pub_addr == peer_pub_addr:
      sock = connect((self.priv_addr, self.priv_port), (peer_priv_addr, peer_priv_port))
    else:
      sock = connect((self.priv_addr, self.priv_port), (peer_pub_addr, peer_pub_port))

    sock.setblocking(False)
    reactor.adoptStreamConnection(sock.fileno(), socket.AF_INET, ReactFactory())
    sock.close()
    self.connections.append(sock)
    print('New user added')


def debug(x):
  f = open('log', 'a+')
  f.write(x)
  f.close()


## CoVim Protocol
class CoVimProtocol(Protocol):
    def __init__(self, fact):
        self.fact = fact

    def send(self, event):
        self.transport.write(event)

    def connectionMade(self):
        self.send(CoVim.username)

    def dataReceived(self, data_string):
        def to_utf8(d):
            if isinstance(d, dict):
                # no dict comprehension in python2.5/2.6
                d2 = {}
                for key, value in d.iteritems():
                    d2[to_utf8(key)] = to_utf8(value)
                return d2
            elif isinstance(d, list):
                return map(to_utf8, d)
            elif isinstance(d, unicode):
                return d.encode('utf-8')
            else:
                return d

        def clean_data_string(d_s):
            bad_data = d_s.find("}{")
            if bad_data > -1:
                d_s = d_s[:bad_data+1]
            return d_s

        data_string = clean_data_string(data_string)
        packet = to_utf8(json.loads(data_string))
        if 'packet_type' in packet.keys():
            data = packet['data']
            if packet['packet_type'] == 'message':
                if data['message_type'] == 'error_newname_taken':
                    CoVim.disconnect()
                    print 'ERROR: Name already in use. Please try a different name'
                if data['message_type'] == 'error_newname_invalid':
                    CoVim.disconnect()
                    print 'ERROR: Name contains illegal characters. Only numbers, letters, underscores, and dashes allowed. Please try a different name'
                if data['message_type'] == 'connect_success':
                    CoVim.setupWorkspace()
                    if 'buffer' in data.keys():
                        CoVim.vim_buffer = data['buffer']
                        vim.current.buffer[:] = CoVim.vim_buffer
                    CoVim.addUsers(data['collaborators'])
                    print 'Success! You\'re now connected [Port '+str(CoVim.port)+']'
                if data['message_type'] == 'user_connected':
                    CoVim.addUsers([data['user']])
                    print data['user']['name']+' connected to this document'
                if data['message_type'] == 'user_disconnected':
                    CoVim.remUser(data['name'])
                    print data['name']+' disconnected from this document'
            if packet['packet_type'] == 'update':
                if 'buffer' in data.keys() and data['name'] != CoVim.username:
                    b_data = data['buffer']
                    CoVim.vim_buffer = vim.current.buffer[:b_data['start']]   \
                                                         + b_data['buffer']   \
                                                         + vim.current.buffer[b_data['end']-b_data['change_y']+1:]
                    vim.current.buffer[:] = CoVim.vim_buffer
                if 'updated_cursors' in data.keys():
                    # We need to update your own cursor as soon as possible, then update other cursors after
                    for updated_user in data['updated_cursors']:
                        if CoVim.username == updated_user['name'] and data['name'] != CoVim.username:
                            vim.current.window.cursor = (updated_user['cursor']['y'], updated_user['cursor']['x'])
                    for updated_user in data['updated_cursors']:
                        if CoVim.username != updated_user['name']:
                            vim.command(':call matchdelete('+str(CoVim.collab_manager.collaborators[updated_user['name']][1]) + ')')
                            vim.command(':call matchadd(\''+CoVim.collab_manager.collaborators[updated_user['name']][0]+'\', \'\%' + str(updated_user['cursor']['x']) + 'v.\%'+str(updated_user['cursor']['y'])+'l\', 10, ' + str(CoVim.collab_manager.collaborators[updated_user['name']][1]) + ')')
                #data['cursor']['x'] = max(1,data['cursor']['x'])
                #print str(data['cursor']['x'])+', '+str(data['cursor']['y'])
            vim.command(':redraw')


#CoVimFactory - Handles Socket Communication
class CoVimFactory(ClientFactory):

    def startedConnecting(self, connector):
        pass

    def buildProtocol(self, addr):
        self.p = CoVimProtocol(self)
        return self.p

    def startFactory(self):
        # CAREFUL, will not be called
        self.isConnected = True

    def stopFactory(self):
        # CAREFUL, will not be called
        self.isConnected = False

    def buff_update(self):
        d = {
            "packet_type": "update",
            "data": {
                "cursor": {
                    "x": max(1, vim.current.window.cursor[1]),
                    "y": vim.current.window.cursor[0]
                },
                "name": CoVim.username
            }
        }
        d = self.create_update_packet(d)
        data = json.dumps(d)
        self.p.send(data)

    def cursor_update(self):
        d = {
            "packet_type": "update",
            "data": {
                "cursor": {
                    "x": max(1, vim.current.window.cursor[1]+1),
                    "y": vim.current.window.cursor[0]
                },
                "name": CoVim.username
            }
        }
        d = self.create_update_packet(d)
        data = json.dumps(d)
        self.p.send(data)

    def create_update_packet(self, d):
        current_buffer = vim.current.buffer[:]
        if current_buffer != CoVim.vim_buffer:
            cursor_y = vim.current.window.cursor[0] - 1
            change_y = len(current_buffer) - len(CoVim.vim_buffer)
            change_x = 0
            if len(CoVim.vim_buffer) > cursor_y-change_y and cursor_y-change_y >= 0 \
                and len(current_buffer) > cursor_y and cursor_y >= 0:
                change_x = len(current_buffer[cursor_y]) - len(CoVim.vim_buffer[cursor_y-change_y])
            limits = {
                'from': max(0, cursor_y-abs(change_y)),
                'to': min(len(vim.current.buffer)-1, cursor_y+abs(change_y))
            }
            d_buffer = {
                'start': limits['from'],
                'end': limits['to'],
                'change_y': change_y,
                'change_x': change_x,
                'buffer': vim.current.buffer[limits['from']:limits['to']+1],
                'buffer_size': len(current_buffer)
            }
            d['data']['buffer'] = d_buffer
            CoVim.vim_buffer = current_buffer
        return d

    def clientConnectionLost(self, connector, reason):
        #THIS IS A HACK
        if hasattr(CoVim, 'buddylist'):
            CoVim.disconnect()
            print 'Lost connection.'

    def clientConnectionFailed(self, connector, reason):
        CoVim.disconnect()
        print 'Connection failed.'
#####
# global variables
#####

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Start a CoVim server.')
    parser.add_argument('-p', '--persist', action='store_true',
                    help='Keep server running if all users disconnect')
    parser.add_argument('port', type=int, nargs='?', default=8555,
                    help='Port number to run on')
    args = parser.parse_args()
    server = Server()
    userManager = UserManager()
    sock = accept(args.port)
    print('accept!')
    send_msg(sock, server.key.encode('utf-8'))
    sock.setblocking(False)
    reactor.adoptStreamConnection(sock.fileno(), socket.AF_INET, ReactFactory())
    sock.close()
    print('Accept connection\n')
    print(server.key)
    # Thread(target=reactor.run, args=(False,)).start()
    reactor.run()
