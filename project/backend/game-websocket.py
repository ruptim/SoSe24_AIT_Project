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
smallest_timestamp: datetime = None

@socketio.on('connect')
def connect():
    """
    Connects a client to the socket.
    :return:
    """
    print('Client connected')


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
    global publisherSocket, current_timestamp, smallest_timestamp

    print('Received RESET')
    current_timestamp = datetime.now()
    smallest_timestamp = None
    print(current_timestamp)
    print('reset smallest timestamp')

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

        print(f"Sending broadcast to channel {config.events['buzzers']} with data:")
        socketio.emit(config.events['buzzers'], json.dumps(transformed_data))
        print (json.dumps(transformed_data))
    except Exception as err:
        print(err)


def mapBuzzers(buzzer_list):
    """
    Maps the buzzer list to fit the requirements of the UI and calculates the delay to current_timestamp
    :param buzzer_list: The deserialized list of buzzers
    :return:
    """
    global current_timestamp, smallest_timestamp
    result = []

    smallest_timestamp = calc_smallest_timestamp(buzzer_list)

    for obj in buzzer_list:
        transformed_obj = {
            "buzzerId": obj["buzzerId"],
            "buzzerName": obj["buzzerName"],
            "isPressed": obj["timestamp"] is not None,  # Set to True if timestamp is provided
            "isLocked": False,
            "delay": None,
            "delay_local": None
        }

        if obj["timestamp"] is not None and current_timestamp is not None:
            timestamp = datetime.strptime(obj["timestamp"], "%Y-%m-%dT%H:%M:%S.%f")
            delay = timestamp - current_timestamp
            delay_local = timestamp - smallest_timestamp
            transformed_obj["delay"] = delay.total_seconds()  # Calculating delay
            transformed_obj["delay_local"] = delay_local.total_seconds()
        else:
            transformed_obj["delay"] = None
            transformed_obj["delay_local"] = None

        result.append(transformed_obj)

    return result

def calc_smallest_timestamp(objects):

    # Filter objects that have 'timestamp' attribute
    objects_with_timestamp = [obj for obj in objects if obj.get('timestamp')]

    # Initialize the smallest timestamp as the timestamp of the first object
    smallest = datetime.strptime(objects_with_timestamp[0]["timestamp"], "%Y-%m-%dT%H:%M:%S.%f")

    # Iterate over the objects in the list
    for obj in objects_with_timestamp:

        if obj["timestamp"]:
            timestamp = datetime.strptime(obj["timestamp"], "%Y-%m-%dT%H:%M:%S.%f")
            # If the current object's timestamp is smaller than the smallest found so far
            if timestamp < smallest:
                # Update the smallest timestamp
                smallest = datetime.strptime(obj["timestamp"], "%Y-%m-%dT%H:%M:%S.%f")

    # Return the smallest timestamp
    return smallest


def initPublisher():
    global publisherSocket

    context = zmq.Context()
    publisherSocket = context.socket(zmq.PUB)
    publisherSocket.bind(f"tcp://*:{config.publish_port}")


    print(f"PUBLISH connected to channel\n")

def initSubscriber():
    global subscriberSocket

    context = zmq.Context()
    subscriberSocket = context.socket(zmq.SUB)
    subscriberSocket.connect(f"tcp://{config.backend_hostname}:{config.subscribe_port}")
    subscriberSocket.setsockopt_string(zmq.SUBSCRIBE, config.channels["buzzers"])

    print(f"SUBSCRIBE connected to channel \"{config.channels['buzzers']}\"\n")

    while True:
        try:
            msg_json = subscriberSocket.recv_json()
            print("SUBSCRIBE message received")

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
