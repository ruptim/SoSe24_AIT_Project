#!/usr/bin/env python3

# SPDX-FileCopyrightText: Christian Amsüss and the aiocoap contributors
#
# SPDX-License-Identifier: MIT

"""This is a usage example of aiocoap that demonstrates how to implement a
simple server. See the "Usage Examples" section in the aiocoap documentation
for some more information."""

from datetime import datetime
import logging

import json

from threading import Thread
from time import time, sleep

import asyncio

import aiocoap.resource as resource
from aiocoap.numbers.contentformat import ContentFormat
from aiocoap import *
import aiocoap

import aiocoap.resourcedirectory.client.register  as rd_register

from queue import Queue

import re


from threading import Thread
import zmq


### logging setup
logging.basicConfig(level=logging.INFO)
# logging.getLogger("coap-server").setLevel(logging.DEBUG)



UI_BACKEND_SUB_ADDR = "tcp://localhost:5556"
BUZZER_BACKEND_PUB_ADDR = "tcp://*:5555"
MULTICAST_URI_BASE = "coap://[ff02::2%lowpan0]"
HEARTBEAT_INTERVAL_S = 4



paring_mode_enabled = False ### change to False
max_device_num_lock = asyncio.Lock()
max_device_num = 0


device_status_map = {}
device_status_map_mutex = asyncio.Lock()
device_heartbeat_map = {}
device_heartbeat_map_mutex = asyncio.Lock()



DEBUG_DONT_SEND_ZMQ = False

class DataSender():
    """
        Wrapper class to send messages to the ui backend via zeromq.
    """

    def __init__(self) -> None:
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.PUB)
        self.socket.bind(BUZZER_BACKEND_PUB_ADDR)

    def send(self,data_dict):
        self.socket.send_json(data_dict)

    async def _send_buzzer_list(self):
        data = []
        async with device_status_map_mutex:
            for (k,v) in device_status_map.items():
                data.append({
                    "buzzerId":v['device_num'],                    
                    "buzzerName":k,
                    "islocked":v['locked'],
                    "timestamp":v['timestamp']
                    })
        self.send(data)

    def send_buzzer_info(self):
        
        if not DEBUG_DONT_SEND_ZMQ:
            asyncio.create_task(self._send_buzzer_list())
        
ui_backend_sender = DataSender()


def backend_message_receiver():
    """
        Routine to receiver messages from the ui backend via zeromq.
    """
    global paring_mode_enabled

    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect(UI_BACKEND_SUB_ADDR)
    socket.setsockopt(zmq.SUBSCRIBE, b'')
   
    while True:        
        msg = socket.recv()
        topic_and_content =  re.findall(r'([^,]+)',msg.decode('ascii'))
        if(topic_and_content[0] == 'reset'):
            print("Reset buzzers")
            asyncio.run(reset_buzzers())

        elif(topic_and_content[0] == 'pairing'):
            paring_mode_enabled = topic_and_content[1].strip() == "True"
            print(f"Pairing mode: {paring_mode_enabled}")
            
        elif(topic_and_content[0] == 'remove'):
            key = topic_and_content[1].strip()
            buzzer = device_status_map[key]
            asyncio.run(remove_old_buzzers([(key, buzzer)]))



class ButtonRegisterResource(resource.Resource):

    def __init__(self):
        super().__init__()

    async def send_registered_name(self,ep):
        """
           Generate a buzzer name for request and create entries in 'device_status_map' and 'device_heartbeat_map'.
           Respond with CREATED and the name as payload.
        """
        print(f"REQ: {ep}")
        device_num = -2
        async with max_device_num_lock:
            global max_device_num
            device_num = max_device_num
            max_device_num+=1
        register_name = f"buzzer{device_num}"
        
       
        
        async with device_status_map_mutex:
            device_status_map[register_name] = {
                'device_num': device_num,
                'endpoint': ep,
                'register_time': datetime.now(),
                'timestamp': None,
                'mutex': asyncio.locks.Lock(),
                "locked": "True"

            }
            print(f"ADDED NEW BUZZER {register_name}")

            async with device_heartbeat_map_mutex:
                device_heartbeat_map[register_name] = {"last_heartbeat":time() + HEARTBEAT_INTERVAL_S}

            ui_backend_sender.send_buzzer_info()
            
        
        return aiocoap.Message(code=aiocoap.CREATED,payload=register_name.encode('ascii'))

    
    async def register_device(self, request):
        """
           Accept register request if pairing mode is enabled.           
           If the buzzer has send a name with its request, check if an entry exists else register as new.
           If pairing is not allowed, respond with FORBIDDEN.
        """
        if paring_mode_enabled:
            payload = request.payload.decode('ascii')
            if len(payload): 
                entry = None
                async with device_status_map_mutex:
                    entry = device_status_map.get(payload)

                # if the re-registration comes from the correct endpoint: accept
                if entry and entry['endpoint'] == request.remote.uri_base:
                    return aiocoap.Message(code=aiocoap.CHANGED,payload="0".encode("ascii"))    

            # if requested name is not found, send new one                

            return await self.send_registered_name(request.remote.uri_base) 
        else:
            print("FORBIDDEN")
            return aiocoap.Message(code=aiocoap.FORBIDDEN)


    
    async def render_put(self, request):
        # print('PUT payload: %s' % request.payload)
        return await self.register_device(request)
     

class ButtonPressedResource(resource.Resource):
    """ 
        Resource for endpoints to write to if theire connected buzzer-button was pressed.
    """ 

    def __init__(self):
        super().__init__()

    async def set_content(self, content):
        """
            Extract buzzer name and timestamp from request and store it in device_status_map.

            Expected content format:
                device_name,time_stamp (ISO 8601) 
            Example:
                buzzer1,2024-07-01T15:45:44.585110
        """

        matches = re.findall(r'([^,]+)',content)

        device_id = matches[0]
        # time_stamp = datetime.fromisoformat(matches[1])
        time_stamp = matches[1]
        
        async with device_status_map_mutex:
            if (device_status_map.get(device_id)):
                async with device_status_map[device_id]['mutex']:
                    device_status_map[device_id]['timestamp'] = time_stamp
                    device_status_map[device_id]['locked'] = "True"
        #TODO: else: send error response!! 
        ui_backend_sender.send_buzzer_info()

    async def render_put(self, request):
        print('PUT payload: %s' % request.payload)
        asyncio.create_task(self.set_content(request.payload.decode('ascii')))

        return aiocoap.Message(code=aiocoap.CREATED)

class HeartbeatResource(resource.Resource):
    """ 
        Resource for endpoints to send a heartbeat to.
    """ 

    def __init__(self):
        super().__init__()

    async def render_put(self, request):
        """
            Receiver heartbeat of buzzer.
            If buzzer is not registerd, respond with FORBIDDEN else 
            get current time and respond with CREATED.
        """
        device_id = request.payload.decode("utf-8")
        async with device_heartbeat_map_mutex:
            if (device_heartbeat_map.get(device_id)):
                device_heartbeat_map[device_id] = {'last_heartbeat':time()+HEARTBEAT_INTERVAL_S} 
                
                return aiocoap.Message(code=aiocoap.CREATED)
            else:
                return aiocoap.Message(code=aiocoap.FORBIDDEN)



"""
    Function to send a CoAP PUT message to a given uri with given payload.
"""
async def send_data(ctx,uri,payload):
    request = aiocoap.Message(code=aiocoap.PUT, uri=uri,payload=payload)
    requester = ctx.request(request)
    resp = await requester.response
    return resp

"""
    Function to send a multicast CoAP PUT message to multicast uri and given resource path with given payload.
"""
async def send_data_multicast(path,payload):
    ctx = await aiocoap.Context.create_client_context()
    request = aiocoap.Message(code=aiocoap.PUT, mtype=aiocoap.NON, uri=f"{MULTICAST_URI_BASE}/{path}",payload=payload)
    requester = ctx.request(request)
    resp = await requester.response
    return resp


"""
    Send a PUT request to reset-uris of a given device.
"""
async def send_reset_to_buzzer(key, device):
    ctx = await aiocoap.Context.create_client_context()
    
    try:
        response = await send_data(ctx, f"{device['endpoint']}/buzzer/reset_buzzer",payload="")
        async with device['mutex']:
            device['timestamp'] = None           
            device['locked'] = "False" 

    except aiocoap.error.NetworkError:
        return key
    # return None

"""
    Remove all given buzzers from internal data representations.
"""
async def remove_old_buzzers(to_remove):
    global device_status_map_mutex
    async with device_status_map_mutex:
        global device_status_map        
        
        for (k,d) in to_remove:
            device_status_map.pop(k)       
            async with device_heartbeat_map_mutex:
                device_heartbeat_map.pop(k)
            print(f"REMOVED: {k}") 
    ui_backend_sender.send_buzzer_info()


async def reset_buzzers():
    """
        Send a reset message to all registered buzzers.
    """
    async with  device_status_map_mutex:
        for (key,device) in device_status_map.items():
            async with device['mutex']:
                device['timestamp'] = None           
                device['locked'] = "False" 

    await send_data_multicast("buzzer/reset_buzzer","")


"""
    Heartbeat monitor routine. 
    Checks the last heartbeat time stamps of all devices in the interval of HEARTBEAT_INTERVAL_S.
    If the last heartbeat of a device was to long ago, the device is removed.
"""
async def heartbeat_monitor_routine():

    while True:
        to_remove = []
        start_time = time()

        async with device_heartbeat_map_mutex:
            for (dev_id,dev) in device_heartbeat_map.items():
                diff =  abs(start_time - dev['last_heartbeat'])
                if diff > HEARTBEAT_INTERVAL_S:
                    to_remove.append((dev_id,dev))
                    print(f"HB: {dev_id}: {diff}")
        
        if len(to_remove) > 0: asyncio.create_task(remove_old_buzzers(to_remove))

                

        check_time = time() - start_time
        sleep_time = HEARTBEAT_INTERVAL_S-check_time
        
        await asyncio.sleep(sleep_time)



async def main():
    # Resource tree creation
    root = resource.Site()

    root.add_resource(['.well-known', 'core'],
            resource.WKCResource(root.get_resources_as_linkheader,impl_info=None))
    root.add_resource(['b','pressed'], ButtonPressedResource())
    root.add_resource(['b','register'], ButtonRegisterResource())
    root.add_resource(['b','heartbeat'], HeartbeatResource())

    server_ctx = await Context.create_server_context(root,bind=("::",9993))

  
    RD = "coap://[2001:67c:254:b0b2:affe:4000:0:1]"

    rd_rg = rd_register.Registerer(context=server_ctx,rd=RD)

   
    asyncio.create_task(heartbeat_monitor_routine())
    # Run forever
    await asyncio.get_running_loop().create_future()



'''

UI-Backend:
    - PUB: 5556
    - SUB: 5555

Buzzer-Backend:
    - PUB: 5555
    - SUB: 5556
    

zu UI-Backend: immer liste aller Buzzer in form von game-types.ts
    - Buzzer-ID
    - Bei Disconnect -> aktuelle Liste

zu Buzzer-Backend:
    - reset:               "reset, "
    - pairing mode an/aus: "pairing, true/false" 
    - remove buzzer:       "remove, id"
'''


if __name__ in {"__main__", "__mp_main__"}:
    t1= Thread(target=backend_message_receiver)
    t1.start()


    asyncio.run(main())


    #### for test ui #####
    # ui.button('Reset buzzers', on_click=reset_buzzers)
    #
    # app.on_startup(main)
    # ui.run(host="192.168.69.111",port=9080)

