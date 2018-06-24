#!/usr/bin/python3

import argparse
import matplotlib.pyplot as plt

def plotOrbsStats(stats_dir):
    f = open(stats_dir + "/orbs.stats", "rb")
    try:
        data = f.read(4)
        x = []
        while data:
            i = int.from_bytes(data, byteorder="little")
            x.append(i)
            data = f.read(4)
    finally:
      f.close()

    plt.plot(x)
    plt.title("Number of orbs")
    plt.show()

def plotAvgLifeTimeStats(stats_dir):
    f = open(stats_dir + "/avg_life_time.stats", "rb")
    try:
        data = f.read(4)
        x = []
        while data:
            i = int.from_bytes(data, byteorder="little")
            x.append(i)
            data = f.read(4)
    finally:
      f.close()

    plt.plot(x)
    plt.title("Average life time")
    plt.show()

def plotAvgInstrUsageStats(stats_dir):
    f = open(stats_dir + "/avg_instr_usage.stats", "rb")
    try:
        data = f.read(4)
        x = []
        while data:
            i = int.from_bytes(data, byteorder="little") / 1000.0
            x.append(i)
            data = f.read(4)
    finally:
      f.close()

    plt.plot(x)
    plt.title("Average instruction usage")
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--stats", required=True,
                        help="Directory with statistics files")
    args = parser.parse_args()

    # plot statistics
    plotOrbsStats(args.stats)
    plt.show()

    plotAvgLifeTimeStats(args.stats)
    plt.show()

    plotAvgInstrUsageStats(args.stats)
    plt.show()

    # show plots
    plt.show()
