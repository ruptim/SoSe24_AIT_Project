import asyncio
from datetime import datetime
import zmq
from flask import Flask, request
from flask_socketio import SocketIO, emit
from threading import Thread
import json
from zmq import ZMQError
import config as config

app = Flask(__name__)
# app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, cors_allowed_origins='*')

current_timestamp: datetime = None

@socketio.on('connect')
def connect():
    """
    Connects a client to the socket.
    :return:
    """
    print('Client connected')
    # updateBuzzers() # FIXME debug only


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
    global publisherSocket, current_timestamp

    print('Received RESET')
    current_timestamp = datetime.now()
    print(current_timestamp)

    publisherSocket.send_string("%s" % (config.channels['reset']))


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


def send_buzzers_to_ui(buzzer_list):
    """
    Maps the buzzer list and sends it to the UI
    :param buzzer_list: list of buzzers of type [{"buzzerId": number, "buzzerName": str, "islocked": bool, "timestamp": timestamp}]
    :return:
    """
    try:
        transformed_data = mapBuzzers(buzzer_list)

        print(f"Sending broadcast to channel {config.events['buzzers']}")
        # socketio.emit(config.events['buzzers'], json.dumps(transformed_data))

        print (json.dumps(transformed_data))
        socketio.emit(config.events['buzzers'], 'TEST')
        print('Sent')
    except Exception as err:
        print(err)


def mapBuzzers(buzzer_list):
    """
    Maps the buzzer list to fit the requirements of the UI and calculates the delay to current_timestamp
    :param buzzer_list: The deserialized list of buzzers
    :return:
    """
    global current_timestamp
    result = []

    for obj in buzzer_list:
        transformed_obj = {
            "buzzerId": obj["buzzerId"],
            "buzzerName": obj["buzzerName"],
            "isPressed": obj["timestamp"] is not None,  # Set to True if timestamp is provided
            "isLocked": obj["islocked"],
            "delay": None
        }

        if obj["timestamp"] is not None and current_timestamp is not None:
            timestamp = datetime.strptime(obj["timestamp"], "%Y-%m-%dT%H:%M:%S.%f")
            delay = current_timestamp - timestamp
            transformed_obj["delay"] = delay.total_seconds()  # Calculating delay
        else:
            transformed_obj["delay"] = None

        result.append(transformed_obj)

    return result


# TODO only for testing, please remove
# START TEST

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

    socketio.emit(config.events['buzzers'], json.dumps(buzzers), callback=lambda: print('received'))

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

    print(f"SUBSCRIBE connected to channel \"{config.channels['buzzers']}\"")

    while True:
        try:
            msg_json = subscriberSocket.recv_json()
            print(msg_json)

            send_buzzers_to_ui(msg_json)

        except Exception as err:
            print(err)


if __name__ == '__main__':

    subscriberThread = Thread(target=initSubscriber)
    publisherThread = Thread(target=initPublisher)

    subscriberThread.start()
    publisherThread.start()

    socketio.run(app, config.hostname, config.port, allow_unsafe_werkzeug=True)
