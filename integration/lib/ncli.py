#!/usr/bin/env python3


from robot.api import logger

import subprocess
import sys


sys.dont_write_bytecode = True


class ncli(object):
    """
    Exposed to Robot Framework

    Class responsible to send network CLI commands
    """

    def send(self, cmd_type, cmd_arg, addr = None):
        """
        Add an instance of c3qo

        - cmd_type: Type of command to send ("raw" or "proto")
        - cmd_arg: Argument to give for the command

        """
        cmd_type_list = ["raw", "proto"]
        cmd_type = str(cmd_type)
        cmd_arg = str(cmd_arg)
        cmd_arg = cmd_arg.replace('"', '\\"')

        if not (cmd_type in cmd_type_list):
            raise ValueError("Command type value not in expected list [actual={} ; expected_list={}]".format(
                cmd_type, cmd_type_list))

        command = list()
        command += ["/tmp/c3qo-0.0.7-local/bin/ncli"]
        if addr is not None:
            command += ["-a", addr]
        command += ["-T", cmd_type]
        command += ["-A", '"' + cmd_arg + '"']

        #
        # Run without logs
        # See TODO in c3qo.py library, same issue here
        #
        command += "> /dev/null 2>&1".split()
        logger.warn(
            "Network CLI command sent [command={}]".format(command))
        subprocess.run(" ".join(command), shell=True, check=True)


#
#   Test Unit
#
if __name__ == "__main__":

    instance = ncli()

    # Exception shall be raised
    try:
        instance.send("toto", "dummy")
        assert(False)
    except:
        pass

    sys.exit(0)
