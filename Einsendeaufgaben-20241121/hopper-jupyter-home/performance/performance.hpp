/**
 * Integrating performance analysis into JupyterLab.
 */

#ifndef PERFORMANCE_HPP
#define PERFORMANCE_HPP

#define PYBIND11_DETAILED_ERROR_MESSAGES

#include "include/lazy_image.hpp" // for lazy_image
#include <filesystem>             // for current_path()
#include <iostream>               // for char_traits, basic_ostream, endl
#include <string>                 // for string, operator+, operator<<, oper...
#include <vector>                 // for vector

#pragma cling add_include_path("/opt/conda/include/python3.11")
#pragma cling load("libpython3")
#include <Python.h>
#include <pybind11/embed.h> // slow because it includes "pybind11.h"? https://pybind11.readthedocs.io/en/stable/advanced/embedding.html
#include <pybind11/stl.h> // https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html#automatic-conversion
#include <pybind11/stl/filesystem.h> // https://pybind11.readthedocs.io/en/stable/changelog.html#v2-7-0-jul-16-2021
namespace py = pybind11;

static std::string original_working_directory;

class performance {
private:
  std::filesystem::path input_file_path;
  std::string execution_model;

public:
  explicit performance(std::string input_file_path,
                       std::string execution_model = "") {
    if (Py_IsInitialized()) {
      // interpreter is already running, don't initialize again
    } else {
      original_working_directory = std::filesystem::current_path();
      py::initialize_interpreter();
      // Replace python stdout and stderr with c classes
      py::module_ sys = py::module_::import("sys");
      py::object python_stdout_redirect =
          py::module_::import("python_redirect").attr("PythonStdoutRedirect")();
      py::object python_stderr_redirect =
          py::module_::import("python_redirect").attr("PythonStderrRedirect")();
      sys.attr("stdout") = python_stdout_redirect;
      sys.attr("stderr") = python_stderr_redirect;
      // Patch parsl to fix this issue: https://github.com/Parsl/parsl/issues/3356
      py::module_::import("performance.parsl_config");
    }

    if (input_file_path.find("/") == 0) {
      this->input_file_path = input_file_path;
    } else {
      this->input_file_path =
          original_working_directory + "/" + input_file_path;
    }
    if (!std::filesystem::exists(this->input_file_path)) {
      std::cerr << "Input file does not exist: " << input_file_path
                << std::endl;
    }
    this->execution_model = execution_model;
  }

  void help() {
    py::module_ documentation =
        py::module_::import("performance.performance_documentation");
    documentation.attr("print_help")();
  }

  // -----------------
  //  Timing Analysis
  // -----------------
  display::lazy_image display_timing(std::vector<int> number_of_execution_units,
                                     std::vector<int> problem_size,
                                     int iterations_per_parameter_pair = 10,
                                     std::string compiler_flags = "-O3",
                                     bool compare = true) {
    std::filesystem::path plot_path;
    try {
      py::module_ mainmod = py::module_::import("performance.wf_timing.main");
      py::tuple tuple = mainmod.attr("main")(
          this->input_file_path, this->execution_model,
          number_of_execution_units, problem_size,
          iterations_per_parameter_pair, compiler_flags, compare);
      plot_path = tuple[0].cast<std::filesystem::path>();
    } catch (py::error_already_set &e) {
      std::cerr << e.what() << std::endl;
    }
    display::lazy_image im(plot_path);
    return im;
  }

  // --------------------
  //  Profiling Analysis
  // --------------------
  display::lazy_image display_profiling(int number_of_execution_units = 2,
                                        std::string metric = "time",
                                        std::string compiler_flags = "-O3") {
    std::filesystem::path plot_path;
    try {
      py::module_ mainmod =
          py::module_::import("performance.wf_profiling.main");
      py::tuple tuple = mainmod.attr("main")(
          this->input_file_path, this->execution_model,
          number_of_execution_units, metric, compiler_flags);
      plot_path = tuple[0].cast<std::filesystem::path>();
    } catch (py::error_already_set &e) {
      std::cerr << e.what() << std::endl;
    }

    display::lazy_image im(plot_path);
    return im;
  }

  void print_call_tree(int number_of_execution_units = 2,
                       std::string metric = "time",
                       std::string compiler_flags = "-O3") {
    try {
      py::module_ mainmod =
          py::module_::import("performance.wf_profiling.main");
      mainmod.attr("print_call_tree")(
          this->input_file_path, this->execution_model,
          number_of_execution_units, metric, compiler_flags);
    } catch (py::error_already_set &e) {
      std::cerr << e.what() << std::endl;
    }
  }

  // ------------------
  //  Tracing Analysis
  // ------------------
  display::lazy_image display_tracing(int number_of_execution_units = 4,
                                      std::string compiler_flags = "-O3") {
    std::filesystem::path plot_path;
    try {
      py::module_ mainmod = py::module_::import("performance.wf_tracing.main");
      py::tuple tuple =
          mainmod.attr("main")(this->input_file_path, this->execution_model,
                               number_of_execution_units, compiler_flags);
      plot_path = tuple[0].cast<std::filesystem::path>();
    } catch (py::error_already_set &e) {
      std::cerr << e.what() << std::endl;
    }
    display::lazy_image im(plot_path);
    return im;
  }
  // ------------------------
  //  Vectorization Analysis
  // ------------------------
  void print_compiler_optimization(std::string compiler_flags = "-O3") {
    try {
      py::module_ mainmod =
          py::module_::import("performance.wf_vectorization.main");
      mainmod.attr("main")(this->input_file_path, this->execution_model,
                           compiler_flags);
    } catch (py::error_already_set &e) {
      std::cerr << e.what() << std::endl;
    }
  }
};

// Custom classes to redirect Python stdout and stderr to C++
class PythonStdoutRedirect {
public:
  void write(const std::string &str) { std::cout << str; }

  void flush() { std::cout << std::flush; }
};

class PythonStderrRedirect {
public:
  void write(const std::string &str) { std::cerr << str; }

  void flush() { std::cerr << std::flush; }
};

PYBIND11_EMBEDDED_MODULE(python_redirect, m) {
  py::class_<PythonStdoutRedirect>(m, "PythonStdoutRedirect")
      .def(py::init<>())
      .def("write", &PythonStdoutRedirect::write)
      .def("flush", &PythonStdoutRedirect::flush);

  py::class_<PythonStderrRedirect>(m, "PythonStderrRedirect")
      .def(py::init<>())
      .def("write", &PythonStderrRedirect::write)
      .def("flush", &PythonStderrRedirect::flush);
}

#endif
