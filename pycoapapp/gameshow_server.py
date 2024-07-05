#!/usr/bin/env python3

# SPDX-FileCopyrightText: Christian Amsüss and the aiocoap contributors
#
# SPDX-License-Identifier: MIT

"""This is a usage example of aiocoap that demonstrates how to implement a
simple server. See the "Usage Examples" section in the aiocoap documentation
for some more information."""

from datetime import datetime
import logging

import asyncio

import aiocoap.resource as resource
from aiocoap.numbers.contentformat import ContentFormat
from aiocoap import *
import aiocoap

import aiocoap.resourcedirectory.client.register  as rd_register

from queue import Queue

import re


'''
        'dev_name':
        {
            'time_stamp': ts    
        }
'''
device_status_map = {}


class Welcome(resource.Resource):
    representations = {
            ContentFormat.TEXT: b"Welcome to the demo server",
            ContentFormat.LINKFORMAT: b"</.well-known/core>,ct=40",
            # ad-hoc for application/xhtml+xml;charset=utf-8
            ContentFormat(65000):
                b'<html xmlns="http://www.w3.org/1999/xhtml">'
                b'<head><title>aiocoap demo</title></head>'
                b'<body><h1>Welcome to the aiocoap demo server!</h1>'
                b'<ul><li><a href="time">Current time</a></li>'
                b'<li><a href="whoami">Report my network address</a></li>'
                b'</ul></body></html>',
            }

    default_representation = ContentFormat.TEXT

    async def render_get(self, request):
        cf = self.default_representation if request.opt.accept is None else request.opt.accept
        try:
            return aiocoap.Message(payload=self.representations[cf], content_format=cf)
        except KeyError:
            raise aiocoap.error.UnsupportedContentFormat



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
        
        device_status_map[register_name] = {
            'endpoint': ep,
            'register_time': datetime.now(),
            'ts_queue': asyncio.Queue(),
            'mutex': asyncio.locks.Lock()

        }
        
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

if __name__ == "__main__":
    asyncio.run(main())
