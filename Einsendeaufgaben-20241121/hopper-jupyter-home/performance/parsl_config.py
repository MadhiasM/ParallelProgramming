import logging
import os
import parsl

from parsl.config import Config
from parsl.executors.threads import ThreadPoolExecutor
from parsl.data_provider.files import File
from parsl.dataflow.memoization import id_for_memo

from pathlib import Path

logger = logging.getLogger(__name__)

def load(input_file_path) -> None:
    try:
        parsl.dfk()
    except parsl.errors.NoDataFlowKernelError:
        results_directory = input_file_path.parent / f"{input_file_path.stem}_results"
        results_directory.mkdir(parents=True, exist_ok=True)
        local_threads = Config(
            run_dir=str(results_directory / "workflow_runinfo"),
            executors=[ThreadPoolExecutor(max_threads=8, label="local_threads")],
        )
        parsl.load(local_threads)

# Adds @bash_app stdout and stderr to BashExitFailure
# https://github.com/Parsl/parsl/issues/3356
def patch_parsl():
    is_patched = Path("/tmp/parsl_is_patched")
    if is_patched.exists():
        return False
    else:
        import parsl
        import shutil
        original = Path(parsl.__path__[0] + "/app/bash.py")
        old = "        raise pe.BashExitFailure(func_name, proc.returncode)"""
        new = """
        import sys
        from pathlib import Path
        if getattr(std_out, "name", None):
            print(std_out.name, file=sys.stderr)
            print("")
        if getattr(std_err, "name", None):
            print(std_err.name, file=sys.stderr)
            print(Path(std_err.name).read_text(), file=sys.stderr)
        raise pe.BashExitFailure(func_name, proc.returncode)
        """
        backup = original.with_name(original.stem + "_original" + original.suffix)
        shutil.copy(original, backup)
        original.write_text(original.read_text().replace(old, new))

        is_patched.touch(exist_ok=True)
        return True

# patch during import
patch_parsl()

# Tell parsl, that caching a File object means checking its size and modification time.
# It's only used if app(cache=True)
# https://github.com/Parsl/parsl/issues/1603#issuecomment-597734901
@id_for_memo.register(File)
def id_for_memo_File(f, output_ref=False):
    if output_ref:
        logger.debug(f"hashing File as output ref without content: {f}")
        return f.url
    else:
        logger.debug(f"hashing File as input with content: {f}")
        assert f.scheme == "file"
        filename = f.filepath
        stat_result = os.stat(filename)

        return [f.url, stat_result.st_size, stat_result.st_mtime]
