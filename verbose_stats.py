#!/usr/bin/env python3

from argparse import ArgumentParser, FileType
from collections import Counter
from json import loads
from matplotlib.pyplot import show
from sys import stdin
from itertools import groupby
from operator import itemgetter


def main():
    args = parse_args()
    events = read(args.file)
    ordered = sorted(events, key=itemgetter('id'))
    grouped = groupby(ordered, key=itemgetter('id'))
    for wizard_id, events in grouped:
        events = sorted(events, key=itemgetter('tick'))
        ticks_count = sum(1 for v in events if v['name'] == 'start')
        sum_duration = sum(v.get('duration', 0) for v in events)
        min_life = min(v['life'] for v in events if v['name'] == 'start')
        actions_count = sum(1 for v in events if v['name'] == 'apply_target_action')
        staff_actions_count = sum(1 for v in events if v['name'] == 'apply_target_action' and v['type'] == 'STAFF')
        magic_missile_actions_count = sum(1 for v in events if v['name'] == 'apply_target_action'
                                          and v['type'] == 'MAGIC_MISSILE')
        max_statuses_count = max(len(v['statuses']) for v in events if v['name'] == 'start')
        ticks_with_statuses = dict()
        for n in range(1, 1 + max_statuses_count):
            ticks_with_statuses[n] = sum(1 for v in events if v['name'] == 'start' and len(v['statuses']) >= n)
        movement_error_overflow_count = sum(1 for v in events if v['name'] == 'movement_error_overflow')
        calculate_movements_count = sum(1 for v in events if v['name'] == 'calculate_movements')
        movements_updated_count = sum(1 for v in events if v['name'] == 'movements_updated')
        get_target_count = sum(1 for v in events if v['name'] == 'get_target')
        target_updated_count = sum(1 for v in events if v['name'] == 'target_updated')
        change_mode_count = sum(1 for v in events if v['name'] == 'change_mode')
        score_increases_count = score_increases_distribution(events)
        print('ticks count: %s' % ticks_count)
        print('sum duration: %s' % sum_duration)
        print('min life: %s' % min_life)
        print('sum damage: %s' % sum_damage(events))
        print('actions count: %s' % actions_count)
        print('staff actions count: %s' % staff_actions_count)
        print('magic missile actions count: %s' % magic_missile_actions_count)
        print('score_increases: %s %s' % (sum(score_increases_count.values()), score_increases_count))
        print('ticks with statuses: %s' % ticks_with_statuses)
        print('movement error overflow count: %s' % movement_error_overflow_count)
        print('calculate_movements_count: %s' % calculate_movements_count)
        print('movements_updated_count: %s' % movements_updated_count)
        print('get_target_count: %s' % get_target_count)
        print('target_updated_count: %s' % target_updated_count)
        print('change_mode_count: %s' % change_mode_count)
    show()


def parse_args():
    parser = ArgumentParser()
    parser.add_argument('file', type=FileType('r'), default=stdin, nargs='?')
    return parser.parse_args()


def read(stream):
    for line in stream:
        try:
            yield loads(line)
        except:
            pass


def sum_damage(events):
    prev_life = None
    result = 0
    for event in events:
        if event['name'] == 'start':
            if prev_life is None:
                prev_life = event['life']
            elif prev_life > event['life']:
                result += prev_life - event['life']
                prev_life = event['life']
    return result


def score_increases_distribution(events):
    prev_score = 0
    result = Counter()
    for event in events:
        if event['name'] == 'start':
            if prev_score < event['player']['score']:
                result[event['player']['score'] - prev_score] += 1
                prev_score = event['player']['score']
    return result


if __name__ == '__main__':
    main()
