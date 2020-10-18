"Check for Python Support"
if !has('python')
    com! -nargs=* CoVim echoerr "Error: CoVim requires vim compiled with +python"
    finish
endif

com! -nargs=* CoVim py CoVim.command(<f-args>)

"Needs to be set on connect, MacVim overrides otherwise"
function! SetCoVimColors ()
    hi CursorUser gui=bold term=bold cterm=bold
    hi Cursor1 ctermbg=DarkRed ctermfg=White guibg=DarkRed guifg=White gui=bold term=bold cterm=bold
    hi Cursor2 ctermbg=DarkBlue ctermfg=White guibg=DarkBlue guifg=White gui=bold term=bold cterm=bold
    hi Cursor3 ctermbg=DarkGreen ctermfg=White guibg=DarkGreen guifg=White gui=bold term=bold cterm=bold
    hi Cursor4 ctermbg=DarkCyan ctermfg=White guibg=DarkCyan guifg=White gui=bold term=bold cterm=bold
    hi Cursor5 ctermbg=DarkMagenta ctermfg=White guibg=DarkMagenta guifg=White gui=bold term=bold cterm=bold
    hi Cursor6 ctermbg=Brown ctermfg=White guibg=Brown guifg=White gui=bold term=bold cterm=bold
    hi Cursor7 ctermbg=LightRed ctermfg=Black guibg=LightRed guifg=Black gui=bold term=bold cterm=bold
    hi Cursor8 ctermbg=LightBlue ctermfg=Black guibg=LightBlue guifg=Black gui=bold term=bold cterm=bold
    hi Cursor9 ctermbg=LightGreen ctermfg=Black guibg=LightGreen guifg=Black gui=bold term=bold cterm=bold
    hi Cursor10 ctermbg=LightCyan ctermfg=Black guibg=LightCyan guifg=Black gui=bold term=bold cterm=bold
    hi Cursor0 ctermbg=LightYellow ctermfg=Black guibg=LightYellow guifg=Black gui=bold term=bold cterm=bold
endfunction

if !exists("CoVim_default_name")
    let CoVim_default_name = 0
endif
if !exists("CoVim_default_port")
    let CoVim_default_port = 0
endif

python << EOF

import argparse
import base64
import binascii
import re
import json
import json
import os
import random
import socket
import socket
import struct
import sys
import time
import vim
import warnings

from threading import Thread, Event
from twisted.internet.protocol import ClientFactory, Factory, Protocol
from twisted.internet import reactor


# Ignore Warnings
warnings.filterwarnings('ignore', '.*', UserWarning)
warnings.filterwarnings('ignore', '.*', DeprecationWarning)


# Find the server path
CoVimServerPath = vim.eval('expand("<sfile>:h")') + '/CoVimServer.py'
STOP = Event()

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
  

def encode(pub_addr, pub_port, priv_addr, priv_port):
  return ','.join([pub_addr, str(pub_port), priv_addr, str(priv_port)])

def connect(local_addr, addr, bind_local=True):
    debug('waiting to connecting...\n')
    debug('{}:{}\n{}:{}\n'.format(type(local_addr[0]), type(local_addr[1]),type( addr[0]), type(addr[1])))
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    if bind_local:
      s.bind(local_addr)
    while not STOP.is_set():
        try:
            s.connect(addr)
        except socket.error as e:
            continue
        else:
            return s

def debug(x):
  f = open('log', 'a+')
  f.write(x)
  f.close()

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
    if self.state == "GETNAME":
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


## CoVim Protocol
class CoVimProtocol(Protocol):
    def __init__(self, fact):
        self.fact = fact

    def encode(key, clear):
        enc = []
        for i in range(len(clear)):
            key_c = key[i % len(key)]
            enc_c = chr((ord(clear[i]) + ord(key_c)) % 256)
            enc.append(enc_c)
        return base64.urlsafe_b64encode("".join(enc))

    def decode(key, enc):
        dec = []
        enc = base64.urlsafe_b64decode(enc)
        for i in range(len(enc)):
            key_c = key[i % len(key)]
            dec_c = chr((256 + ord(enc[i]) - ord(key_c)) % 256)
            dec.append(dec_c)
        return "".join(dec)

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
                    print 'Success! You\'re now connected!'
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


#Manage Collaborators
class CollaboratorManager:

    def __init__(self):
        self.collab_id_itr = 4
        self.reset()

    def reset(self):
        self.collab_color_itr = 1
        self.collaborators = {}
        self.buddylist_highlight_ids = []

    def addUser(self, user_obj):
            if user_obj['name'] == CoVim.username:
                self.collaborators[user_obj['name']] = ('CursorUser', 4000)
            else:
                self.collaborators[user_obj['name']] = ('Cursor' + str(self.collab_color_itr), self.collab_id_itr)
                self.collab_id_itr += 1
                self.collab_color_itr = (self.collab_id_itr-3) % 11
                vim.command(':call matchadd(\''+self.collaborators[user_obj['name']][0]+'\', \'\%' + str(user_obj['cursor']['x']) + 'v.\%'+str(user_obj['cursor']['y'])+'l\', 10, ' + str(self.collaborators[user_obj['name']][1]) + ')')
            self.refreshCollabDisplay()

    def remUser(self, name):
        vim.command('call matchdelete('+str(self.collaborators[name][1]) + ')')
        del(self.collaborators[name])
        self.refreshCollabDisplay()

    def refreshCollabDisplay(self):
        buddylist_window_width = int(vim.eval('winwidth(0)'))
        CoVim.buddylist[:] = ['']
        x_a = 1
        line_i = 0
        vim.command("1wincmd w")
        for match_id in self.buddylist_highlight_ids:
            vim.command('call matchdelete('+str(match_id) + ')')
        self.buddylist_highlight_ids = []
        for name in self.collaborators.keys():
            x_b = x_a + len(name)
            if x_b > buddylist_window_width:
                line_i += 1
                x_a = 1
                x_b = x_a + len(name)
                CoVim.buddylist.append('')
                vim.command('resize '+str(line_i+1))
            CoVim.buddylist[line_i] += name+' '
            self.buddylist_highlight_ids.append(vim.eval('matchadd(\''+self.collaborators[name][0]+'\',\'\%<'+str(x_b)+'v.\%>'+str(x_a)+'v\%'+str(line_i+1)+'l\',10,'+str(self.collaborators[name][1]+2000)+')'))
            x_a = x_b + 1
        vim.command("wincmd p")


#Manage all of CoVim
class CoVimScope:

    def init(self, port):
      pass
      # TOOD

    def get_key(self):
      self.pub_addr, self.pub_port, self.priv_addr, self.priv_port, self.ss = get_ip_info()
      self.key = encode(self.pub_addr, self.pub_port, self.priv_addr, self.priv_port)
      return self.key

    def get_connection(self, server_key):

        server_pub_addr, server_pub_port, server_priv_addr, server_priv_port = decode(server_key)
        # behind NAT, same subnet as the server
        if self.priv_addr != self.pub_addr and self.pub_addr == server_pub_addr:
          return connect((self.priv_addr, self.priv_port), (server_priv_addr, server_priv_port))
        return connect((self.priv_addr, self.priv_port), (server_pub_addr, server_pub_port))

    def initiate(self, server_key, name, sock=None):
    
        #Check if connected. If connected, throw error.
        if hasattr(self, 'fact') and self.fact.isConnected:
            print 'ERROR: Already connected. Please disconnect first'
            return

        vim.command('autocmd VimLeave * py CoVim.quit()')
        if not hasattr(self, 'connection'):

            self.username = name
            self.vim_buffer = []
            self.fact = CoVimFactory()
            self.collab_manager = CollaboratorManager()
            print('Connecting...\n')
            # self.connection hold the socket
            if sock is not None:
              self.connection = sock
            else:
              self.connection = self.get_connection(server_key)
            # socket must be set to nonblocking
            self.connection.setblocking(False)
            # self.tcp_serer has type of twisted.internet.tcp.Server
            self.tcp_server = reactor.adoptStreamConnection(self.connection.fileno(), socket.AF_INET, self.fact)
            # close the socket
            self.connection.close()
            if not reactor.running:
                self.reactor_thread = Thread(target=reactor.run, args=(False,))
                self.reactor_thread.start()
            self.connection.close()
            print('Connected\n')
            debug('Connected\n')
            self.fact.isConnected=True

       #elif (hasattr(self, 'port') and port != self.port) or (hasattr(self, 'addr') and addr != self.addr):
       #    print 'ERROR: Different address/port already used. To try another, you need to restart Vim'
       #else:
       #    self.collab_manager.reset()
       #    self.connection.connect()
       #    print 'Reconnecting...'

    def createServer(self, name, port):

        self.get_key()
        debug(self.key)
        # setup the server
        # vim.command(':silent execute "!python2 '+CoVimServerPath+' '+str(port)+' &>/dev/null &"')
        # get myself connected(only the host should execute createServer)
        sock = connect(('localhost', self.priv_port), ('localhost', port), bind_local=False)
        self.host_key = self.key = recv_msg(sock).decode('utf-8')
        self.initiate(None, name, sock)
        time.sleep(.5)
        print 'server_key {}'.format(self.host_key)

    def setupWorkspace(self):
        vim.command('call SetCoVimColors()')
        vim.command(':autocmd!')
        vim.command('autocmd CursorMoved <buffer> py reactor.callFromThread(CoVim.fact.cursor_update)')
        vim.command('autocmd CursorMovedI <buffer> py reactor.callFromThread(CoVim.fact.buff_update)')
        vim.command('autocmd VimLeave * py CoVim.quit()')
        vim.command("1new +setlocal\ stl=%!'CoVim-Collaborators'")
        self.buddylist = vim.current.buffer
        self.buddylist_window = vim.current.window
        vim.command("wincmd j")

    def addUsers(self, list):
        map(self.collab_manager.addUser, list)

    def remUser(self, name):
        self.collab_manager.remUser(name)

    def refreshCollabDisplay(self):
        self.collab_manager.refreshCollabDisplay()

    def command(self, arg1=False, arg2=False, arg3=False):

        default_port = 8555
        default_name = 'HOST'

        if arg1 == "connect":
            if arg2 and arg3:
                self.initiate(arg2, arg3)
            elif arg2 and default_name != '0':
                self.initiate(arg2, default_name)
            else:
                print "usage :CoVim connect [host_key] [name]"

        if arg1 == "disconnect":
            self.disconnect()
        elif arg1 == "quit":
            self.exit()
        elif arg1 == "start":
            if not hasattr(self, 'key'):
                self.key = self.get_key()
            print self.key
        elif arg1 == 'add':
            self.fact.p.send('add{}'.format(arg2))
        elif arg1 == 'host':
            name = arg2 if arg2 else default_name
            port = int(arg3) if arg3 else default_port
            if name != '0' and port != '0':
              self.createServer(name, port)
            else:
                print "usage :CoVim host [name] [port]"
        else:
            print "usage: CoVim [host] [start] [connect] [disconnect] [quit]"

    def exit(self):
        if hasattr(self, 'buddylist_window') and hasattr(self, 'connection'):
            self.disconnect()
            vim.command('q!')
        else:
            print "ERROR: CoVim must be running to use this command"

    def disconnect(self):
        if hasattr(self, 'buddylist'):
            vim.command("1wincmd w")
            vim.command("q!")
            self.collab_manager.buddylist_highlight_ids = []
            for name in self.collab_manager.collaborators.keys():
                if name != CoVim.username:
                    vim.command(':call matchdelete('+str(self.collab_manager.collaborators[name][1]) + ')')
            del(self.buddylist)
        if hasattr(self, 'buddylist_window'):
            del(self.buddylist_window)
        if hasattr(self, 'connection'):
            reactor.callFromThread(self.tcp_server.abortConnection)
            self.fact.isConnected=False
            print 'Successfully disconnected from document!'
        else:
            print "ERROR: CoVim must be running to use this command"

    def quit(self):
        reactor.callFromThread(reactor.stop)

#####
# global variables
#####
CoVim = CoVimScope()

EOF
