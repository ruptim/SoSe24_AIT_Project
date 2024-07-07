from flask import Flask, request
from flask_socketio import SocketIO, emit
from threading import Timer
import json

import config as config

app = Flask(__name__)
# app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, cors_allowed_origins='*')

buzzers = []

@socketio.on('connect')
def connect():
    """
    Connects a client to the socket.
    :return:
    """
    print('Client connected')
    # emit(config.events['connect'], 'connected')
    # t = Timer(10.0, updateBuzzers)
    # t.start()
    updateBuzzers()


@socketio.on('disconnect')
def disconnect():
    """
    Disconnects a client from the socket.
    :return:
    """
    print('Client disconnected')
    # emit(config.events['disconnect'], 'disconnected')


@socketio.event
def echo(message):
    print('Client received message: {}'.format(message))
    emit(config.events['test'], {'data': message})
    t = Timer(10.0, broadcast)
    t.start()


@socketio.on(config.events['reset'])
def reset():
    """
    Resets all buzzers to not be pressed anymore
    """

    # TODO implement
    print('Received RESET')


@socketio.on(config.events['lock'])
def lock():
    """
    Locks all buzzers
    """

    # TODO implement
    print('Received LOCK')


@socketio.on(config.events['remove'])
def remove(buzzer):
    """
    Removes buzzer from all buzzers
    """

    # TODO implement
    print(f'Received REMOVE for buzzer {buzzer}')


@socketio.on(config.events['pairing'])
def pairing():
    """
    Activates pairing mode
    """

    # TODO implement
    print(f'Received PAIRING')


@socketio.on_error_default  # handles all namespaces without an explicit error handler
def default_error_handler(e):
    print(request.event["message"]) # "my error event"
    print(request.event["args"])    # (data,)
    print('An error occurred:', e)
    pass


def broadcast():
    print('Sending broadcast.')
    socketio.emit(config.events['game'], {'data': 'Broadcast'})


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


if __name__ == '__main__':
    socketio.run(app, config.hostname, config.port, allow_unsafe_werkzeug=True, debug=True)
