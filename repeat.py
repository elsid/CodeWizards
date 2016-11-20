#!/usr/bin/env python3

from sys import argv

import strategy

from auto_kill_process import AutoKillProcess


if __name__ == '__main__':
    token = argv[1]
    repeater = ['java', '-Xms128M', '-Xmx1G', '-cp', '.:*', '-jar', 'repeater/repeater.jar', token]
    with AutoKillProcess(repeater):
        strategy.run()
