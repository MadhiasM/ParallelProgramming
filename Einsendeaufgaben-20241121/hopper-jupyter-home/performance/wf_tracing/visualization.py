import os
from pathlib import Path


import plotly.express as px
import polars as pl

import otf2
import multiprocessing


# https://github.com/bokeh/bokeh/wiki/Migration-Guides#bokeh-object-ids
# Use unique html ids so jupyterlab can display multiple plots
os.environ["BOKEH_SIMPLE_IDS"] = "no"

# show more dataframe rows on print(df)
pl.Config.set_tbl_rows(100)


# convert to chrome json: https://github.com/hpcgroup/pipit/blob/v0.1.0/pipit/trace.py#L99
def main(input_file_path: Path, number_of_execution_units) -> tuple:
    count = 0
    results_directory = input_file_path.parent / f"{input_file_path.stem}_results"
    while (
        plot_path := results_directory
        / f"tracing_{number_of_execution_units}_plot{count}.html"
    ).exists():
        count += 1
    results_directory = input_file_path.parent / f"{input_file_path.stem}_results"

    df = create_dataframe_from_results(results_directory, plot_path)
    plot = create_plot(df)
    try:
        # https://plotly.com/python/configuration-options/#customizing-modebar-download-plot-button
        config = {
            "toImageButtonOptions": {
                "format": "svg",  # one of png, svg, jpeg, webp
                "filename": "newplot",
                "height": None,
                "width": None,
                "scale": 1,  # Multiply title/legend/axis/canvas sizes by this factor
            }
        }
        plot.write_html(plot_path, config=config)  # , include_plotlyjs=True)
    except Exception as e:
        print(e)

    return plot_path, plot, df


def create_dataframe_from_results(results_directory, output_file) -> pl.DataFrame:
    filepath = results_directory / "scorep_measurements" / "traces.otf2"
    if not filepath.exists():
        raise Exception("No tracing data found")
    filepath = str(filepath)
    # get number of independent trace parts, and metadata
    with otf2.reader.open(filepath) as trace:
        number_of_trace_locations = len(trace.definitions._locations)
        clock_properties = trace.definitions.clock_properties

    pool = multiprocessing.Pool()
    list_of_dicts = pool.starmap(
        read_otf2,
        [(filepath, trace_num) for trace_num in range(number_of_trace_locations)],
    )
    pool.close()
    dfs = []
    for i in list_of_dicts:
        dfs.append(pl.from_dicts(i))
    df = pl.concat(dfs).sort("Process", "Thread", "Event Start")
    # remove MPI_Init and int main
    df = df.filter(pl.col("Function").str.contains("MPI_Init|int main").not_())
    trace_start = df.select(pl.col("Event Start")).to_series().min()
    # shift and convert to nanoseconds
    df = df.with_columns(
        (pl.col("Event Start") - trace_start)
        * 10**9
        / clock_properties.timer_resolution,
        (pl.col("Event Stop") - trace_start)
        * 10**9
        / clock_properties.timer_resolution,
    )
    return df


def create_plot(df):
    if df is None or df.is_empty():
        raise Exception("No tracing data found")

    # plotly zoom breaks for small numbers:
    # move milliseconds to the seconds position in the timestamp by using "us" instead of "ns"
    df = df.with_columns(
        pl.col("Event Start").cast(pl.Datetime("us")),
        pl.col("Event Stop").cast(pl.Datetime("us")),
        (pl.col("Process").cast(pl.Utf8) + ":" + pl.col("Thread").cast(pl.Utf8)).alias(
            "Process:Thread"
        ),
    )

    plot = px.timeline(
        df,
        x_start="Event Start",
        x_end="Event Stop",
        y="Process:Thread",
        color="Function",
        template="simple_white",
        # the default color scale repeats after 10 colors
        color_discrete_sequence=[
            *px.colors.qualitative.D3,
            "#000000",
            "#4EB265",
            "#DC050C",
            "#F7CB45",
            "#BA8D84",
            "#777777",
            "#EE8026",
            "#994F88",
            "#F7F056",
            "#196580",
        ],
    )
    plot.update_layout(barmode="group", font_size=16)
    plot.update_yaxes(
        showgrid=True, autorange="reversed"
    )  # otherwise tasks are listed from the bottom up
    plot.update_xaxes(
        # https://d3js.org/d3-time-format
        tickformat="%s.%f",
        title_text="Time (ms)",
        showgrid=True,
    )
    return plot


def read_otf2(filepath, trace_num) -> dict[str, list]:
    dict_of_stacks: dict = {}
    with otf2.reader.open(filepath) as trace:
        timestamps_start, timestamps_stop, function_names, process_ids, thread_ids = (
            [],
            [],
            [],
            [],
            [],
        )
        trace_part = list(trace.definitions._locations)[trace_num]
        for location, event in trace.events(trace_part):
            # for each event get start and end timestamp into the same row
            # alternative to https://github.com/hpcgroup/pipit/blob/4c6ceeca90688155a14a657c68731e71ae4afbd4/pipit/trace.py#L91
            if isinstance(event, otf2.events.Enter):
                process_id = location.group._ref
                thread_id = location.name.split(" ")[-1]
                if not thread_id.isdigit():
                    thread_id = "0"
                identifier = (process_id, thread_id, event.region)
                stack = dict_of_stacks.get(identifier, [])
                stack.append(event)
                dict_of_stacks[identifier] = stack
            elif isinstance(event, otf2.events.Leave):
                process_id = location.group._ref
                thread_id = location.name.split(" ")[-1]
                if not thread_id.isdigit():
                    thread_id = "0"
                identifier = (process_id, thread_id, event.region)
                try:
                    event_enter = dict_of_stacks[identifier].pop()
                except KeyError as e:
                    raise KeyError("Found leave event but no previous enter event", e)
                process_ids.append(process_id)
                thread_ids.append(int(thread_id))
                function_names.append(event.region.name)
                timestamps_start.append(event_enter.time)
                timestamps_stop.append(event.time)

            else:
                continue
        return {
            "Process": process_ids,
            "Thread": thread_ids,
            "Function": function_names,
            "Event Start": timestamps_start,
            "Event Stop": timestamps_stop,
        }
