import random
import time

import json
import time
import argparse
import thread
import threading
import traceback

from ws4py.client.threadedclient import WebSocketClient
from cmd import Cmd


def parse_command(params):
    """Parses a command with a first string param and a second
    json-encoded param"""
    name, args = (params + ' ').split(' ', 1)
    return name, args and json.loads(args) or []



Matrix = [[0,0,0,0,0,0,0,0,0,0],[0,1,0,0,0,0,0,0,0,0],[0,1,0,0,0,0,0,1,1,0],
          [0,1,0,0,0,0,0,1,1,0],[0,1,0,0,0,0,0,1,1,0],[0,1,0,0,0,0,0,0,0,0],
          [0,1,0,0,0,0,0,0,0,0],[0,1,0,0,0,0,0,0,0,0],[0,1,0,0,0,0,0,0,0,0],
          [0,0,0,0,0,0,0,0,0,0]]
val = 0
min_val = 0.1
max_val = 0.8
loc= []
temp = []
radtemp = []
humid = []
velocity = []
for i,j in enumerate(Matrix):
   for k,l in enumerate(j):
     if l==0:
         loc.append((i,k))
#print loc

for m in range (0, len(loc)):
    temp.append(random.randint(20,30))
    
#print temp

for n in range (0, len(loc)):
    radtemp.append(random.randint(70,80))

#print radtemp

for o in range (0, len(loc)):
    humid.append(random.randint(20,50))

#print humid

for p in range (0, len(loc)):
    velocity.append(round(random.uniform(min_val, max_val), 1))

#print velocity
for q in range (0, len(loc)):
    command = "insertMap [{'loc':loc[q],'temp':temp[q],'radtemp':radtemp[q],'humid':humid[q],'velocity':velocity[q]}"
    method_name, params = parse_command(command)
    ddpclient = DDPClient(
    	'ws://' + "localhost:3000" + '/websocket',
    	args.print_raw)
    ddpclient.connect()
    ddpclient.send({
    	"msg": "method",
    	"method": method_name,
    	"params": params,
    	"id": "1"
    })
