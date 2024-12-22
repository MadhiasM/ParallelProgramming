import shutil
from pathlib import Path

from parsl.data_provider.files import File

from ..shared_functions import converter, compilation, scorep

from . import visualization
from .. import parsl_config


def main(
    input_file_path,
    execution_model,
    number_of_execution_units,
    compiler_flags,
) -> tuple:
    parsl_config.load(input_file_path)
    converter.convert_input_file(input_file_path)
    compute(
        input_file_path,
        execution_model,
        number_of_execution_units,
        compiler_flags,
    )
    plot_path, plot, df = visualization.main(input_file_path, number_of_execution_units)
    return plot_path, plot, df


def compute(
    input_file_path,
    execution_model,
    number_of_execution_units,
    compiler_flags,
) -> None:
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
        enable_tracing=True,
        inputs=[parsl_file],
        outputs=[File(Path(parsl_file.filepath).with_suffix(".exe"))],
    )
    future_instrument.result()
    # with open(future.stderr, "r") as f:
    #     print(f.read(), file=sys.stderr)
    # with open(future.stdout, "r") as f:
    #     print(f.read())
