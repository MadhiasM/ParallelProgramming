import parsl
from parsl.app.app import bash_app, python_app


@python_app(cache=True)
def detect_execution_model(execution_model, inputs=()) -> tuple:
    if not execution_model:
        has_mpi = False
        has_openmp = False
        with open(inputs[0], "r") as input_file:
            for line in input_file:
                if "MPI_Init" in line:
                    has_mpi = True
                elif "#pragma omp" in line:
                    has_openmp = True

        if has_openmp and has_mpi:
            execution_model = "hybrid"
        elif has_openmp:
            execution_model = "openmp"
        elif has_mpi:
            execution_model = "mpi"
        else:
            execution_model = "serial"

    if execution_model == "mpi" or execution_model == "hybrid":
        compiler = "mpicxx"
    elif execution_model == "openmp" or execution_model == "serial":
        compiler = "g++"
    else:
        raise Exception("Unknown execution model.")

    return execution_model, compiler


@bash_app(cache=True)
def compile(
    execution_model,
    compiler,
    compiler_flags,
    inputs=(),
    outputs=(),
    stdout=parsl.AUTO_LOGNAME,
    stderr=parsl.AUTO_LOGNAME,
) -> str:
    if execution_model == "hybrid":
        return f"{compiler} -I .. {inputs[0]} -o {outputs[0]} -fopenmp -lm {compiler_flags} -std=c++17"
    elif execution_model == "openmp":
        return f"{compiler} -I .. {inputs[0]} -o {outputs[0]} -fopenmp -lm {compiler_flags} -std=c++17"
    elif execution_model == "mpi":
        return f"{compiler} -I .. {inputs[0]} -o {outputs[0]} -lm {compiler_flags} -std=c++17"
    elif execution_model == "serial":
        return f"{compiler} -I .. {inputs[0]} -o {outputs[0]} -lm {compiler_flags} -std=c++17"
    else:
        raise Exception("Unknown execution model.")
