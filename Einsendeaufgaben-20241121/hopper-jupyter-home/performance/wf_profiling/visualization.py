import os
from pathlib import Path

import hvplot.polars

# pandas for white space parsing regex (pandas.read_csv)
import pandas
import polars as pl


# https://github.com/bokeh/bokeh/wiki/Migration-Guides#bokeh-object-ids
# Use unique html ids so jupyterlab can display multiple plots
os.environ["BOKEH_SIMPLE_IDS"] = "no"


# show more dataframe rows on print(df)
pl.Config.set_tbl_rows(100)


def main(
    input_file_path: Path, data_file: Path, number_of_execution_units, metric
) -> tuple:
    count = 0
    results_directory = input_file_path.parent / f"{input_file_path.stem}_results"
    while (
        plot_path := results_directory
        / f"profiling_{metric}_{number_of_execution_units}_plot{count}.html"
    ).exists():
        count += 1

    df, metric = create_dataframe_from_results(data_file)
    plot = create_plot(df, metric)
    try:
        hvplot.save(plot, plot_path, resources="cdn")
    except Exception as e:
        print(e)
    return plot_path, plot, df


def create_dataframe_from_results(data_file) -> tuple:
    metric = "_".join(str(data_file.stem).split("_")[2:-1])
    df = pandas.read_csv(
        data_file,
        on_bad_lines="warn",
        skiprows=7,
        header=None,
        sep=r"\s{2,}",
        engine="python",
    )
    df = pl.DataFrame(df)
    df = df.rename({"0": "Function"})
    df = df.melt(
        id_vars="Function",
        value_vars=df.columns[1:],
        value_name=metric,
        variable_name="Execution Unit",
    )
    df = df.filter(pl.col("Function").str.contains_any(["MEASUREMENT", ".exe"]).not_())
    return df, metric


def create_plot(df, metric):
    if df is None or df.is_empty():
        raise Exception("No profiling data found")
    df = df.sort(by=["Execution Unit", metric])
    old_metric_name = metric
    if metric == "time":
        metric += " (s)"
    elif "bytes" in metric:
        metric += " (bytes)"

    df = df.rename({old_metric_name: metric})
    plot = df.hvplot.barh(
        x="Function",
        y=metric,
        by="Execution Unit",
        stacked=False,
        responsive=True,
        height=500,
        grid=True,
    ).opts(
        multi_level=False,
        # fontscale=1.2,
        fontscale=1.4,
        legend_opts={
            "title": "Execution Unit",
            "title_text_font_size": "16px",
            "label_text_font_size": "16px",
        },
        # backend_opts={"plot.output_backend": "svg"},
    )
    return plot
