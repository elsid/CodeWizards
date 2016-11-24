#!/usr/bin/env python3

import strategy

from time import sleep
from sys import argv

from auto_kill_process import AutoKillProcess


if __name__ == '__main__':
    runner = ['java', '-Xms512m', '-Xmx2G', '-server', '-jar',
              'local-runner-ru/local-runner.jar', argv[1]]
    with AutoKillProcess(runner):
        sleep(3)
        if len(argv) > 2 and argv[2] == 'cpp':
            strategy.cpp()
        else:
            strategy.python3()
