import matplotlib.pyplot as plt
import math

plt.rcParams.update({"font.size": 20})
import matplotlib.colors as mcolors
import os

colors = list(mcolors.TABLEAU_COLORS)


def display(folder="Results.hopper", run=1, threads=32, flops=6):
    folder2 = os.path.join(
        folder, f"Run.{run:03d}", f"FLOPS.{flops:03d}", f"OpenMP.{threads:04d}", "sum"
    )

    if not os.path.exists(folder2):
        print("Results file not found in", folder2)
        return
    else:
        print("Reading", folder2)

    with open(folder2) as file:
        lines = [line.split() for line in file if len(line) > 1]
    max_flops = float(lines[0][0])
    mem_limits = []
    i = 2
    while lines[i][0] != "META_DATA":
        mem_limits.append(float(lines[i][0]))
        i += 1
    intensities = [max_flops / lim for lim in mem_limits]

    plt.rcParams.update({"font.size": 14})
    fig, ax = plt.subplots(figsize=(10, 7))
    x = set([2 ** i for i in range(-8, 8)])
    for intensity in intensities:
        x.add(intensity)

    x = list(x)
    x.sort()
    mem_y = []
    for i in range(len(mem_limits)):
        mem_y.append(
            [
                intens * mem_limits[i] if intens < intensities[i] else max_flops
                for intens in x
            ]
        )
    for i in range(len(mem_y) - 1):
        plt.plot(
            x,
            mem_y[i],
            color=colors[i],
            label="L" + str(i + 1) + " " + str(mem_limits[i]) + " GB/s",
        )
    i += 1
    plt.plot(x, mem_y[i], color=colors[i], label="DRAM " + str(mem_limits[i]) + " GB/s")

    plt.plot([min(intensities), x[-1]], [max_flops, max_flops], "m-", linewidth=2)
    plt.text(min(intensities), max_flops + 100, f"{max_flops} GFLOPs/sec")
    plt.legend()
    plt.xscale("log")
    plt.yscale("log")
    plt.xlim([0.01, 100])
    plt.ylim([10, max_flops + 500])
    xticks = [0.01, 0.1, 1, 10, 100]
    yticks = [10, 100, 1000]
    ax.set_xticks(xticks)
    ax.set_xticklabels(xticks)
    ax.set_yticks(yticks)
    ax.set_yticklabels(yticks)
    plt.xlabel("FLOPs/Bytes")
    plt.ylabel("GFLOPs/sec")
    plt.grid(which="both")
    plt.savefig("Results.hopper/roofline.png")
    plt.show()