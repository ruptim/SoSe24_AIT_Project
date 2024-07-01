from aiocoap import *
import argparse
from time import sleep


import asyncio

import logging
logging.basicConfig(level=logging.INFO)

from aiocoap import *
from aiocoap.options import Options
from aiocoap.optiontypes import *
import re


from rich.console import Console

console = Console()
from rich.table import Table
from rich.text import Text
from rich.tree import Tree
# drop in replacement for default print
print = console.print


from ahrs.filters import FAMC
from ahrs.common.dcm import DCM
import numpy as np


RD = "fe80::3867:9b25:ae2:ea8b%lowpan0"
BDR = "2001:67c:254:b0b2:affe:4000:0:1%lowpan0"
RD = BDR


ENDPOINT_REGEX = r'<\/([^,]+)'
ENDPOINT_NAME_REGEX = r'ep=\"([^"]+)'
ENDPOINT_BASE_REGEX = r'base=\"([^"]+)'
RESOURCE_PATH_REGEX = r'<([^>]+)'
RESOURCE_DATA_REGEX = r'[:\s]([+-]?\d+(?:\.\d+)?(?:e[+-]?\d+)?)\b\s*([^\d\s,]+)'
RESOURCE_SENSOR_NAME_REGEX = r'^([^:]+)'


'''
    endpoint_name:
        'index': int,
        'base: str,
        'resources': {
            uri: str
            'last_readings': Data
        },
        'default_orientation_up': bool
        
'''
endpoint_map = {}
device_orientation = {}



SUPPORTED_MAG_SENSORS = ["lsm6dsxx","mag3110"]
SUPPORTED_ACC_SENSORS = ["lis3mdl","mma8x5x"]



class Data:
    def __init__(self,data_str):
        self.data_str = data_str
        self.values = []
        self.sensor_name = re.findall(RESOURCE_SENSOR_NAME_REGEX,data_str)[0]
        self.dim = 0

        self._convert_to_numbers()

        

    def _convert_to_numbers(self):
        vals_and_units = re.findall(RESOURCE_DATA_REGEX,self.data_str)
        for (v,u) in vals_and_units:
            val = {"value":float(v),'unit':u}
            self.values.append(val)
        
        self.dim = len(self.values)

    def get_values(self):
            
        return tuple([v['value'] for v in self.values])




async def send_led_status(ctx,active):
    status = "0".encode() if active else "1".encode()
    for ep in endpoint_map.values():
        led_res = None
        for r in ep['resources'].values():
            if r['last_reading'].sensor_name.find("LED") != -1: 
                led_res = r['uri']
                break
        if led_res: await send_data(ctx, f"{ep['base']}{led_res}",payload=status)


async def check_orientation_of_all_devs(ctx):
    device_upside_down = False
    for dev in device_orientation.values():
        device_upside_down |= dev['upside_down']
    
    await send_led_status(ctx, device_upside_down)


def is_upside_down(pitch, yaw, roll, default_up):
    upside_down = (-70.0 < roll and roll < 70.0)
    return bool(upside_down if default_up else not upside_down)


def check_orientation(mag, acc,default_up):
    famc = FAMC(mag=np.array([mag.get_values()]), acc=np.array([acc.get_values()]))
    dcm = DCM(DCM.from_q(DCM(),famc.Q[0]))
    vec = dcm.to_rpy()
    vec = np.rad2deg(vec)     
    return is_upside_down(vec[1],vec[2],vec[0],default_up), vec
            
        


def check_for_mag_and_acc_sensor(readings: dict[Data]):
    mag = None
    acc = None

    for r in readings.values():
        if not mag and r.sensor_name in SUPPORTED_MAG_SENSORS: mag = r
        if not acc and r.sensor_name in SUPPORTED_ACC_SENSORS: acc = r
    
    return mag, acc


'''
    Iterate over resources of end point and request current data.
    Create 'Data' object with requested data and save in endpoint_map.
    If resources include data from magnetomers and accelerometer, compute orientation and notify other devices.
'''
async def read_all_data_task(ctx,ep_name,ep_base):

    ## --- output ---
    tree = Tree("[bold blue]Endpoint")
    table = Table()
    table.add_column("Resource",justify='left',style="yellow",no_wrap=True)
    table.add_column("Data",style="green")
    ## --- output ---

    
    
    readings = {}
    for res in endpoint_map[ep_name]["resources"]:

        response = await get_data(ctx,f'{ep_base}{res}')
        data = response.payload.decode('utf-8')
        reading = Data(data)
        readings[res] = reading
        endpoint_map[ep_name]['resources'][res]['last_reading'] = reading
    
        table.add_row(res,data)

    mag, acc = check_for_mag_and_acc_sensor(readings)
    upside_down = None
    if mag and acc:
        # pitch, roll, yaw = calculate_orientation(mag, acc)
        upside_down, rpy_vec = check_orientation(mag, acc, endpoint_map[ep_name]["default_orientation_up"])
        device_orientation[ep_name] = {"rpy":rpy_vec,"upside_down":upside_down}

    ## --- output ---
    tree.add(f"Name: [i]{ep_name}")
    
    base = Text(f"Base: {ep_base}")
    base.highlight_regex(r"[:\s](.*)","i")
    tree.add(base)
    tree.add(table)
    tree.add(f"Upside-Down: {'[green b]Yes' if upside_down else '[red b]No' if type(upside_down) == bool else '[yellow b]None'}")
    
    print(tree)
    ## --- output ---


    await check_orientation_of_all_devs(ctx)
    
        

async def get_resources_for_endpoint(ctx,ep_base):
    response = await get_data(ctx,f'{ep_base}/.well-known/core')
    payload = response.payload.decode('utf-8')
    resources = re.findall(RESOURCE_PATH_REGEX,payload)

    resource_dict = {}
    for r in resources:
        resource_dict[r] = {"uri":r}

    return resource_dict
            


async def parse_ep_lookup_data(ctx,resp,endpoint_map_lock):
    ep_names = re.findall(ENDPOINT_NAME_REGEX,resp)
    ep_bases = re.findall(ENDPOINT_BASE_REGEX,resp)
    

    for i,(n,b) in enumerate(zip(ep_names,ep_bases)):
        async with endpoint_map_lock:
            if n not in endpoint_map:
                resources = await get_resources_for_endpoint(ctx, b)
                endpoint_map[n] = {
                    'index':i+1,
                    "base":b,
                    "resources":resources,
                    "default_orientation_up": True if n.find("FEATHER") != -1 else False
                }
                await read_all_data_task(ctx,n,b)


async def send_data(ctx,uri,payload):

    request = Message(code=PUT, uri=uri,payload=payload)
    
    requester = ctx.request(request)

    resp = await requester.response
    return resp


async def get_data(ctx,uri):

    request = Message(code=GET, uri=uri)
    
    requester = ctx.request(request)

    resp = await requester.response
    return resp


async def get_rd_update_data(ctx):
    uri = f"coap://[{RD}]/endpoint-lookup/"# ?rt=core.rd*

    request = Message(code=GET, uri=uri)
    
    requester = ctx.request(request)

    resp = await requester.response

    return resp.payload.decode("utf-8")


async def observe_endpoints(ctx,endpoint_map_lock):
    uri = f"coap://[{RD}]/endpoint-lookup/"# ?rt=core.rd*

    request = Message(code=GET, uri=uri,
                  observe=0
                  )
    
    requester = ctx.request(request)

    async def observe_eps():
        response = await requester.response
        await parse_ep_lookup_data(ctx,response.payload.decode("utf-8"),endpoint_map_lock)

        try:            
            async for obs in requester.observation:
                    
                    # -- GET changed endpoint-lookup data
                    resp = await asyncio.create_task(get_rd_update_data(ctx))
                    # print("[OBS] Update:",resp)
                    await parse_ep_lookup_data(ctx,resp,endpoint_map_lock)

        except Exception as e:
            pass

    obss = asyncio.create_task(observe_eps())

async def main():

    ctx = await Context.create_client_context()

    endpoint_map_lock = asyncio.locks.Lock()

    running_task = asyncio.create_task(observe_endpoints(ctx,endpoint_map_lock))


    while not asyncio.get_event_loop().is_closed():
        await asyncio.sleep(1)
    



if __name__ == "__main__":

  

    asyncio.run(main())    
