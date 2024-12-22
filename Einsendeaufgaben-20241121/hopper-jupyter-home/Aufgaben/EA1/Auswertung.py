import matplotlib as mpl # imortiert das Hauptmodul
import matplotlib.pyplot as plt
import os
import subprocess

times = []
threads_exp_min = 1
threads_exp_max = 6
threads = [2**n for n in range(threads_exp_min, threads_exp_max+1)]
start = 16
length = 5
size = [2**n for n in range(start, start+length+1)]


# Weak Scaling
for n in zip(threads, size):
    cmd = f"./mergesort {str(n[0])} {str(n[1])}"
    output = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0]
    #print(output)
    times.append(float(str(output).split()[6]))
    print("{} {}".format(n, (str(output).split()[6])))

fig, ax = plt.subplots() # neuer Plot
ax.plot(threads,times, color= 'red',marker="*")
ax.set_xscale("log", base=2)
ax.set_xlabel("Threads")
ax.set_ylabel("Time [s]")
ax.set_title("Weak scaling")

fig.savefig("WeakSkaling.png")

speedup=[times[0]/t for t in times[1:]]
fig,ax = plt.subplots() # neuer Plot
ax.bar([str(thr) for thr in threads[1:]],speedup, color= 'red')
ax.set_xlabel("Threads")
ax.set_title("Weak scaling Speedup")
fig.savefig("WeakSkalingSpeedup.png")

# Strong Scaling für 2**16
times =[]

for n in threads:
    cmd = f"./mergesort {n} {size[4]}"
    output = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0]
    #print(output)
    times.append(float(str(output).split()[6]))
    print("{} {}".format(n, (str(output).split()[6])))

fig, ax = plt.subplots() # neuer Plot
ax.plot(threads,times, color= 'red',marker="*")
ax.set_xscale("log", base=2)
ax.set_xlabel("Threads")
ax.set_ylabel("Time [s]")
ax.set_title(f"Strong scaling für {size[0]}")

fig.savefig("StrongSkaling16.png")

speedup=[times[0]/t for t in times[1:]]
fig,ax = plt.subplots() # neuer Plot
ax.bar([str(thr) for thr in threads[1:]],speedup, color= 'red')
ax.set_xlabel("Threads")
ax.set_title("Strong scaling Speedup for n=$2^{16}$")
fig.savefig("StrongSkalingSpeedup16.png")

# Strong Scaling für 2**20
times =[]

for n in threads:
    cmd = f"./mergesort {n} {size[4]}"
    output = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0]
    #print(output)
    times.append(float(str(output).split()[6]))
    print("{} {}".format(n, (str(output).split()[6])))

fig, ax = plt.subplots() # neuer Plot
ax.plot(threads,times, color= 'red',marker="*")
ax.set_xscale("log", base=2)
ax.set_xlabel("Threads")
ax.set_ylabel("Time [s]")
ax.set_title(f"Strong scaling für {size[4]}")

fig.savefig("StrongSkaling20.png")

speedup=[times[0]/t for t in times[1:]]
fig,ax = plt.subplots() # neuer Plot
ax.bar([str(thr) for thr in threads[1:]],speedup, color= 'red')
ax.set_xlabel("Threads")
ax.set_title("Strong scaling Speedup for n=$2^{20}$")
fig.savefig("StrongSkalingSpeedup20.png")
