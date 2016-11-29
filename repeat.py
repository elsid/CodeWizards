#!/usr/bin/env python3

from sys import argv
from time import sleep

import strategy

from auto_kill_process import AutoKillProcess


if __name__ == '__main__':
    token = argv[1]
    repeater = ['java', '-Xms128M', '-Xmx1G', '-cp', '.:*', '-jar', 'repeater/repeater.jar', token]
    with AutoKillProcess(repeater):
        sleep(3)
        if len(argv) > 2 and argv[2] == 'cpp':
            strategy.cpp()
        else:
            strategy.python3()
