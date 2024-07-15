hostname = '127.0.0.1'
port = 5000
events = {
    "connect": "connect",
    "disconnect": "disconnect",
    "buzzers": "buzzers",
    "reset": "reset",
    "lock": "lock",
    "remove": "remove",
    "pairing": "pairing"
}
publish_port = 5556
subscribe_port = 5555
channels = {
    "reset": "reset, ",
    "pairing": "pairing, ",
    "remove": "remove, ",
    "buzzers": ""
}