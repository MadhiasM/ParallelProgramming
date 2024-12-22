import shutil
from pathlib import Path

import parsl
from parsl.app.app import bash_app
from parsl.data_provider.files import File

from ..shared_functions import compilation, converter, scorep

from . import visualization
from .. import parsl_config


def main(
    input_file_path,
    execution_model,
    number_of_execution_units,
    metric,
    compiler_flags,
    printcalltree=False,
) -> tuple:
    parsl_config.load(input_file_path)
    converter.convert_input_file(input_file_path)
    data_file = compute(
        input_file_path,
        execution_model,
        number_of_execution_units,
        metric,
        compiler_flags,
        printcalltree,
    )

    plot_path, plot, df = visualization.main(
        input_file_path,
        data_file,
        number_of_execution_units,
        metric,
    )

    return plot_path, plot, df


def print_call_tree(
    input_file_path, execution_model, number_of_execution_units, metric, compiler_flags
) -> None:
    parsl_config.load(input_file_path)
    converter.convert_input_file(input_file_path)
    compute(
        input_file_path,
        execution_model,
        number_of_execution_units,
        metric,
        compiler_flags,
        printcalltree=True,
    )


def compute(
    input_file_path,
    execution_model,
    number_of_execution_units,
    metric,
    compiler_flags,
    printcalltree,
) -> Path:
    if Path("scorep_measurements").exists():
        shutil.rmtree(Path("scorep_measurements"))
    source_file = Path(input_file_path.with_suffix(".cpp").name)
    future_detect = compilation.detect_execution_model(
        execution_model, inputs=[File(source_file)]
    )
    execution_model, compiler = future_detect.result()

    future_replace_markers = scorep.replace_scorep_markers_in_source_file(
        inputs=[File(source_file)],
        # use short file name for readable plot
        # outputs=[File(source_file.with_stem(source_file.stem + "_with_scorep"))]
        outputs=[File(source_file.with_stem("p"))],
    )
    parsl_file = future_replace_markers.outputs[0]

    future_instrument = scorep.instrument_with_scorep(
        execution_model,
        compiler,
        compiler_flags,
        number_of_execution_units,
        inputs=[parsl_file],
        outputs=[File(Path(parsl_file.filepath).with_suffix(".exe"))],
    )
    future_instrument.result()

    # Extract the collected data from the .cubex file and save it in a text file
    parsl_input_file = File(Path("scorep_measurements") / "profile.cubex")
    future = run_cube_dump(
        metric,
        inputs=[parsl_input_file],
        outputs=[
            File(Path(f"profiling_metric_{metric}_{number_of_execution_units}.txt"))
        ],
    )
    future.result()
    dump_txt_file = Path(future.outputs[0].filepath)

    if printcalltree:
        future = run_cube_calltree(
            metric,
            inputs=[parsl_input_file],
            outputs=[File("call_tree.txt")],
        )
        future.result()
        call_tree_file = future.outputs[0].filepath
        with open(call_tree_file, "r") as f:
            print(f.read())

    return dump_txt_file


@bash_app
def run_cube_dump(
    metric,
    inputs=(),
    outputs=(),
    stdout=parsl.AUTO_LOGNAME,
    stderr=parsl.AUTO_LOGNAME,
) -> str:
    return f"cube_dump -m {metric} -c all {inputs[0]} | c++filt > {outputs[0]}"


@bash_app
def run_cube_calltree(
    metric,
    inputs=(),
    outputs=(),
    stdout=parsl.AUTO_LOGNAME,
    stderr=parsl.AUTO_LOGNAME,
) -> str:
    return f"cube_calltree -i -m {metric} -p {inputs[0]} | c++filt > {outputs[0]}"
