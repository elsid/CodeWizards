from os import environ

from auto_kill_process import AutoKillProcess


def python3():
    sort = environ.get('CPROFILE')
    if sort:
        args = ['python3', '-m', 'cProfile', '-s', (sort if sort != '1' else 'tottime'), 'Runner.py']
    else:
        args = ['python3', 'Runner.py']
    with AutoKillProcess(args, cwd='python3-cgdk', env=environ) as strategy:
        strategy.wait()


def cpp():
    with AutoKillProcess([environ.get('BIN')], env=environ) as strategy:
        strategy.wait()
