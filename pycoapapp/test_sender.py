import zmq
import random
import sys
from time import sleep


def msq2():
    context = zmq.Context()
    socket = context.socket(zmq.PUB)
    socket.bind("tcp://*:5556")
    while True:        
        socket.send_string("AAAA")
        sleep(1)
def msq():
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect("tcp://localhost:5555")
    socket.setsockopt(zmq.SUBSCRIBE, b'')
   
    while True:        
        msg = socket.recv()
        print(msg)
        sleep(1)


import sys
from threading import Thread




a= Thread(target=msq2)
b= Thread(target=msq)


# b.join()

# a.start()
# # b.start()

# a.join()

# if __name__ == "__main__":

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:5556")

msg = ""

if sys.argv[1] == "0":
    print("Send: reset")
    msg = "reset,2"
elif sys.argv[1] == "1":
    print("Send: pairing true")
    msg = "pairing, True"
elif sys.argv[1] == "2":
    print("Send: pairing false")
    msg = "pairing, False"
elif sys.argv[1] == "3":
    print("Send: remove")
    msg = "remove, buzzer0"

sleep(1)

socket.send_string(msg)

import queue
import datetime

a = queue.Queue()
a.put(datetime.datetime.isoformat(datetime.datetime.now()))


print(a.get())
