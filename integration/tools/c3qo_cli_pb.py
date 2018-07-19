#!/usr/local/bin/python3

import sys
import zmq

# Protobuf generated file
import block_pb2

# Disable bytecode
sys.dont_write_bytecode = 1

# Create a ZMQ client
ctx = zmq.Context.instance()
sock = ctx.socket(zmq.PAIR)

# Create a ZMQ poller
poller = zmq.Poller()
poller.register(sock, zmq.POLLIN)

# Connect to c3qo
sock.connect('tcp://127.0.0.1:1664')

# Send a stats request
topic = b"STATS"
data = b"HELLO"
msg_parts = [topic, data]
sock.send_multipart(msg_parts)

# Wait for a response
events = dict(poller.poll(100))
if not sock in events.keys():
    print("Polling: KO")
    sys.exit(1)

msg_parts = sock.recv_multipart()

print("multi-part received: {}".format(msg_parts))

msg_pb = block_pb2.pb_msg_block()
msg_pb.ParseFromString(msg_parts[1])

print("has type: {}".format(msg_pb.HasField("type")))
print("type: {}".format(msg_pb.type))
