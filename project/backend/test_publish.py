import zmq
import random
import sys
from time import sleep


def msq2():
    context = zmq.Context()
    socket = context.socket(zmq.PUB)
    socket.bind("tcp://*:5555")
    print("Publishing")
    socket.send_string('[{"buzzerId": 0, "buzzerName": "buzzer0", "islocked": "True", "timestamp": null}, {"buzzerId": 1, "buzzerName": "buzzer1", "islocked": "True", "timestamp": "2024-07-15T14:09:28.423909"}]')
    sleep(1)
    print("Publishing")
    socket.send_string('[{"buzzerId": 0, "buzzerName": "buzzer0", "islocked": "True", "timestamp": null}, {"buzzerId": 1, "buzzerName": "buzzer1", "islocked": "True", "timestamp": "2024-07-15T14:09:28.423909"}]')



def msq():
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect("tcp://localhost:5556")
    socket.setsockopt(zmq.SUBSCRIBE, b'')
   
    while True:        
        msg = socket.recv()
        print(msg)
        sleep(1)


from threading import Thread

b= Thread(target=msq2)

b.start()

b.join()

