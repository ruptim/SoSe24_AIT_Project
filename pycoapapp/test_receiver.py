import zmq
import random
import sys
from time import sleep


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




b= Thread(target=msq)

b.start()

b.join()

