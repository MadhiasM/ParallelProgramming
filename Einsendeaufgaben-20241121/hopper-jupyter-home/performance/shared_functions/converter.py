import os
import shutil
import parsl
from parsl.app.app import bash_app, python_app
from parsl.data_provider.files import File


def convert_input_file(input_file_path) -> None:
    results_directory = input_file_path.parent / f"{input_file_path.stem}_results"
    os.chdir(results_directory)
    if results_directory.exists():
        # clean
        for file in results_directory.glob("*.csv"):
            file.unlink()
    else:
        results_directory.mkdir(parents=True, exist_ok=True)

    if input_file_path.suffix == ".ipynb":
        future = convert_notebook(
            inputs=[File(input_file_path)], outputs=[File(results_directory)]
        )
        future.result()
    else:
        shutil.copy(input_file_path, results_directory / input_file_path.name)

    source_file = input_file_path.with_suffix(".cpp").name
    future = replace_main_markers_in_source_file(inputs=[File(source_file)])
    future.result()


@bash_app(cache=True)
def convert_notebook(
    inputs=(),
    outputs=(),
    stdout=parsl.AUTO_LOGNAME,
    stderr=parsl.AUTO_LOGNAME,
) -> str:
    return f"jupyter nbconvert --to script --output-dir {outputs[0]} {inputs[0]}"


@python_app(cache=True)
def replace_main_markers_in_source_file(inputs=()) -> None:
    """
    Modify the source file:
    - Place code between // start_main and // end_main markers into a main method.
    """
    found_start_main = False
    found_end_main = False
    found_existing_main = False
    found_stdio_header = False
    found_stdlib_header = False
    found_math_header = False

    lines = []
    with open(inputs[0], "r") as source_file:
        if "// MODIFICATION MAIN DONE" in source_file.read():
            return
        source_file.seek(0)
        for line_number, line in enumerate(source_file):
            # Remove lines
            if line.startswith("%"):
                continue
            elif line.startswith("!"):
                continue
            elif "performance.hpp" in line:
                continue
            # Check for main markers or main method
            elif "// start_main" in line or "int main(" in line:
                found_start_main = True
                if "// start_main" in line:
                    lines.append(line)
                if "int main(" in line:
                    lines.append("// start_main\n")
                    found_existing_main = True
                    found_end_main = True
                lines.append("// >>> main start\n")
                lines.append("int main(int argc, char *argv[]) {\n")
                lines.append("// <<< main start\n")
            elif "// end_main" in line:
                found_end_main = True
                lines.append(line)
                lines.append("// >>> main end\n")
                lines.append("return 0;\n}\n")
                lines.append("// <<< main end\n")
                break
            elif "#include <stdio.h>" in line:
                found_stdio_header = True
                lines.append(line)
            elif "#include <stdlib.h>" in line:
                found_stdlib_header = True
                lines.append(line)
            elif "#include <math.h>" in line:
                found_math_header = True
                lines.append(line)
            else:
                lines.append(line)

    if not found_start_main:
        print("Marker missing: // start_main")
        return
    if not found_end_main:
        print("Marker missing: // end_main")
        return

    with open(inputs[0], "w") as new_source:
        # Always provide FILE
        if not found_stdio_header:
            new_source.write("// >>> stdio\n")
            new_source.write("#include <stdio.h>\n")
            new_source.write("// <<< stdio\n")
        # Always provide atoi
        if not found_stdlib_header:
            new_source.write("// >>> stdlib\n")
            new_source.write("#include <stdlib.h>\n")
            new_source.write("// >>> stdlib\n")
        # Always provide math functions
        if not found_math_header:
            new_source.write("// >>> math\n")
            new_source.write("#include <math.h>\n")
            new_source.write("// >>> math\n")

        if found_existing_main:
            for line_number, line in enumerate(lines):
                if "}" in line:
                    end_main_line_number = line_number
            lines.insert(end_main_line_number, "// end_main\n")
        for li in lines:
            new_source.write(li)
        new_source.write("\n// MODIFICATION MAIN DONE\n")
