import os
from pathlib import Path

import holoviews as hv
import hvplot.polars
import polars as pl


# https://github.com/bokeh/bokeh/wiki/Migration-Guides#bokeh-object-ids
# Use unique html ids so jupyterlab can display multiple plots
os.environ["BOKEH_SIMPLE_IDS"] = "no"

# show more dataframe rows on print(df)
pl.Config.set_tbl_rows(100)


def main(input_file_path: Path, compare) -> tuple:
    count = 0
    results_directory = input_file_path.parent / f"{input_file_path.stem}_results"
    while (plot_path := results_directory / f"timing_plot{count}.html").exists():
        count += 1

    df = create_dataframe_from_results(results_directory)
    df = compute_results_on_dataframe(df)

    plot, plots_list = create_plot(df, compare)
    try:
        hvplot.save(plot, plot_path, resources="cdn")
    except Exception as e:
        print(e)
    return plot_path, plot, plots_list, df


def compute_results_on_dataframe(df) -> pl.DataFrame:
    if df is None or df.is_empty():
        raise Exception("No timing data found")

    # Calculate mean and median
    df = df.with_columns(
        pl.col("Time (s)")
        .mean()
        .over(["Marker", "Problem Size", "Number of Execution Units"])
        .name.prefix("Mean "),
        pl.col("Time (s)")
        .median()
        .over(["Marker", "Problem Size", "Number of Execution Units"])
        .name.prefix("Median "),
    )

    # Calculate speedup if possible: serial / parallel
    if not df.filter(pl.col("Number of Execution Units") == 1).is_empty():
        df = df.with_columns(
            (
                pl.col("Time (s)")
                .filter(pl.col("Number of Execution Units") == 1)
                .first()
                / pl.col("Time (s)")
            )
            .over(["Marker", "Problem Size", "Iteration"])
            .alias("Speedup"),
        )
        df = df.with_columns(
            pl.col("Speedup")
            .mean()
            .over(["Marker", "Problem Size", "Number of Execution Units"])
            .name.prefix("Mean "),
        )
    return df


def create_dataframe_from_results(results_directory) -> pl.DataFrame:
    df = pl.DataFrame()

    # Read data from result files
    for file_path in results_directory.glob("*.csv"):
        if file_path.name.startswith("timing_"):
            name_snippets = file_path.stem.split("_")
            number_of_execution_units = name_snippets[1]
            problem_size = name_snippets[2]
            df_current = pl.read_csv(
                file_path,
                separator=",",
                has_header=False,
                new_columns=["Marker", "Iteration", "Time (s)"],
            )

            df_current = df_current.with_columns(
                # Overwrite "Marker" column
                # ("Timing " + pl.col("Marker").str.strip_chars(":")).alias("Marker"),
                pl.col("Marker").str.strip_chars(":").alias("Marker"),
                # Create two columns from literals
                pl.lit(int(problem_size)).alias("Problem Size"),
                pl.lit(int(number_of_execution_units)).alias(
                    "Number of Execution Units"
                ),
            )
            df = pl.concat([df, df_current])

    return df


def create_plot(df, compare=False) -> tuple:
    if df is None or df.is_empty():
        print("Error: No timing data found")
        return None, None

    plots_list = []
    if compare:
        df = df.sort(by="Number of Execution Units")
        df_dict = df.partition_by(["Number of Execution Units"], as_dict=True)
        for num_exec, df in df_dict.items():
            timing_plot = plot_time_compare(df, "Mean Time (s)")
            timing_plot = timing_plot.opts(
                title=f"Number of Execution Units: {num_exec[0]}",
            )
            plots_list.append(timing_plot)

    else:
        df_dict = df.partition_by(["Marker"], as_dict=True)
        for marker, df in df_dict.items():
            timing_plot = plot_time_or_speedup(df, "Time (s)")
            timing_plot = timing_plot.opts(title=marker[0])
            if "Speedup" in df.columns:
                speedup_plot = plot_time_or_speedup(df, "Speedup")
                plots_for_marker = (timing_plot + speedup_plot).cols(1)
            else:
                plots_for_marker = timing_plot
            plots_list.append(plots_for_marker)

    complete_plot = hv.Layout(plots_list).cols(1).opts(shared_axes=False)
    return complete_plot, plots_list


def plot_time_compare(df, y):
    args = {
        "x": "Problem Size",
        "y": "Mean Time (s)",
        # by selects the parameter in the legend
        "by": "Marker",
        "legend": "top_left",
        # groupby selects the parameter that is selectable in extra drop down widget
        # "groupby": "Number of Execution Units",
        "fontscale": 1.2,
        "responsive": True,
        "height": 300,
        "grid": True,
    }
    df = df.filter(pl.col("Iteration") == 1)
    df = df.sort(by="Problem Size")
    plot = df.hvplot.bar(**args).opts(
        multi_level=False,
        legend_opts={"title": "Marker", "background_fill_alpha": 0.5},
        # Error for combined plots: Failed to execute 'drawImage' on 'CanvasRenderingContext2D'
        # backend_opts={"plot.output_backend": "svg"},
    )
    return plot


def plot_time_or_speedup(df, y):
    args = {
        "x": "Problem Size",
        # by selects the parameter in the legend
        "by": "Number of Execution Units",
        # groupby selects the parameter that is selectable in extra drop down widget
        # "groupby": "Marker",
        "fontscale": 1.2,
        "responsive": True,
        "height": 300,
        "grid": True,
        "yformatter": "%.3g",
    }
    df = df.sort(by="Problem Size")
    # filter first because bokeh prints duplicate lines
    df_filtered = df.filter(pl.col("Iteration") == 1)
    line = df_filtered.hvplot.line(
        **args, y="Mean " + y, line_dash="dashdot", line_alpha=0.8
    )
    # additional step to create markers until hvplot.line() supports markers
    markers = df.hvplot.scatter(**args, y=y)
    # merge both plots (overlay)
    plot = markers * line
    plot = plot.opts(
        legend_opts={"title": "Number of\nExecution Units"},
    )
    return plot
