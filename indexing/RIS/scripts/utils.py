#!/usr/bin/env python3

'''
Utils for RIQ scripts
'''

import json
import os
import re
import subprocess
import sys


def clear_cache():
    '''Clear system cache'''
    print("clearing cache...")
    os.system('sync')
    # open('/proc/sys/vm/drop_caches','w').write("1\n")
    with open('/proc/sys/vm/drop_caches', 'w') as stream:
        stream.write('1\n')


def write_json(json_file, results_d, verbose):
    with open(json_file, 'w') as f:
        json.dump(results_d, f)

    if verbose:
        print(json.dumps(results_d, sort_keys=True, indent=4))


# XXX: better way?
# TODO: add check for options too
def verify_config(config):
    '''Verify config has all required sections'''
    sections = ['RIQ',
                'TDB',
                'RDF3X',
                'Jaccard',
                'LSH',
                'BDB',
                'Query',
                'Dablooms',
                'Limits',
                'Dataset']

    return sorted(sections) == sorted(config.sections())


def print_config(config):
    '''Print parsed config as key-value pairs'''
    for section_name in config.sections():
        print('Section:', section_name)
        print('  Options:', config.options(section_name))
        for name, value in config.items(section_name):
            print('  {name} = {value}'.format(name=name, value=value))
        print


# TODO: handle empty results better
# TODO: exit if unique as soon as found
def extract_pattern(filename, pattern, unique=True, nonempty=True):
    '''Extract value of pattern from file name using'''
    matched = grep_re(filename, pattern)
    if len(matched) > 1 and unique:
        print('warn: not unique')

    vals = []
    for m in matched:
        vals.append(m.split()[-1])

    if unique and vals:
        return vals[0]

    if not vals:
        print('warn: empty pattern extraction')
        if nonempty:
            print('error: non-empty pattern extraction required')
            print('file: {0}'.format(filename))
            print('pattern: {0}'.format(pattern))
            # TODO: raise exception?
            sys.exit(1)

    return vals


# TODO: finish?
def grep_shell(filename, pattern, args, pipe_cmd):
    '''
    Grep pattern from file using 'grep' command
    Not finished
    '''
    cmd = ['grep']
    cmd.extend(args)
    cmd.extend((pattern, filename))
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    # return stdout, stderr
    return stdout


def grepc_shell(filename, pattern):
    '''
    Count number of pattern occurences in a file using 'grep' command'
    (source:
     http://code.activestate.com/recipes/577069-access-grep-from-python/)
    '''
    process = subprocess.Popen(['grep', '-c', pattern, filename],
                               stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    # return stdout, stderr
    return stdout


def grep_re(filename, pattern):
    '''
    Grep pattern from file using Python.
    Pattern is valid regex, have to escape special chars
    (source:
     http://code.activestate.com/recipes/577069-access-grep-from-python/)
    '''
    matched = []
    grepper = re.compile(pattern)
    with open(filename) as f:
        for line in f:
            if grepper.search(line):
                matched.append(line)

    return matched


def hms2sec(t):
    '''
    Convert h:mm:ss or m:ss strings to number of seconds
    (source: stackoverflow.com)
    '''
    return sum(float(x) * 60 ** i
               for i, x in enumerate(reversed(t.split(":"))))


# TODO: time
def count_lines(filename):
    '''
    Count number of lines in a file
    (source: stackoverflow.com)
    '''
    return sum(1 for line in open(filename))


# TODO: time
def count_lines2(filename):
    '''
    Count number of lines in a file
    (source: http://stackoverflow.com/questions/845058/)
    '''
    with open(filename) as f:
        for i, l in enumerate(f, 1):
            pass

    return int(i)


if __name__ == "__main__":
    sys.exit(main())
