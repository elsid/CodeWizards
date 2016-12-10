#!/usr/bin/env python3

from os import listdir
from os.path import join, isdir, exists
from sys import argv
from collections import Counter
from statistics import mean, median, variance


def print_stats(test_path):
    positions = Counter()
    scores = list()
    results = Counter()
    for run in listdir(test_path):
        run_path = join(test_path, run)
        if not isdir(run_path):
            continue
        result_path = join(run_path, 'result.txt')
        if not exists(result_path):
            continue
        with open(result_path) as f:
            lines = [v for v in f]
            my_position, my_score, my_result = lines[2].strip().split()
            results[my_result] += 1
            positions[int(my_position)] += 1
            scores.append(int(my_score))
    print('results: %s' % ' '.join('%s: %s' % (k, results[k]) for k in sorted(results.keys())))
    print('positions: %s' % ' '.join('%s: %s (%s)' % (k, positions[k], positions[k] / sum(positions.values()))
                                     for k in sorted(positions.keys())))
    if len(scores) == 1:
        scores += scores
    print('scores: min: {min} max: {max} mean: {mean} median: {median} variance: {variance}'.format(
        min=min(scores), max=max(scores), mean=mean(scores), median=median(scores), variance=variance(scores)
    ))


if __name__ == '__main__':
    print_stats(argv[1])
