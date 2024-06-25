from aiocoap import *
import argparse
from time import sleep


import asyncio
from aiocoap import *
import logging
from aiocoap.options import Options
from aiocoap.optiontypes import *
import re

from rich.console import Console

console = Console()
from rich.table import Table

# drop in replacement for default print
print = console.print

logging.basicConfig(level=logging.INFO)



parser = argparse.ArgumentParser(
                    prog='PyCoAPApp',
                    description='',
                    )
# parser.add_argument('-i','--ip',help="Host IP")

# IP = "fe80::3867:9b25:ae2:ea8b%lowpan0" #rd
IP = "fe80::aca7:23b2:757a:8c36%lowpan0" # node
IP = "localhost" # server.py
RD = "fe80::3867:9b25:ae2:ea8b%lowpan0"


ENDPOINT_REGEX = '<\/([^,]+)'
ENDPOINT_NAME_REGEX = 'ep=\"([^"]+)'
ENDPOINT_BASE_REGEX = 'base=\"([^"]+)'
RESOURCE_PATH_REGEX = '<([^>]+)'

endpoint_map = {}




async def get_data(ctx,uri):

    request = Message(code=GET, uri=uri)
    
    requester = ctx.request(request)

    resp = await requester.response
    return resp


async def read_all_data_task(ctx,ep_name,ep_base):
    table = Table(title=f"Data of {ep_name}")
    table.add_column("Resource",justify='left',style="red",no_wrap=True)
    table.add_column("Data",style="green")

    for res in endpoint_map[ep_name]["resources"]:
        response = await get_data(ctx,f'{ep_base}{res}')
        data = response.payload.decode('utf-8')
        # print(f"[bold red]{res}: [green]{data}")
        table.add_row(res,data)
    print(table)
        

async def get_resources_for_endpoint(ctx,ep_base):
    response = await get_data(ctx,f'{ep_base}/.well-known/core')
    payload = response.payload.decode('utf-8')
    resources = re.findall(RESOURCE_PATH_REGEX,payload)
    return resources
            


async def parse_ep_data(ctx,resp):
    ep_names = re.findall(ENDPOINT_NAME_REGEX,resp)
    ep_bases = re.findall(ENDPOINT_BASE_REGEX,resp)
    for i,(n,b) in enumerate(zip(ep_names,ep_bases)):
        # print("[Init] Found Endpoints: ",i+1,n,b) 
        if n not in endpoint_map:
            resources = await get_resources_for_endpoint(ctx, b)
            endpoint_map[n] = {'index':i+1,"base":b,"resources":resources}
            await read_all_data_task(ctx,n,b)
            


async def get_update_data(ctx):
    uri = f"coap://[{RD}]/endpoint-lookup/"# ?rt=core.rd*

    request = Message(code=GET, uri=uri)
    
    requester = ctx.request(request)

    resp = await requester.response

    return resp.payload.decode("utf-8")

async def observe_endpoints(ctx):
    uri = f"coap://[{RD}]/endpoint-lookup/"# ?rt=core.rd*

    request = Message(code=GET, uri=uri,
                  observe=0
                  )
    
    requester = ctx.request(request)

    async def observe_eps():
        response = await requester.response
        await parse_ep_data(ctx,response.payload.decode("utf-8"))

        
        

        try:            
            async for obs in requester.observation:
                    
                    # -- GET changed endpoint-lookup data
                    resp = await asyncio.create_task(get_update_data(ctx))
                    # print("[OBS] Update:",resp)
                    await parse_ep_data(ctx,resp)

        except Exception as e:
            pass

    obss = asyncio.create_task(observe_eps())

   


async def main():
    args = parser.parse_args()

    ctx = await Context.create_client_context()
    # uri = f"coap://[{IP}]/.well-known/core"# ?rt=core.rd*
    uri = f"coap://[{IP}]/b/Button(SW0)_3"
    # uri = f"coap://[{IP}]/cli/stats"
    # uri = f"coap://{IP}/time"
    # uri = f"coap://[{RD}]/endpoint-lookup/"# ?rt=core.rd*
    # print("URI:", uri)

    running_task = asyncio.create_task(observe_endpoints(ctx))
    # request = Message(code=GET, uri=uri,
    #             #   observe=0
    #               )
    
    # requester = ctx.request(request)

    # async def observe_task():
       


    # response = await requester.response
    # print("First response: %s\n%r"%(response, response.payload))
    
    # print("Loop ended, sticking around")
    await asyncio.sleep(50)


    while not asyncio.get_event_loop().is_closed():
        await asyncio.sleep(1)
    



if __name__ == "__main__":

  

    asyncio.run(main())    
    # loop = asyncio.get_event_loop()
    # loop.run_until_complete(main())
    # loop.close()

