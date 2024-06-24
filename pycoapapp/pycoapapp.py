from aiocoap import *

import argparse


import asyncio
from aiocoap import *

parser = argparse.ArgumentParser(
                    prog='PyCoAPApp',
                    description='',
                    )
# parser.add_argument('-i','--ip',help="Host IP")

IP = "fe80::94cc:5a67:b319:9742%lowpan0"
# IP = "fe80::aca7:23b2:757a:8c36"


async def main():
    args = parser.parse_args()

    protocol = await Context.create_client_context()
    uri = f"coap://[{IP}]/.well-known/core"# ?rt=core.rd*
    print("URI:", uri)

    msg = Message(code=GET, uri=uri)
    response = await protocol.request(msg).response
    print(response)



if __name__ == "__main__":
    asyncio.run(main())

