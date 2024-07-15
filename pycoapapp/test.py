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


from threading import Thread

a= Thread(target=msq)
b= Thread(target=msq2)

a.start()
b.start()

a.join()
b.join()

