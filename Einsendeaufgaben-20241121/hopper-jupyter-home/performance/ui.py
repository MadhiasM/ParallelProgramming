import os
import sys
import time
from pathlib import Path

import panel as pn

import ipylab
import nbformat
from nbformat.sign import NotebookNotary

pn.extension(
    "codeeditor",
    "terminal",
    design="material",
    sizing_mode="stretch_width",
    embed=False,
)
app = ipylab.JupyterFrontEnd()  # interact with jupyterlab
main = pn.WidgetBox("# Performance Tool")
SESSION = os.environ["SESSION"]
# extract input_file path from notebook name
input_file_from_filename = (
    Path(SESSION).stem.replace("ui_", "").replace("|", "/") + ".ipynb"
)
input_file = os.environ["PWD"] + "/" + input_file_from_filename
input_file_path = Path(input_file)
results_directory = input_file_path.parent / f"{input_file_path.stem}_results"

if "-without-inputfile" in SESSION or "src/ui.ipynb" in SESSION:
    input_file = os.environ["PWD"]


def cpp_timing(
    input_file, exec_units, problem_size, iterations, compiler_flags, compare
) -> str:
    return f"""#include <performance.hpp>
performance p("{input_file}");
display::lazy_image im;
im = p.display_timing({{{exec_units}}}, {{{problem_size}}}, {iterations}, "{compiler_flags}", {str(compare).lower()});
im
"""


def cpp_profiling(
    input_file, exec_units, compiler_flags, display_call_tree, metric
) -> str:
    if display_call_tree:
        call_tree = f'p.print_call_tree({exec_units}, "{metric}");'
    else:
        call_tree = ""
    return f"""#include <performance.hpp>
performance p("{input_file}");
display::lazy_image im;
{call_tree}
im = p.display_profiling({exec_units}, "{metric}", "{compiler_flags}");
im
"""


def cpp_tracing(input_file, exec_units, compiler_flags) -> str:
    return f"""#include <performance.hpp>
performance p("{input_file}");
display::lazy_image im;
im = p.display_tracing({exec_units}, "{compiler_flags}");
im
"""


def cpp_compiler(input_file, compiler_flags) -> str:
    return f"""#include <performance.hpp>
performance p("{input_file}");
p.print_compiler_optimization("{compiler_flags}");
"""


def py_timing(
    input_file, exec_units, problem_size, iterations, compiler_flags, compare
) -> str:
    return f"""import performance.wf_timing.main
from pathlib import Path
plot_path, plot, plots_list, df = performance.wf_timing.main.main(Path("{input_file}"), "", [{exec_units}], [{problem_size}], {iterations}, "{compiler_flags}", {compare})
"""


def py_profiling(
    input_file, exec_units, compiler_flags, display_call_tree, metric
) -> str:
    return f"""import performance.wf_profiling.main
from pathlib import Path
plot_path, plot, df = performance.wf_profiling.main.main(Path("{input_file}"), "", {exec_units}, "{metric}", "{compiler_flags}", {display_call_tree})
"""


def py_tracing(input_file, exec_units, compiler_flags) -> str:
    return f"""import performance.wf_tracing.main
from pathlib import Path
plot_path, plot, df = performance.wf_tracing.main.main(Path("{input_file}"), "", {exec_units}, "{compiler_flags}")
"""


def py_compiler(input_file, compiler_flags) -> str:
    return f"""import performance.wf_vectorization.main
from pathlib import Path
performance.wf_vectorization.main.main(Path("{input_file}"), "", "{compiler_flags}")
"""


# GUI ########################################################################
def create_widgets(analysis, cpp, py, user_inputs) -> pn.Row:
    editor_cpp = pn.widgets.CodeEditor(
        value=cpp,
        language="c_cpp",
        name="cpp",
    )
    editor_py = pn.widgets.CodeEditor(
        value=py,
        language="python",
        name="py",
    )
    editors = pn.Tabs(
        ("C++ API", editor_cpp), ("Python API", editor_py), name=f"{analysis}_editors"
    )
    # save API setting
    pn.bind(on_api_change, editors.param.active, watch=True)
    # show warning if input_file is missing
    alert_pane = pn.pane.Alert(
        "**Warning:** Input file is not a file.", alert_type="warning"
    )
    pn.bind(on_input_file_change, user_inputs[0], alert_pane, watch=True)
    on_input_file_change(user_inputs[0].value, alert_pane)
    return pn.Row(
        pn.Column(alert_pane, *user_inputs),
        editors,
    )


def gui_timing() -> pn.Row:
    user_inputs = (
        pn.widgets.TextInput(name="Input File", value=input_file),
        pn.widgets.TextInput(
            name="Number of Execution Units (Threads or Processes)", value="1,2,4,8"
        ),
        pn.widgets.TextInput(name="Problem Sizes", value="32,64"),
        pn.widgets.TextInput(name="Number of Iterations", value="10"),
        pn.widgets.TextInput(name="Compiler Flags", value="-O3"),
        pn.widgets.Checkbox(name="Compare Timing Regions", value=True),
    )
    cpp = pn.bind(cpp_timing, *user_inputs, watch=True)
    py = pn.bind(py_timing, *user_inputs, watch=True)
    return create_widgets("timing", cpp, py, user_inputs)


def gui_profiling() -> pn.Row:
    user_inputs = (
        pn.widgets.TextInput(name="Input File", value=input_file),
        pn.widgets.TextInput(
            name="Number of Execution Units (Threads or Processes)", value="4"
        ),
        pn.widgets.TextInput(name="Compiler Flags", value="-O3"),
        pn.widgets.Checkbox(name="Display Call Tree"),
        pn.widgets.RadioBoxGroup(
            name="RadioBoxGroup",
            options=[
                "time",
                "visits",
                "min_time",
                "max_time",
                "bytes_put",
                "bytes_get",
                "io_bytes_read",
                "io_bytes_written",
            ],
            # inline=True,
        ),
    )
    cpp = pn.bind(cpp_profiling, *user_inputs, watch=True)
    py = pn.bind(py_profiling, *user_inputs, watch=True)
    return create_widgets("profiling", cpp, py, user_inputs)


def gui_tracing() -> pn.Row:
    user_inputs = (
        pn.widgets.TextInput(name="Input File", value=input_file),
        pn.widgets.TextInput(
            name="Number of Execution Units (Threads or Processes)", value="4"
        ),
        pn.widgets.TextInput(name="Compiler Flags", value="-O3"),
    )
    cpp = pn.bind(cpp_tracing, *user_inputs, watch=True)
    py = pn.bind(py_tracing, *user_inputs, watch=True)
    return create_widgets("tracing", cpp, py, user_inputs)


def gui_compiler() -> pn.Row:
    user_inputs = (
        pn.widgets.TextInput(name="Input File", value=input_file),
        pn.widgets.TextInput(name="Compiler Flags", value="-O3"),
    )
    cpp = pn.bind(cpp_compiler, *user_inputs, watch=True)
    py = pn.bind(py_compiler, *user_inputs, watch=True)
    return create_widgets("vectorization", cpp, py, user_inputs)


def on_api_change(active_tab) -> None:
    """Synchronize current API across all analysis tabs."""
    for name, obj in widgets_by_name.items():
        if "_editors" in name:
            obj.active = active_tab


def on_input_file_change(new_input_file, alert_pane) -> None:
    """Synchronize input file across all analysis tabs."""
    global input_file
    if Path(new_input_file).is_file():
        alert_pane.visible = False
        input_file = new_input_file
    else:
        alert_pane.visible = True
    if "widgets_by_name" in globals():
        for name, obj in widgets_by_name.items():
            if "Input File" in name:
                obj.value = new_input_file


# def on_active_tab_change(active_tab):
#    print(f"The active tab has changed to: {active_tab}")
# pn.bind(on_active_tab_change, tabs.param.active, watch=True)

# Add widgets to main GUI ########################################################################
tabs = pn.Tabs(
    ("Timing", gui_timing()),
    ("Profiling", gui_profiling()),
    ("Tracing", gui_tracing()),
    ("Compiler Optimization", gui_compiler()),
)
main.append(tabs)

# Refer to widgets by name
# TODO: find a different way to access widgets
duplicate_names_counter = 0


def walk_tree(obj, widgets_by_name) -> None:
    global duplicate_names_counter
    if obj.name in widgets_by_name.keys():
        key = obj.name + str(duplicate_names_counter)
        duplicate_names_counter += 1
    else:
        key = obj.name
    widgets_by_name[key] = obj
    if hasattr(obj, "objects"):
        for o in obj.objects:
            walk_tree(o, widgets_by_name)


widgets_by_name: dict = {}
walk_tree(tabs, widgets_by_name)


def get_code_from_active_tab() -> tuple:
    if tabs.active == 0:
        analysis = "timing"
    elif tabs.active == 1:
        analysis = "profiling"
    elif tabs.active == 2:
        analysis = "tracing"
    elif tabs.active == 3:
        analysis = "vectorization"
    editor_tabs = widgets_by_name[f"{analysis}_editors"]
    index = editor_tabs.active
    code = editor_tabs[index].value
    if index == 0:
        return code, "cpp", analysis
    else:
        return code, "python", analysis


def run_cpp_code(input_file, code, analysis) -> None:
    # https://nbconvert.readthedocs.io/en/latest/execute_api.html#executing-notebooks-using-the-python-api-interface
    nb = nbformat.v4.new_notebook()
    if Path(input_file).suffix == ".ipynb":
        with open(input_file) as f:
            metadata = nbformat.read(f, as_version=4)["metadata"]
            nb["metadata"] = metadata
    else:
        nb["metadata"] = {
            "kernelspec": {
                "display_name": "C++17 (-O3)",
                "language": "cpp",
                "name": "xcpp17",
            },
            "language_info": {
                "codemirror_mode": "text/x-c++src",
                "file_extension": ".cpp",
                "mimetype": "text/x-c++src",
                "name": "c++",
                "version": "17",
            },
        }

    new_cell = nbformat.v4.new_code_cell(source=code)
    nb["cells"].append(new_cell)

    # SAVE NOTEBOOK
    results_directory.mkdir(parents=True, exist_ok=True)
    count = 0
    while (output_file := results_directory / f"nb_{analysis}_{count}.ipynb").exists():
        count += 1

    with open(output_file, "w", encoding="utf-8") as f:
        nbformat.write(nb, f)
    NotebookNotary().sign(nb)
    # ipylab only supports paths relative to jupyterlab root
    output_file_relative = str(Path(output_file).relative_to(os.environ["PWD"]))
    app.commands.execute(
        "docmanager:open",
        {"path": output_file_relative, "options": {"mode": "split-right"}},
    )
    time.sleep(1)
    app.commands.execute(
        "notebook:run-cell",
    )


# UI buttons
def create_data_and_visualization(event) -> None:
    code, language, analysis = get_code_from_active_tab()
    # write all print output into the terminal widget
    sys.stdout = terminal
    sys.stderr = terminal
    terminal.clear()
    if language == "cpp":
        run_cpp_code(input_file, code, analysis)
    else:
        if "plot_path" in globals():
            del globals()["plot_path"]

        exec(code, globals())

        if "plot_path" not in globals():
            if analysis == "vectorization":
                print("Run finished.")
            else:
                print("No plot was generated, check logs for errors.")
            return

        plot_path = str(Path(globals()["plot_path"]).relative_to(os.environ["PWD"]))
        app.commands.execute(
            "docmanager:open",
            {"path": plot_path, "options": {"mode": "split-right"}},
        )


button_create_and_visualize = pn.widgets.Button(
    name="Generate Diagram", button_type="primary"
)
pn.bind(create_data_and_visualization, button_create_and_visualize, watch=True)

terminal = pn.widgets.Terminal(
    min_height=250,
    sizing_mode="stretch_both",
    styles={"resize": "both", "overflow": "hidden"},
)
main_widget = pn.FlexBox(
    main, button_create_and_visualize, terminal, sizing_mode="stretch_both"
)
