#!/usr/bin/env python3

from time import time, sleep
from shutil import copy
from os import mkdir, chmod
from os.path import join, exists
from sys import argv
from collections import Counter
from auto_kill_process import AutoKillProcess
from mult_run_test_stats import print_stats

runner = ['java', '-Xms512m', '-Xmx2G', '-server', '-jar', 'local-runner-ru/local-runner.jar']
test_id = int(time())
test_path = join('tests', str(test_id))
if not exists('tests'):
    mkdir('tests')
mkdir(test_path)
binary = join(test_path, 'cpp-cgdk')
if len(argv) > 2:
    copy(argv[2], binary)
else:
    copy('cpp-cgdk-release/bin/cpp-cgdk', binary)
positions = Counter()
scores = list()
for run in range(int(argv[1])):
    print('run %s ...' % run)
    run_path = join(test_path, str(run))
    command = join(run_path, 'run.sh')
    mkdir(run_path)
    run_log = join(run_path, 'run.log')
    with open(command, 'w') as f:
        f.write('nice -n -20 %s ${@} &> %s\n' % (binary, run_log))
    chmod(command, 0o755)
    config_path = join(run_path, 'config.properties')
    result_path = join(run_path, 'result.txt')
    log_path = join(run_path, 'game.log')
    port = 32001 + (test_id + run) % 1000
    with open('local-runner.mult_run_test.properties') as src:
        with open(config_path, 'w') as dst:
            dst.write(src.read().format(run=run, port=port, result=result_path, log=log_path))
    start = time()
    with AutoKillProcess(runner + [config_path]) as p:
        sleep(3)
        with AutoKillProcess(['bash', command, '127.0.0.1', str(port), '0000000000000000']) as strategy:
            strategy.wait()
        p.wait()
    finish = time()
    with open(result_path) as f:
        lines = [v for v in f]
        result = lines[0].strip()
        seed = lines[1].strip()
        my_position, my_score, my_result = lines[2].strip().split()
        print('finished %s in %s seconds' % (' '.join((result, seed, my_position, my_score, my_result)), finish - start))
    print_stats(test_path)
print_stats(test_path)
print('test path: %s' % test_path)
