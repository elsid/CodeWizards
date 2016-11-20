#!/usr/bin/env python3

import strategy

from time import sleep

from auto_kill_process import AutoKillProcess


if __name__ == '__main__':
    runner = ['java', '-Xms512m', '-Xmx2G', '-server', '-jar',
              'local-runner-ru/local-runner.jar', 'local-runner.properties']
    with AutoKillProcess(runner):
        sleep(3)
        strategy.run()
