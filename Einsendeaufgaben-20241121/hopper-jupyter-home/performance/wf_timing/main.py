import itertools
import re
import sys
import textwrap
from pathlib import Path

import parsl
from parsl.app.app import bash_app, python_app
from parsl.data_provider.files import File

from tqdm.auto import tqdm

from ..shared_functions import compilation, converter

from . import visualization
from .. import parsl_config


def main(
    input_file_path,
    execution_model,
    number_of_execution_units,
    problem_size,
    iterations_per_parameter_pair,
    compiler_flags,
    compare,
) -> tuple:
    parsl_config.load(input_file_path)
    converter.convert_input_file(input_file_path)
    compute(
        input_file_path,
        execution_model,
        number_of_execution_units,
        problem_size,
        iterations_per_parameter_pair,
        compiler_flags,
    )
    plot_path, plot, plots_list, df = visualization.main(input_file_path, compare)
    return plot_path, plot, plots_list, df


def compute(
    input_file_path,
    execution_model,
    number_of_execution_units,
    problem_size,
    iterations_per_parameter_pair,
    compiler_flags,
) -> None:
    source_file = Path(input_file_path.with_suffix(".cpp").name)
    future_detect = compilation.detect_execution_model(
        execution_model, inputs=[File(source_file)]
    )
    execution_model, compiler = future_detect.result()

    future_replace_markers = replace_timing_markers_in_source_file(
        execution_model,
        inputs=[File(source_file)],
        outputs=[File(source_file.with_stem(source_file.stem + "_with_timing"))],
    )
    parsl_file = future_replace_markers.outputs[0]

    future_compile = compilation.compile(
        execution_model,
        compiler,
        compiler_flags,
        inputs=[parsl_file],
        outputs=[File(Path(parsl_file.filepath).with_suffix(".exe"))],
    )
    exec_file = future_compile.outputs[0]

    parameters = list(itertools.product(problem_size, number_of_execution_units))
    logged_single_execution = False
    progress_bar = tqdm(parameters, write_bytes=True, file=sys.stdout)
    for prob_size, num_exec in progress_bar:
        progress_bar.set_description(f"Parameter sets (Current set: {prob_size=}, {num_exec=})")
        for i in range(1, iterations_per_parameter_pair + 1):
            results_file = Path(f"timing_{num_exec}_{prob_size}.csv")

            if logged_single_execution:
                stdout = None
                stderr = None
            else:
                stdout = parsl.AUTO_LOGNAME
                stderr = parsl.AUTO_LOGNAME
                logged_single_execution = True

            future_execute = execute(
                execution_model,
                num_exec,
                prob_size,
                results_file,
                i,
                inputs=[exec_file],
                stdout=stdout,
                stderr=stderr,
            )
            future_execute.result()


@python_app
def replace_timing_markers_in_source_file(
    execution_model,
    inputs=(),
    outputs=(),
) -> tuple:
    """
    Make the following changes to the source file:
    1. Replace each marker with a call to omp_get_wtime, MPI_Wtime, or clock
       functions depending on the execution model.
    2. Add code "int n = {new_problem_size}" to specify the size of the problem.
    3. Add code to store the measurement results in a text file.
    """

    start_marker_count = {}

    lines = []
    found_mpi_header = False
    found_omp_header = False
    found_time_header = False

    with open(inputs[0], "r") as source_file:
        if "// MODIFICATION TIMING DONE" in source_file.read():
            return outputs
        source_file.seek(0)
        for line in source_file:
            # Ignore lines
            if "// start_main" in line or "// end_main" in line:
                lines.append(line)
                continue
            # Check for headers
            if "#include" in line:
                if "#include <mpi.h>" in line:
                    found_mpi_header = True
                elif "#include <omp.h>" in line:
                    found_omp_header = True
                elif "#include <time.h>" in line:
                    found_time_header = True
                lines.append(line)

            # Check for timing markers
            elif "// start_" in line:
                marker = line.replace("// start_", "").strip()
                lines.append(line)

                if marker in start_marker_count:
                    start_marker_count[marker] += 1
                else:
                    start_marker_count[marker] = 0

                count = start_marker_count[marker]

                if execution_model == "serial":
                    timing_start = "clock_gettime(CLOCK_MONOTONIC, &start);"
                elif execution_model == "openmp":
                    timing_start = f"t_start_{marker}_{count} = omp_get_wtime();"
                else:
                    timing_start = f"t_start_{marker}_{count} = MPI_Wtime();"

                newline = f"""\
                    // >>> timing start
                    {timing_start}
                    // <<< timing start
                """
                newline = textwrap.dedent(newline)
                lines.append(newline)
            elif "// end_" in line:
                marker = line.replace("// end_", "").strip()

                if marker in start_marker_count:
                    count = start_marker_count[marker]
                else:
                    raise Exception(
                        f"Marker Error: End {marker=} was found before start marker."
                    )

                if execution_model == "serial":
                    timing_end = f"""\
                        // >>> timing end
                        clock_gettime(CLOCK_MONOTONIC, &end);
                        t_elapsed_{marker}_{count} = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1e9);
                        // <<< timing end
                    """
                elif execution_model == "openmp":
                    timing_end = f"""\
                        // >>> timing end
                        t_elapsed_{marker}_{count} = omp_get_wtime() - t_start_{marker}_{count};
                        // <<< timing end
                    """
                else:
                    timing_end = f"""\
                        // >>> timing end
                        t_elapsed_{marker}_{count} = MPI_Wtime() - t_start_{marker}_{count};
                        // <<< timing end
                    """

                newline = textwrap.dedent(timing_end)
                lines.append(newline)
                lines.append(line)
            # Replace initialization of integer 'n'
            elif re.match(r"^\s*(unsigned\s+)?(const\s+)?\s*int\s+n\s*=\s*", line):
                newline = """\
                    // >>> problem size
                    int n = atoi(argv[1]);
                    // <<< problem size
                """
                newline = textwrap.dedent(newline)
                lines.append(newline)
            # Edit 'int main' function
            elif "int main(" in line:
                lines.append(line)
                newline = """\
                    // >>> main timing
                    FILE *out_file = fopen(argv[3], "a");
                    if (out_file == NULL) {{
                      printf("Error: Can't open file %s\\n", argv[3]);
                      return -1;
                    }}
                    unsigned long int iteration = atoi(argv[4]);
                    // <<< main timing
                """
                newline = textwrap.dedent(newline)
                lines.append(newline)
            elif "MPI_Init(" in line:
                newline = """\
                    // >>> timing rank
                    int processorRank;
                    MPI_Comm_rank(MPI_COMM_WORLD, &processorRank);
                    // <<< timing rank
                """
                newline = textwrap.dedent(newline)
                lines.append(line)
                lines.append(newline)
            else:
                lines.append(line)

    with open(outputs[0], "w") as f:
        # Add missing headers
        if not found_mpi_header and (
            execution_model == "mpi" or execution_model == "hybrid"
        ):
            f.write("#include <mpi.h>\n")
        elif not found_omp_header and execution_model == "openmp":
            f.write("#include <omp.h>\n")
        elif not found_time_header:
            f.write("#include <time.h>\n")

        # Write file
        for li in lines:
            if "unsigned long int iteration" in li:
                f.write(li)
                f.write("// >>> timing variables\n")
                for marker, count in start_marker_count.items():
                    f.write(
                        f"double t_start_{marker}_{count};\n"
                        f"double t_elapsed_{marker}_{count};\n"
                    )
                if execution_model == "serial":
                    f.write("struct timespec start, end;\n")
                f.write("// <<< timing variables\n")

            elif "// end_main" in li:
                if execution_model == "serial":
                    firstline = ""
                    lastline = ""
                elif execution_model == "openmp":
                    firstline = "if(omp_get_thread_num() == 0){\n"
                    lastline = "}\n"
                elif execution_model == "mpi" or execution_model == "hybrid":
                    firstline = "if(processorRank == 0){\n"
                    lastline = "}\n"
                f.write("// >>> timing output\n")
                f.write(firstline)
                for marker, count in start_marker_count.items():
                    newline = f"""\
                        fprintf(out_file, "{marker}_{count},%d,%.9f\\n", iteration, t_elapsed_{marker}_{count});
                    """
                    newline = textwrap.dedent(newline)
                    f.write(newline)
                f.write(lastline)
                f.write("// <<< timing output\n")
                f.write(li)
            else:
                f.write(li)
        f.write("// MODIFICATION TIMING DONE\n")
    return outputs


@bash_app
def execute(
    execution_model,
    number_of_execution_units,
    problem_size,
    results_file,
    iteration,
    inputs=(),
    stdout=parsl.AUTO_LOGNAME,
    stderr=parsl.AUTO_LOGNAME,
) -> str:
    if execution_model == "hybrid":
        number_of_threads = 2
        number_of_processes = number_of_execution_units
        # suppress mpi load errors https://github.com/open-mpi/ompi/issues/7752
        return f"OMPI_MCA_mca_base_component_show_load_errors=0 OMP_NUM_THREADS={number_of_threads} mpirun -np {number_of_processes} ./{inputs[0]} {problem_size} {number_of_processes} {results_file} {iteration}"
    elif execution_model == "openmp":
        return f"OMP_NUM_THREADS={number_of_execution_units} ./{inputs[0]} {problem_size} {number_of_execution_units} {results_file} {iteration}"
    elif execution_model == "mpi":
        return f"OMPI_MCA_mca_base_component_show_load_errors=0 mpirun -np {number_of_execution_units} ./{inputs[0]} {problem_size} {number_of_execution_units} {results_file} {iteration}"
    elif execution_model == "serial":
        return f"./{inputs[0]} {problem_size} {number_of_execution_units} {results_file} {iteration}"
    else:
        raise Exception("Unknown execution model.")
