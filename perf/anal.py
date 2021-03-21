#!/usr/bin/env python3

import sys

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

def read_data(file):
    data = [[] for _ in range(32)]
    with open(file) as f:
        for line in f:
            threads, time = line.split()
            threads, time = int(threads), float(time)
            data[threads - 1].append(time)
    return [np.array(times) for times in data]

def stats(data):
    avgs = np.array([np.average(t) for t in data])
    vars = np.array([np.var(t,ddof=1) for t in data])
    invtime = 1/avgs
    error = np.sqrt(vars) * invtime ** 2
    return invtime, error

def plot_stats(ax, stats, **kwargs):
    y, dy = stats['data']
    name, cores, threads = stats['cpu']
    linecol, vertcol = stats['colors']
    x = range(1,len(y)+1)
    if old:
        props = {
            'c': vertcol,
            'elinewidth': 0.8,
            'linewidth': 0.5,
        }
    else:
        props = {
            'c': linecol,
            'elinewidth': 1.5,
            'linewidth': 1,
        }
    props.update({
        'fmt': '.-',
        'markeredgewidth': 0,
        'markeredgecolor': vertcol,
    }, **kwargs)
    eb, *_ = ax.errorbar(x, y, dy, **props)
    if old:
        eb.set_label(f'{name}: pthreads')
    else:
        eb.set_label(f'{name}: {cores} cores, {threads} threads')
        coreline = ax.axvline(cores, ls='-.', c=vertcol, lw=0.6, zorder=-1)
        if threads != cores:
            threadline = ax.axvline(threads, ls='-.', c=vertcol, lw=0.6, zorder=-1)


mycpu = ('Ryzen 5600', 6, 12)
servercpu = ('2x Opteron 6172', 24, 24)

me = ('boron', mycpu, 'tab:green', 'limegreen')
srv = ('subway.sys.ict.kth.se', servercpu, 'tab:blue', 'tab:cyan')

def anal(*sources):
    fig = plt.figure(dpi=600,figsize=(8,4))
    ax = fig.add_subplot()
    for s in sources:
        file, cpu, *colors = s
        meme = {
            'data': stats(read_data(file)),
            'cpu': cpu,
            'colors': colors
        }
        plot_stats(ax, meme)
    ax.legend(labelcolor='linecolor', framealpha=0.9)
    ax.set_ylabel('Runs per second')
    ax.set_xlabel('Worker threads')
    ax.set_xlim((0,33))
    ax.set_ylim((0,18))
    fig.show()
    fig.savefig(sys.argv[1],bbox='tight')
    input()

if __name__ == '__main__':
    anal(me, srv)

