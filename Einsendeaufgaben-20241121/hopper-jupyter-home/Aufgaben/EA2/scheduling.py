# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def write_times(N, sort=False, filename=None):
    # Zeitbedarf für N Kleidungsstücke (Minuten)
    schau = np.ones(N) * 5.0
    vor = np.ones(N) * 7.0
    wasch = np.ones(N) * 10.0
    nach = np.ones(N) * 6.0
    finish = np.ones(N) * 3.5

    # Randomisierung/Filterung der Zeiten für die mittleren drei Arbeitsschritte
    nach = nach * np.random.randint(0, 2, N)
    vor[np.random.randint(0, 4, N) > 0] = 0
    wasch = wasch * (1 - 0.05 * np.random.randint(-5, 7, N))

    # Berechnen der Gesamtzeit und Speichern als .csv
    zeiten = pd.DataFrame(
        {
            "Schauen": schau,
            "Vordetachur": vor,
            "Waschen": wasch,
            "Nachdetachur": nach,
            "Finish": finish,
        }
    )
    zeiten["gesamt"] = (
        zeiten["Schauen"]
        + zeiten["Vordetachur"]
        + zeiten["Waschen"]
        + zeiten["Nachdetachur"]
        + zeiten["Finish"]
    )

    zeiten2 = zeiten.sort_values("Waschen", ignore_index=True)

    if filename:
        if sort == True:
            zeiten2.to_csv(filename, index=False)
        else:
            zeiten.to_csv(filename, index=False)

    if sort == "both":
        return zeiten, zeiten2
    if sort == False:
        return zeiten
    if sort == True:
        return zeiten2


def read_times(filename="Waschzeiten.csv"):
    zeiten = pd.read_csv(filename)
    return zeiten


def visualize_times(zeiten, title="zeiten"):
    N = len(zeiten)
    fig, ax = plt.subplots(1, 1, figsize=(20, 6))
    zeiten[["Schauen", "Vordetachur", "Waschen", "Nachdetachur", "Finish"]].plot(
        kind="bar", stacked=True, ax=ax, title=title
    )
    ax.set_xticks(range(0, N, 100))
    ax.legend(ncol=4)


class waescherei:
    def __init__(self, file=None, zeiten=None, nworkers=8, overhead=0.5, p=400):
        self.n_workers = nworkers
        self.scheduling_overhead = overhead
        self.current_time = 0
        self.x_axis_limit = 0
        self.current_item = nworkers
        if file is not None:
            self.zeiten = read_times(file)
        elif zeiten is not None:
            self.zeiten = zeiten
        else:
            self.zeiten = write_times(p, filename="tmp.csv")

        self.n_jobs = len(self.zeiten)
        self.worker_finish_time = np.zeros(self.n_workers, dtype=np.float32)
        self.i_next_job = 0
        self.queue = []
        self.start_times = []
        for i in range(self.n_workers):
            self.start_times.append({})

    def add_idle_workers_to_queue(self):
        for i_worker, finish_time in enumerate(self.worker_finish_time):
            if self.current_time >= finish_time and i_worker not in self.queue:
                self.queue.append(i_worker)

    def run(self, loud=False):
        while self.i_next_job < self.n_jobs:
            self.progress(loud)

    def progress(self, loud=True):
        self.current_time = max(self.current_time, min(self.worker_finish_time))
        self.add_idle_workers_to_queue()
        self.current_time += self.scheduling_overhead
        i_idle_worker = np.random.randint(0, len(self.queue))
        self.schedule_next_job(self.queue[i_idle_worker], loud)
        self.i_next_job += 1
        self.queue.pop(i_idle_worker)  # removes worker from idle queue
        if self.current_item > min(self.worker_finish_time):
            self.add_idle_workers_to_queue()
        return self.i_next_job

    def schedule_next_job(self, i_worker, loud=False):
        if loud:
            print(
                f"scheduling job {self.i_next_job} on worker {i_worker} at time {self.current_time}, {list(self.worker_finish_time)}"
            )
        self.worker_finish_time[i_worker] = (
            self.zeiten["gesamt"][self.i_next_job] + self.current_time
        ).round(1)
        self.start_times[i_worker][self.current_time] = self.zeiten["gesamt"][
            self.i_next_job
        ]
        self.x_axis_limit = max(self.x_axis_limit, self.worker_finish_time[i_worker])
    
    def visualize_schedule_dynamic(self, xlimits=None):
        fig, ax = plt.subplots(1, figsize=(16, self.n_workers))
        for i in range(self.n_workers):
            if not self.start_times[i]:
                # create a dummy bar
                ax.barh(
                    y=i,
                    width=[1],
                    left=[0],
                    alpha=0.0,
                )
            else:
                ax.barh(
                    y=i,
                    width=list(self.start_times[i].values()),
                    left=list(self.start_times[i].keys()),
                    linewidth=1,
                    edgecolor="black",
                )
        plt.ylabel("Worker")
        plt.xlabel("Time")
        plt.title("Task schedule dynamic")
        plt.grid(axis="x", alpha=0.5)
        plt.yticks(np.arange(0, max(plt.yticks()[0])))
        if xlimits:
            plt.xlim(xlimits)

    def visualize_schedule_static(self, xlimits=None):
        zeiten = self.zeiten
        fig, ax = plt.subplots(1, figsize=(16, self.n_workers))
        pre = 0
        for i in range(self.n_workers):
            n_jobs_per_worker = self.n_jobs // self.n_workers
            if self.n_jobs % self.n_workers > i:
                n_jobs_per_worker += 1
            print()
            left_values = np.zeros(n_jobs_per_worker + 1)
            left_values[1:] = np.cumsum(self.zeiten["gesamt"][pre : pre + n_jobs_per_worker])
            ax.barh(
                y=i,
                width=zeiten["gesamt"][pre : pre + n_jobs_per_worker],
                left=left_values[:-1],
                linewidth=1,
                edgecolor="black",
            )
            pre = pre + n_jobs_per_worker
        plt.ylabel("Worker")
        plt.xlabel("Time")
        plt.title("Task schedule static")
        plt.grid(axis="x", alpha=0.5)
        plt.yticks(np.arange(0, max(plt.yticks()[0])))
        if xlimits:
            plt.xlim(xlimits)
