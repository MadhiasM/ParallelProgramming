import sys
from parsl.data_provider.files import File
from pathlib import Path

from ..shared_functions import converter, compilation
from .. import parsl_config


def main(input_file_path, execution_model, compiler_flags) -> None:
    parsl_config.load(input_file_path)
    converter.convert_input_file(input_file_path)
    print_compiler_optimization(input_file_path, execution_model, compiler_flags)


def print_compiler_optimization(
    input_file_path, execution_model, compiler_flags
) -> None:
    source_file = Path(input_file_path.with_suffix(".cpp").name)
    future_detect = compilation.detect_execution_model(
        execution_model, inputs=[File(source_file)]
    )
    execution_model, compiler = future_detect.result()

    # Add flags for vectorization
    compiler_flags = f"{compiler_flags} -fopt-info-vec"
    # For clang compiler
    # compiler_flags = f"{compiler_flags} -Rpass-analysis=loop-vectorize -Rpass-missed=loop-vectorize"
    future_compile = compilation.compile(
        execution_model,
        compiler,
        compiler_flags,
        inputs=[File(source_file)],
        outputs=[File(source_file.with_suffix(".exe"))],
    )
    future_compile.result()
    with open(future_compile.stderr, "r") as f:
        print(f.read(), file=sys.stderr)
    with open(future_compile.stdout, "r") as f:
        print(f.read())
