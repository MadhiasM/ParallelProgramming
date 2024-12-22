info_string = """
This tool integrates performance analysis into Jupyter notebooks.

Given a Jupyter Notebook or a C/C++ file containing a cell with a piece of code enclosed between
"// start_main" and "// end_main", it provides the following functionality

1. Convert the notebook into a C++ file.
2. Compile and execute the input file.
3. Perform timing, profiling or tracing analysis.
4. Visualize results.

-----
Usage
-----
To use the tool, the user must create an object of the class.
An object is created by calling the constructor. This can be done as follows.

performance(std::string input_file_path,
            std::string execution_model = "")


input_file_path supports the following extensions: .c, .cpp, or .ipynb.
The available execution models are:
- "mpi"
- "openmp"
- "hybrid" (number_of_threads is 2)
- "serial"

---------------
Timing Analysis
---------------
display::lazy_image display_timing(std::vector<int> number_of_execution_units,
                                   std::vector<int> problem_size,
                                   int iterations_per_parameter_pair = 10,
                                   std::string compiler_flags = "-O3",
                                   bool compare = false) {

Perform a timing analysis on a piece of code enclosed by timing markers.
Timing markers must start with "// start_" or "// end_" and identical markers can't be nested.
Examples:
- "// start_timing" and "// end_timing"
- "// start_init" and "// end_init"
- "// start_compute" and "// end_compute"
- "// start_check" and "// end_check"
- "// start_cleanup" and "// end_cleanup"

------------------
Profiling Analysis
------------------
display::lazy_image display_profiling(int number_of_execution_units = 2,
                                      std::string metric = "time",
                                      std::string compiler_flags = "-O3")

void print_call_tree(int number_of_execution_units = 2,
                     std::string metric = "time",
                     std::string compiler_flags = "-O3") {

Available metrics:
- "time": Display the amount of time that each process/thread has spent in each function.
- "visits": Display the number of times each process/thread has visited each function in the source file.
- "min_time": Display the minimal amount of time that each process/thread has spent in each function.
- "max_time": Display the maximal amount of time that each process/thread has spent in each function.
- "bytes_put": Display the number of bytes transferred by each execution unit using the MPI_Put RMA operation.
- "bytes_get": Display the number of bytes transferred by each execution unit using the MPI_Get RMA operation.
- "io_bytes_read": Display the number of IO bytes read by each execution unit.
- "io_bytes_written": Display the number of IO bytes written by each execution unit.

Markers:
- "// pause_recording"
- "// continue_recording"

----------------
Tracing Analysis
----------------
display::lazy_image display_tracing(int number_of_execution_units = 4,
                                    std::string compiler_flags = "-O3") {

Markers:
- "// pause_recording"
- "// continue_recording"

----------------------------------
Vectorization Analysis
----------------------------------
void print_compiler_optimization(std::string compiler_flags = "-O3")
"""


def print_help() -> None:
    print(info_string)
