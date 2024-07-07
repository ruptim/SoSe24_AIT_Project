from flask import Flask, request
from flask_socketio import SocketIO, emit
from threading import Timer

import config as config

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)


@socketio.on('connect')
def connect():
    print('Client connected')
    emit(config.events['management'], {'data': 'Connected'})


@socketio.on('disconnect')
def disconnect():
    print('Client disconnected')


@socketio.event
def echo(message):
    print('Client received message: {}'.format(message))
    emit(config.events['test'], {'data': message})
    t = Timer(10.0, broadcast)
    t.start()


@socketio.on_error_default  # handles all namespaces without an explicit error handler
def default_error_handler(e):
    print(request.event["message"]) # "my error event"
    print(request.event["args"])    # (data,)
    print('An error occurred:', e)
    pass


def broadcast():
    print('Sending broadcast.')
    socketio.emit(config.events['game'], {'data': 'Broadcast'})


if __name__ == '__main__':
    socketio.run(app, config.hostname, config.port, allow_unsafe_werkzeug=True, debug=True, log_output=True)
