#!/usr/bin/env python3

import sys

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from collections import defaultdict

THREADS_PLOTTED = 5

def read_data(file):
    data = defaultdict(lambda: defaultdict(list))
    with open(file) as f:
        for line in f:
            threads, bodies, time = line.split()
            bodies, time = int(bodies), float(time)
            data[threads][bodies].append(time)
    return data

def process(data):
    data["0"] = data["single"]
    del data["single"]
    newdata = [None] * len(data)
    for n,d in data.items():
        newdata[int(n)] = {k: median(v) for k,v in d.items()}
    print(newdata)
    return newdata

def median(list_):
    list_ = sorted(list_)
    l = len(list_)
    if l % 2 == 1:
        return list_[l//2]
    else:
        return 0.5 * (list_[l//2] + list_[l//2 - 1])

def single_plot(ax, info, **kwargs):
    for t,result in enumerate(info[:THREADS_PLOTTED]):
        x = []
        y = []
        for bodies,time in result.items():
            x.append(bodies)
            y.append(time)

        props = {
            # 'elinewidth': 1.5,
            'linewidth': 0.8,
            'markersize': 1.2,
            'linestyle': '-',
            'marker': 'o',
            'markeredgewidth': 0,
        }
        props.update(kwargs)
        g, *_ = ax.plot(x, y, **props)
        threading = f"OpenMP, {t} threads" if t else "Singlethreaded"
        g.set_label(threading)


mycpu = ('Ryzen 5600', 6, 12)
servercpu = ('2x Opteron 6172', 24, 24)

me = ('boron', mycpu, 'tab:green', 'limegreen')
srv = ('subway.sys.ict.kth.se', servercpu, 'tab:blue', 'tab:cyan')

def anal(*sources):
    for s in sources:
        fig = plt.figure(dpi=600,figsize=(8,9))
        file, cpu, *colors = s
        title = '{0}, {1} cores, {2} threads'.format(*cpu)
        # fig.suptitle(title)
        axes = fig.subplots(2, sharex=True)
        axes[1].set_xlabel('Number of bodies')
        for ax ,prog in zip(axes, ('quadratic', 'barneshut')):
            info = process(read_data(f'data/{file}-{prog}'))
            single_plot(ax, info)
            ax.legend(labelcolor='linecolor', framealpha=0.9)
            ax.set_ylabel('Running time (s)')
            ax.set_title(prog)
            # ax.set_xlim((0,250))
            ax.set_ylim((0,None))
        fig.show()
        fig.savefig(f'{file}.pdf',bbox='tight')
    input()

if __name__ == '__main__':
    anal(me, srv)

