import zmq
from flask import Flask, request
from flask_socketio import SocketIO, emit
from threading import Timer, Thread
import json

from zmq import ZMQError

import config as config

app = Flask(__name__)
# app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, cors_allowed_origins='*')

buzzers = []

# TODO: set last_reset_timestamp here, when reset is being pressed (when reset pressed: send reset event as well)
# no isPressed as input -> isLocked -> isPressed
# map isLocked -> isPressed
# calculate delay from last_reset_timestamp - timestamp

# for answers to Timon Backend:
# reset: "reset, "
# pairing mode an/aus: "pairing, true/false"
# remove buzzer: "remove, name"

#

@socketio.on('connect')
def connect():
    """
    Connects a client to the socket.
    :return:
    """
    print('Client connected')
    updateBuzzers() # FIXME debug only


@socketio.on('disconnect')
def disconnect():
    """
    Disconnects a client from the socket.
    :return:
    """
    print('Client disconnected')


@socketio.on(config.events['reset'])
def reset():
    """
    Resets all buzzers to not be pressed anymore
    """
    global publisherSocket

    print('Received RESET')
    publisherSocket.send_string("%s" % (config.events['reset']))


@socketio.on(config.events['remove'])
def remove(buzzer_str):
    """
    Removes buzzer from all buzzers
    """
    global publisherSocket

    buzzer = json.loads(buzzer_str)

    print(f'Received REMOVE for buzzer {buzzer}')
    publisherSocket.send_string("%s%s" % (config.channels['remove'], buzzer['buzzerName']))


@socketio.on(config.events['pairing'])
def pairing(pairing_active: bool):
    """
    Activates pairing mode
    """
    global publisherSocket

    print(f'Received PAIRING {pairing_active}')
    publisherSocket.send_string("%s%s" % (config.channels['pairing'], pairing_active))


@socketio.on_error_default  # handles all namespaces without an explicit error handler
def default_error_handler(e):
    print(request.event["message"]) # "my error event"
    print(request.event["args"])    # (data,)
    print('An error occurred:', e)
    pass


def broadcast(buzzerList):
    print(f"Sending broadcast to channel {config.events['buzzers']}")
    socketio.emit(config.events['buzzers'], json.dumps(buzzerList))


def updateBuzzers():
    """
    Sends a buzzer array to the channel 'buzzers'.
    :return:
    """
    # TODO send array of buzzers

    buzzerList = [
        {
            "buzzerId": 0,
            "buzzerName": "First Buzzer",
            "isPressed": True,
            "isLocked": False,
            "delay": 2.56,
        },
        {
            "buzzerId": 1,
            "buzzerName": "Second Buzzer that has a long name",
            "isPressed": False,
            "isLocked": False,
            "delay": None,
        },
    ];

    emit(config.events['buzzers'], json.dumps(buzzerList))


# TODO only for testing, please remove
# START TEST

buzzerList = [
    {
        "buzzerId": 0,
        "buzzerName": "First Buzzer",
        "isPressed": True,
        "isLocked": False,
        "delay": 2.56,
    },
    {
        "buzzerId": 1,
        "buzzerName": "Second Buzzer that has a long name",
        "isPressed": False,
        "isLocked": False,
        "delay": None,
    },
];


@socketio.on('test_reset')
def testReset(param):
    print('Reset test')
    for buzzer in buzzerList:
        buzzer['isPressed'] = False
        buzzer['delay'] = None
    emitBuzzers(buzzerList)


@socketio.on('test_1_pressed')
def testBuzzerOnePressed(param):
    print('Buzzer one pressed')
    buzzerList[0]['isPressed'] = True
    buzzerList[0]['delay'] = 1.1
    emitBuzzers(buzzerList)


@socketio.on('test_2_pressed')
def testBuzzerOnePressed(param):
    print('Buzzer two pressed')
    buzzerList[1]['isPressed'] = True
    buzzerList[1]['delay'] = 2.2
    emitBuzzers(buzzerList)


@socketio.on('test_lock')
def testLockAll(param):
    print('Reset lock all buzzers')
    for buzzer in buzzerList:
        buzzer['isLocked'] = True
    emitBuzzers(buzzerList)


@socketio.on('test_unlock')
def testUnlockAll(param):
    print('Reset lock all buzzers')
    for buzzer in buzzerList:
        buzzer['isLocked'] = False
    emitBuzzers(buzzerList)


@socketio.on('test_add')
def testBuzzerAdd(param):
    print('Add third buzzer')
    buzzerList.append({
        "buzzerId": 3,
        "buzzerName": "New Buzzer",
        "isPressed": False,
        "isLocked": False,
        "delay": None,
    })
    emitBuzzers(buzzerList)


@socketio.on('test_del')
def testBuzzerDel(param):
    print('Delete last buzzer')
    buzzerList.pop()
    emitBuzzers(buzzerList)


def emitBuzzers(buzzers):
    print('Emit buzzers')
    print(buzzers)
    socketio.emit(config.events['buzzers'], json.dumps(buzzers))

# END TEST

def initPublisher():
    global publisherSocket

    context = zmq.Context()
    publisherSocket = context.socket(zmq.PUB)
    publisherSocket.bind(f"tcp://*:{config.publish_port}")

    print(f"PUBLISH connected to channel")

def initSubscriber():
    global subscriberSocket

    context = zmq.Context()
    subscriberSocket = context.socket(zmq.SUB)
    subscriberSocket.connect(f"tcp://{config.hostname}:{config.subscribe_port}")
    subscriberSocket.setsockopt_string(zmq.SUBSCRIBE, config.channels["buzzers"])

    print(f"SUBSCRIBE connected to channel {config.channels['buzzers']}")

    while True:
        try:
            msg_json = subscriberSocket.recv_json()
            print(msg_json)
        except ZMQError as err:
            print(err)


if __name__ == '__main__':

    subscriberThread = Thread(target=initSubscriber)
    publisherThread = Thread(target=initPublisher)

    publisherThread.start()
    subscriberThread.start()


    socketio.run(app, config.hostname, config.port, allow_unsafe_werkzeug=True, debug=False)
