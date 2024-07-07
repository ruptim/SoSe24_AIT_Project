#!/usr/bin/env python3

# SPDX-FileCopyrightText: Christian Amsüss and the aiocoap contributors
#
# SPDX-License-Identifier: MIT

"""This is a usage example of aiocoap that demonstrates how to implement a
simple server. See the "Usage Examples" section in the aiocoap documentation
for some more information."""

from datetime import datetime
import logging

from threading import Thread

import asyncio

import aiocoap.resource as resource
from aiocoap.numbers.contentformat import ContentFormat
from aiocoap import *
import aiocoap

import aiocoap.resourcedirectory.client.register  as rd_register

from queue import Queue

import re

from nicegui import ui, app



'''
        'dev_name':
        {
            'time_stamp': ts    
        }
'''
device_status_map = {}
device_list = ui.row()

class status_label(ui.label):
    def _handle_text_change(self, text: str) -> None:
        # super()._handle_text_change(text)
        if text == 'False':
            self.style("color:green")
        else:
            self.style("color:red")
    



class ButtonResource(resource.ObservableResource):
    """Example resource that can be observed. The `notify` method keeps
    scheduling itself, and calles `update_state` to trigger sending
    notifications."""

    def __init__(self):
        super().__init__()

        self.handle = None
        self.status = 0

    def notify(self):
        self.updated_state()
        self.reschedule()

    def reschedule(self):
        self.handle = asyncio.get_event_loop().call_later(5, self.notify)

    def update_observation_count(self, count):
        if count and self.handle is None:
            print("Starting the clock")
            self.reschedule()
        if count == 0 and self.handle:
            print("Stopping the clock")
            self.handle.cancel()
            self.handle = None

    async def render_get(self, request):
        payload = f"Button(SW0): {self.status} none".encode('ascii')
        return aiocoap.Message(payload=payload)


class ButtonRegisterResource(resource.Resource):

    def __init__(self):
        super().__init__()

    def send_registered_name(self,ep):
        device_count = len(device_status_map)
        register_name = f"buzzer{device_count}"
        
        l = None        
        with device_list:
            l = status_label(register_name)
            
            l.style('color:red')

        device_status_map[register_name] = {
            'endpoint': ep,
            'register_time': datetime.now(),
            'ts_queue': asyncio.Queue(),
            'mutex': asyncio.locks.Lock(),
            'label': l,
            "locked": "True"

        }
        l.bind_text_from(device_status_map[register_name],'locked')
        
        
        return aiocoap.Message(code=aiocoap.CHANGED,payload=register_name.encode('ascii'))

    def register_device(self, request):
        payload = request.payload.decode('ascii')
        if len(payload): 
            entry = device_status_map.get(payload)

            # if the re-registration comes from the correct endpoint: accept
            if entry and entry['endpoint'] == request.remote.uri_base:
                return aiocoap.Message(code=aiocoap.CHANGED,payload="0".encode("ascii"))    

            # if requested name is not found, send new one                 

        return self.send_registered_name(request.remote.uri_base)

            

    
    async def render_put(self, request):
        print('PUT payload: %s' % request.payload)
        return self.register_device(request)
     

class ButtonPressedResource(resource.Resource):
    """ 
        Resource for endpoints to write to if theire connected buzzer-button was pressed.
    """ 

    def __init__(self):
        super().__init__()

    async def set_content(self, content):
        '''
            expected content format:
                device_name,time_stamp (ISO 8601) 
            Example:
                buzzer1,2024-07-01T15:45:44.585110
        '''
        # todo regex for timestamp and device name
        matches = re.findall(r'([^,]+)',content)

        device_id = matches[0]
        time_stamp = datetime.fromisoformat(matches[1])
        
        if (device_status_map.get(device_id)):
            async with device_status_map[device_id]['mutex']:
                await device_status_map[device_id]['ts_queue'].put(time_stamp)
                device_status_map[device_id]['locked'] = "True"
        #TODO: else: send error response!! 
        

    async def render_put(self, request):
        print('PUT payload: %s' % request.payload)
        await self.set_content(request.payload.decode('ascii'))

        return aiocoap.Message(code=aiocoap.CHANGED)



async def send_data(ctx,uri,payload):
    request = aiocoap.Message(code=aiocoap.PUT, uri=uri,payload=payload)
    requester = ctx.request(request)
    resp = await requester.response
    return resp


async def reset_buzzers():
    ctx = await aiocoap.Context.create_client_context()

    to_remove = []


    #todo: make each send a seperate task, so the resets are 'simultaneous'
    for (key,device) in device_status_map.items():
        try:
            response = await send_data(ctx, f"{device['endpoint']}/buzzer/reset_buzzer",payload="")

            async with device['mutex']:
                device['ts_queue'] = asyncio.Queue()
                # device['label'].style('color:green')
                device['locked'] = "False"
        # if response.code == CHANGED:
        #     pass # success
        except aiocoap.error.NetworkError:
            to_remove.append(key)

    for k in to_remove:
        device_status_map.pop(k)       
        print(f"REMOVED: {k}") 


# logging setup

logging.basicConfig(level=logging.ERROR)
logging.getLogger("coap-server").setLevel(logging.DEBUG)

async def main():
    # Resource tree creation
    root = resource.Site()

    root.add_resource(['.well-known', 'core'],
            resource.WKCResource(root.get_resources_as_linkheader,impl_info=None))
    # root.add_resource([], Welcome())
    root.add_resource(['b','0'], ButtonResource())
    root.add_resource(['b','pressed'], ButtonPressedResource())
    root.add_resource(['b','register'], ButtonRegisterResource())

    server_ctx = await Context.create_server_context(root,bind=("::",9993))
  
    RD = "coap://[2001:67c:254:b0b2:affe:4000:0:1]"

    rd_rg = rd_register.Registerer(context=server_ctx,rd=RD)

   

    # Run forever
    await asyncio.get_running_loop().create_future()



if __name__ in {"__main__", "__mp_main__"}:
    # asyncio.run(main()) ## to run without ui 


    ui.button('Reset buzzers', on_click=reset_buzzers)

    app.on_startup(main)

    ui.run(host="192.168.69.111",port=9080)

