import json
import unittest

from flask_socketio import SocketIO, emit

import config

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
def testReset():
    print('Reset test')
    for buzzer in buzzerList:
        buzzer['isPressed'] = False
        buzzer['delay'] = None
    emitBuzzers(buzzerList)


@socketio.on('test_1_pressed')
def testBuzzerOnePressed():
    print('Buzzer one pressed')
    buzzerList[0]['isPressed'] = True
    buzzerList[0]['delay'] = 1.1
    emitBuzzers(buzzerList)


@socketio.on('test_2_pressed')
def testBuzzerOnePressed():
    print('Buzzer two pressed')
    buzzerList[1]['isPressed'] = True
    buzzerList[1]['delay'] = 2.2
    emitBuzzers(buzzerList)


@socketio.on('test_lock_all')
def testLockAll():
    print('Reset lock all buzzers')
    for buzzer in buzzerList:
        buzzer['isLocked'] = True
    emitBuzzers(buzzerList)


@socketio.on('test_unlock_all')
def testUnlockAll():
    print('Reset lock all buzzers')
    for buzzer in buzzerList:
        buzzer['isLocked'] = False
    emitBuzzers(buzzerList)


def emitBuzzers(buzzers):
    emit(config.events['buzzers'], json.dumps(buzzers))