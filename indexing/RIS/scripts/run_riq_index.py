#!/usr/bin/env python3

'''
Index RDF data with RIQ
'''

# TODO: capture run times in Python too?

# from __future__ import print_function
import argparse
from configparser import ConfigParser, ExtendedInterpolation
import json
import os
import subprocess
import sys

# local
from utils import *


def main():
    # process arguments
    parser = argparse.ArgumentParser(description='Index RDF data with RIQ')
    parser.add_argument('-C', '--config', required=True, help='config file')
    parser.add_argument('-s', '--steps', required=False, default='all',
                        choices=('all', 'bloom', 'group', 'split'),
                        help='index steps (default: all)')
    parser.add_argument('-v', '--verbose', required=False,
                        action='store_true',
                        help='Print JSON object results')
    args = parser.parse_args()
    # print(args)

    # process config file
    config_path = args.config
    config = ConfigParser(interpolation=ExtendedInterpolation())
    config.read(config_path)
    # print_config(config)

    if not verify_config(config):
        print('error: invalid config')
        return 1

    log_pre = build_log_pre(config)

    if args.steps in ('all', 'group'):
        riq_log = call_bin(config, log_pre, 'riq')
        print('getting group results...')
        group_results_d = {}
        get_group_results(riq_log, group_results_d)

        print('writing group json file...')
        riq_json = '.'.join([log_pre, 'riq', 'json'])
        write_json(riq_json, group_results_d, args.verbose)

    if args.steps in ('all', 'bloom'):
        cbf_log = call_bin(config, log_pre, 'cbf')
        print('getting bloom results...')
        bloom_results_d = {}
        get_bloom_results(cbf_log, bloom_results_d)

        print('writing cbf json file...')
        cbf_json = '.'.join([log_pre, 'cbf', 'json'])
        write_json(cbf_json, bloom_results_d, args.verbose)

    if args.steps in ('all', 'split'):
        split_log = call_bin(config, log_pre, 'split')
        print('getting split results...')
        split_results_d = {}
        split_timev = '.'.join([log_pre, 'split', 'timev'])
        get_split_results(split_timev, split_results_d)

        print('writing split json file...')
        split_json = '.'.join([log_pre, 'split', 'json'])
        write_json(split_json, split_results_d, args.verbose)

    if args.steps == 'all':
        all_results_d = {}
        all_results_d['group'] = group_results_d
        all_results_d['cbf'] = bloom_results_d
        all_results_d['split'] = split_results_d

        print('writing all json file...')
        all_json = '.'.join([log_pre, 'all', 'json'])
        write_json(all_json, all_results_d, args.verbose)

    return 0


def get_group_results(log, results_d):
    '''Extract group results'''
    results_d['groups'] = int(extract_pattern(log,
                                              '^Total connected components:'))
    results_d['pv_lsh_t'] = float(extract_pattern(log, '^Reading PVs'))
    results_d['lsh2gids_size'] = int(extract_pattern(log,
                                                     '^lsh2gids\.size'))
    results_d['build_graph_t'] = float(extract_pattern(log,
                                                   '^Building graph time:'))
    results_d['union_t'] = float(extract_pattern(log,
                                                 '^Union construction time:'))
    results_d['total_t'] = float(extract_pattern(log,
                                                 '^Total indexing time:'))


def get_bloom_results(log, results_d):
    '''Extract cbf results'''
    results_d['cbf_t'] = float(extract_pattern(log, '^Total CBF time:'))


def get_split_results(log, results_d):
    '''Extract split results'''
    results_d['split_t'] = hms2sec(extract_pattern(log, 'Elapsed'))


def call_bin(config, log_pre, bin_str):
    '''Call binary'''

    methods = {'riq': build_riq_cmd,
               'cbf': build_cbf_cmd,
               'split': build_split_cmd}

    log_pre = '.'.join([log_pre, bin_str])
    # print("log_pre: {0}".format(log_pre))

    if bin_str in methods:
        cmd = methods[bin_str](config, log_pre)
        # print("cmd: {0}".format(' '.join(cmd)))
    else:
        print("method {0} not implemented".format(bin_str))
        # TODO: raise exception?
        sys.exit(1)

    log = '.'.join([log_pre, 'log'])
    # print("log: {0}".format(log))

    if bin_str == 'riq':
        # TODO: use config
        group_with = 'LSH'
        print('grouping similar graphs using {0}...'.
              format(group_with))
    elif bin_str == 'cbf':
        print('generating CBFs...')
    elif bin_str == 'split':
        print('splitting graphs...')

    with open(log, 'w') as outerr:
        proc = subprocess.Popen(cmd, shell=False,
                                stdout=outerr, stderr=outerr)
        result = proc.communicate()
        # print(result)

    return log


def build_riq_cmd(config, log_pre):
    '''Build RIQ command line call string'''
    dablooms_cfg = config['Dablooms']
    riq_cfg = config['RIQ']
    bdb_cfg = config['BDB']
    limits_cfg = config['Limits']
    data_cfg = config['Dataset']

    riq_cmd = [riq_cfg['time'], '-v', '-o',
               '.'.join([log_pre, 'timev']),
               riq_cfg['riq'],
               '-D', bdb_cfg['dbname'],
               '-E', bdb_cfg['dbenv'],
               '-b', bdb_cfg['numbuf'],
               '-d', '/'.join([riq_cfg['risrun'], 'index']),
               '-I',
               '-P', data_cfg['pv_file'],
               '-m', limits_cfg['maxsigs']]

    if riq_cfg['group_with'] == 'LSH':
        lsh_cfg = config['LSH']
        riq_cmd.extend(['-L',
                        '-k', lsh_cfg['k'],
                        '-l', lsh_cfg['l'],
                        '-i', lsh_cfg['irrpoly'],
                        '-H', lsh_cfg['hash_file'],
                        '-C', limits_cfg['maxccsize'],
                        '-N', limits_cfg['minccsize'],
                        '-U', limits_cfg['maxccunionsize']])

        if lsh_cfg.get('skiplists'):
            riq_cmd.extend(['-S', lsh_cfg['skiplists']])
    elif riq_cfg['group_with'] == 'Jaccard':
        jaccard_cfg = config['Jaccard']
        riq_cmd.extend(['-J',
                        '-f', jaccard_cfg['fanout'],
                        '-s', jaccard_cfg['seed_size']])
    else:
        print('invalid method of grouping')
        # TODO: raise exception?
        sys.exit(1)

    if riq_cfg.getboolean('verbose'):
        riq_cmd.append('-V')

    #return riq_cmd, riq_cfg['group_with']
    return riq_cmd


def build_cbf_cmd(config, log_pre):
    '''Build RIQ cbf command line call string'''
    dablooms_cfg = config['Dablooms']
    riq_cfg = config['RIQ']

    cbf_cmd = [riq_cfg['time'], '-v', '-o',
               '.'.join([log_pre, 'timev']),
               riq_cfg['cbf'],
               '-d', '/'.join([riq_cfg['risrun'], 'index']),
               '-c', dablooms_cfg['capacity'],
               '-e', dablooms_cfg['error_rate'],
               '-M', dablooms_cfg['cbfmeta_file']]

    return cbf_cmd


def build_split_cmd(config, log_pre):
    '''Build split graph command line call string'''
    data_cfg = config['Dataset']
    riq_cfg = config['RIQ']

    split_cmd = [riq_cfg['time'], '-v', '-o',
               '.'.join([log_pre, 'timev']),
               riq_cfg['split'],
               data_cfg['nt_file'],
               data_cfg['off_file'],
               data_cfg['prefix_file'],
               data_cfg['format']]

    return split_cmd


def build_log_pre(config):
    '''Build prefix string used for constructing log file names'''
    pvs_cfg = config['Dataset']
    riq_cfg = config['RIQ']

    pv_file = os.path.basename(pvs_cfg['PV_FILE']).split('.')[0]
    log_pre = '.'.join(['index', pv_file])
    log_pre = '/'.join([riq_cfg['log'], log_pre])
    # print("log_pre: {0}".format(log_pre))

    return log_pre


if __name__ == "__main__":
    sys.exit(main())
