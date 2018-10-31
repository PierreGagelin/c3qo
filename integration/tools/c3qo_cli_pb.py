#!/usr/local/bin/python3

import sys
import zmq

# Protobuf generated file
import block_pb2
import conf_pb2

# Disable bytecode
sys.dont_write_bytecode = 1

# Create a ZMQ client
ctx = zmq.Context.instance()
sock = ctx.socket(zmq.PAIR)

# Create a ZMQ poller
poller = zmq.Poller()
poller.register(sock, zmq.POLLIN)

# Connect to c3qo
sock.connect('tcp://127.0.0.1:5555')

def send_pbc_cmd(cmd_type, bk_id, bk_arg = ""):
    """
    Send a protobuf management command
    """
    topic = b"CONF.PROTO.CMD"
    data = conf_pb2.pbc_cmd()

    # customize
    data.type = cmd_type
    data.block_id = bk_id
    data.block_arg = bk_arg

    data = data.SerializeToString()
    msg_parts = [topic, data]
    sock.send_multipart(msg_parts)

# Send management commands to c3qo
send_pbc_cmd(conf_pb2.pbc_cmd.CMD_ADD, 1, "hello")
send_pbc_cmd(conf_pb2.pbc_cmd.CMD_INIT, 1)
send_pbc_cmd(conf_pb2.pbc_cmd.CMD_START, 1)
send_pbc_cmd(conf_pb2.pbc_cmd.CMD_STOP, 1)


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
