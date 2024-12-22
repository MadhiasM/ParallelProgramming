import parsl
from parsl.app.app import bash_app, python_app


@python_app(cache=True)
def replace_scorep_markers_in_source_file(inputs=(), outputs=()) -> tuple:
    """
    Modify the source file:
    - Add scorep include
    - Replace // continue_recording and // pause_recording with scorep calls
    """
    lines = []
    added_scorep_include = False
    with open(inputs[0], "r") as source_file:
        if "// MODIFICATION SCOREP DONE" in source_file.read():
            return
        source_file.seek(0)
        for line in source_file:
            if "// continue_recording" in line:
                if not added_scorep_include:
                    lines.insert(0, "// >>> scorep\n")
                    lines.insert(1, '#include "scorep/SCOREP_User.h"\n')
                    lines.insert(2, "// <<< scorep\n")
                    added_scorep_include = True
                lines.append(line)
                lines.append("// >>> scorep\n")
                lines.append("SCOREP_RECORDING_ON();\n")
                lines.append("// <<< scorep\n")
            elif "// pause_recording" in line:
                if not added_scorep_include:
                    lines.insert(0, "// >>> scorep\n")
                    lines.insert(1, '#include "scorep/SCOREP_User.h"\n')
                    lines.insert(2, "// <<< scorep\n")
                    added_scorep_include = True
                lines.append(line)
                lines.append("// >>> scorep\n")
                lines.append("SCOREP_RECORDING_OFF();\n")
                lines.append("// <<< scorep\n")
            else:
                lines.append(line)

    with open(outputs[0], "w") as f:
        for li in lines:
            f.write(li)
        f.write("// MODIFICATION SCOREP DONE\n")
    return outputs


@bash_app
def instrument_with_scorep(
    execution_model,
    compiler,
    compiler_flags,
    number_of_execution_units,
    enable_tracing=False,
    inputs=(),
    outputs=(),
    stdout=parsl.AUTO_LOGNAME,
    stderr=parsl.AUTO_LOGNAME,
) -> str:
    if enable_tracing:
        trace = "-t"
    else:
        trace = ""

    if execution_model == "hybrid":
        # TODO: Fix number of processes and number of threads
        return f"""
SCOREP_WRAPPER_INSTRUMENTER_FLAGS=--user scorep-{compiler} -I .. {inputs[0]} -o {outputs[0]} -fopenmp {compiler_flags} -std=c++17
OMPI_MCA_mca_base_component_show_load_errors=0 OMP_NUM_THREADS={number_of_execution_units} SCOREP_TOTAL_MEMORY=4000M scalasca -analyze {trace} -e scorep_measurements mpirun -np {number_of_execution_units} ./{outputs[0]}
"""
    elif execution_model == "openmp":
        return f"""
SCOREP_WRAPPER_INSTRUMENTER_FLAGS=--user scorep-{compiler} -I .. {inputs[0]} -o {outputs[0]} -fopenmp {compiler_flags} -std=c++17
OMP_NUM_THREADS={number_of_execution_units} SCOREP_TOTAL_MEMORY=4000M scalasca -analyze {trace} -e scorep_measurements ./{outputs[0]}
"""
    elif execution_model == "mpi":
        return f"""
SCOREP_WRAPPER_INSTRUMENTER_FLAGS=--user scorep-{compiler} -I .. {inputs[0]} -o {outputs[0]} {compiler_flags} -std=c++17
OMPI_MCA_mca_base_component_show_load_errors=0 SCOREP_TOTAL_MEMORY=4000M scalasca -analyze {trace} -e scorep_measurements mpirun -np {number_of_execution_units} ./{outputs[0]}
"""
    elif execution_model == "serial":
        return f"""
SCOREP_WRAPPER_INSTRUMENTER_FLAGS=--user scorep-{compiler} -I .. {inputs[0]} -o {outputs[0]} {compiler_flags} -std=c++17
OMP_NUM_THREADS=1 SCOREP_TOTAL_MEMORY=4000M scalasca -analyze {trace} -e scorep_measurements ./{outputs[0]}
"""
    else:
        raise Exception("Unknown execution model.")
