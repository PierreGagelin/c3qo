#!/usr/bin/env python3


from robot.api import logger

import subprocess
import os
import sys


sys.dont_write_bytecode = True


def get_log_value(logger_level):
    """
    Convert a log string into a c3qo log level
    """
    if False:
        return -1
    elif logger_level == "LOG_DEBUG":
        return 8
    elif logger_level == "LOG_INFO":
        return 7
    elif logger_level == "LOG_NOTICE":
        return 6
    elif logger_level == "LOG_WARNING":
        return 5
    elif logger_level == "LOG_ERR":
        return 4
    elif logger_level == "LOG_CRIT":
        return 3
    elif logger_level == "LOG_ALERT":
        return 2
    elif logger_level == "LOG_EMERGENCY":
        return 1
    elif logger_level == "LOG_NONE":
        return 0
    else:
        return -1


class c3qo(object):
    """
    Exposed to Robot Framework

    Class responsible of c3qo binaries
    """

    def __init__(self):
        self.instances = dict()

    def add(self, key, log=None, conf_filename=None, helper=False, arg=None):
        """
        Add an instance of c3qo

        - key: Key to identify the instance
        - log: Specify a log level as a string
            - LOG_DEBUG
            - LOG_INFO
            - LOG_NOTICE
            - LOG_WARNING
            - LOG_ERR
            - LOG_CRIT
            - LOG_ALERT
            - LOG_EMERGENCY
            - LOG_NONE
        - conf_filename: Specify a configuration file
        - helper: Truth value to display help and exit
        - arg: Custom argument to provide to command line

        """
        command = ["/tmp/c3qo-0.0.7-local/bin/c3qo"]

        # Custom argument
        if not (arg is None):
            command += arg.split()

        # Display help and exit
        if helper is True:
            command.append("-h")

        # Add configuration file
        if not (conf_filename is None):
            command.append("-f")
            command.append(str(conf_filename))

        # Choose level of log
        if not (log is None):
            command.append("-l")
            log = str(log)
            log_level = get_log_value(log)
            command.append(str(log_level))

        # Run in background without logs
        command += "> /dev/null 2>&1 &".split()

        self.instances[key] = command

    def run(self, key):
        """
        Run the c3qo executable command
        """
        command = self.instances[key]
        #
        #   TODO: Not really proud of it, several problems:
        #   - shell is used because I need to redirect stdout (to get a clean Robot output)
        #   - I need to join the command in a single string to use shell
        #   - several solutions:
        #       - add an option to c3qo to disable printf
        #       - set log level default to LOG_NONE
        #
        subprocess.run(" ".join(command), shell=True, check=True)
        logger.warn(
            "Started c3qo instance [key={} ; command={}]".format(key, command))

    def check_present(self):
        """
        Check if a c3qo instance is present by checking processes names

        Should be better to keep pid but that's not possible at the moment
        """
        present = False

        entries = os.listdir("/proc/")
        for entry in entries:
            # Look for a process
            if entry.isdigit() is False:
                continue

            # Get the full process name
            try:
                process_path = os.readlink("/proc/" + entry + "/exe")
            except:
                # We probably don't have priviledges
                continue

            # Look for c3qo
            if process_path == "/tmp/c3qo-0.0.7-local/bin/c3qo":
                present = True
                break

        return present

    def stop(self):
        """
        Stop every c3qo processes
        """
        logger.warn("Killed every c3qo instances")
        subprocess.run(["pkill", "c3qo"], check=True)
        self.instances.clear()


#
#   Test Unit
#
if __name__ == "__main__":

    import time

    def check_status(status_expected):
        """
        Wait at most 3 seconds to see expected status
        """
        for _ in range(3):
            if instance.check_present() is status_expected:
                return True
            time.sleep(1)
        return False

    # Started with default should remain present
    instance = c3qo()
    instance.add("toto")
    instance.run("toto")
    assert(check_status(True))
    instance.stop()

    # Started with help should return immediately
    instance = c3qo()
    instance.add("toto", helper=True)
    instance.run("toto")
    assert(check_status(False))

    sys.exit(0)
