from os import environ

from auto_kill_process import AutoKillProcess


def run():
    if environ.get('CPROFILE') == '1':
        args = ['python3', '-m', 'cProfile', '-s', 'tottime', 'Runner.py']
    else:
        args = ['python3', 'Runner.py']
    with AutoKillProcess(args, cwd='python3-cgdk', env=environ) as strategy:
        strategy.wait()
