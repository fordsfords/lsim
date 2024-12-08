#!/usr/bin/env python3
# lcc.py
"""Program to provide a higher-level language for lsim.
Claude.ai helped me with significant parts of this logic."""

from abc import ABC, abstractmethod
import importlib
import os
import argparse
import fileinput
import sys
from contextlib import nullcontext
from typing import Optional, TextIO, Dict, Any, List


class LccApi:
    """The API that device plugins use to interact with the main program"""

    def __init__(self, args: argparse.Namespace) -> None:
        """Init LCC API."""
        self.symtab: Dict[str, Any] = {}
        self.args = args
        self._outfile: Optional[TextIO] = None  # set as a property.

    @property
    def outfile(self) -> TextIO:
        """Getter for outfile: file handle for lcc output."""
        if self._outfile is None:
            raise ValueError("outfile hasn't been initialized yet")
        return self._outfile

    @outfile.setter
    def outfile(self, value: TextIO) -> None:
        """Setter for outfile: file handle for lcc output."""
        self._outfile = value

    def write(self, message: str) -> None:
        """Use instead of print for lcc output."""
        print(message, file=self.outfile)

    def error(self, message: str) -> None:
        """Use instead of print to standard error."""
        self.outfile.flush()
        print(message, file=sys.stderr)


class DevPlugin(ABC):
    """Base class that all device plugins must inherit from"""

    def __init__(self, lcc_api: LccApi) -> None:
        self.lcc_api = lcc_api  # Functions plugin can make.

    @abstractmethod
    def parse_dev(self, ref_line: str, fields: List[str]) -> int:
        """Parse the device parameters and use the API to update program state"""

    @property
    @abstractmethod
    def dev_type_name(self) -> str:
        """The name of the device type this plugin handles"""


class Main:
    """Main program."""

    def __init__(self) -> None:
        # Command-line options.
        parser = argparse.ArgumentParser(
            description="Usage: lcc.py [-v] files...",
        )
        parser.add_argument("-o", "--output", help="Output file")
        parser.add_argument("files", nargs="*", help="Input files (use - for stdin)")

        self.args = parser.parse_args()
        self.lcc_api = LccApi(self.args)
        self.dev_plugins: Dict[str, DevPlugin] = {}

    def load_dev_plugins(self) -> None:
        """Load device plugins from directory 'plugins'."""
        for filename in os.listdir("plugins"):
            if not filename.endswith(".py"):
                continue
            if filename.startswith("_"):
                continue

            mod_name = filename[:-3]
            try:
                module = importlib.import_module(f"plugins.{mod_name}")
                # Look for a class that inherits from DevPlugin
                found = 0
                for item in dir(module):
                    item_obj = getattr(module, item)
                    if not hasattr(item_obj, "__bases__"):
                        continue

                    # Get base class names as strings
                    base_names = [f"{base.__module__}.{base.__name__}" for base in item_obj.__bases__]
                    if "lcc.DevPlugin" in base_names:
                        dev_plugin = item_obj(self.lcc_api)
                        self.dev_plugins[dev_plugin.dev_type_name] = dev_plugin
                        found += 1
                if found < 1:
                    self.lcc_api.error(f"ERROR, plugin {mod_name} needs class based on DevPlugin")
            except ImportError as ex:
                self.lcc_api.error(f"Failed to load plugin {mod_name}: {ex}")

    def process_d_cmd(self, ref_line: str, fields: List[str]) -> int:
        """Define device command."""
        dev_type = fields[1]
        if dev_type not in self.dev_plugins:
            self.lcc_api.error(f'ERROR, unrecognized device type {dev_type} "{ref_line}"')
            return 1

        # Call plugin and return its status
        return self.dev_plugins[dev_type].parse_dev(ref_line, fields)

    def process_c_cmd(self, ref_line: str, _: List[str]) -> int:
        """Define device command."""
        # cmd, src_dev, src_out_id, dst_dev, dst_in_id = fields[0:5]
        self.lcc_api.error(f'ERROR, process_c_cmd is TBD "{ref_line}"')
        return 1

    def process_line(self, ref_line: str, line: str) -> int:
        """Simple device parsing: semicolon-termianted command, parameters."""
        # Strip comments
        fields = line.split("#", 1)
        no_comment = fields[0].strip()
        if len(no_comment) == 0:  # Blank lines are OK.
            return 0

        if " " in no_comment:
            self.lcc_api.error(f'ERROR, command should contain no spaces "{ref_line}"')
            return 1
        if not no_comment.endswith(";"):
            self.lcc_api.error(f'ERROR, line must end with semicolon "{ref_line}"')
            return 1

        # Remove final semicolon so that split doesn't create trailing empty field.
        no_comment = no_comment[:-1]
        fields = no_comment.split(";")

        cmd = fields[0]
        match cmd:
            case "d":
                return self.process_d_cmd(ref_line, fields)
            case "c":
                return self.process_c_cmd(ref_line, fields)
            case _:
                self.lcc_api.error(f'ERROR, unrecognized command {cmd} "{ref_line}"')
                return 1

    def read_file(self) -> int:
        """Read input file and process."""
        num_errs = 0
        # Similar to Perl's 'while (<>) {' diamond operator.
        with fileinput.input(files=self.args.files) as file_input:
            for line in file_input:
                ref_line = f"[{file_input.filename()}:{file_input.filelineno()}] {line}"
                num_errs += self.process_line(ref_line.strip(), line.strip())
        return num_errs

    def main(self) -> None:
        """Main program."""
        self.load_dev_plugins()

        # If output is None, this returns sys.stdout in a dummy context manager
        output: Optional[str] = self.args.output
        with open(output, "w", encoding="utf-8") if output else nullcontext(sys.stdout) as outfile:  # type: ignore
            self.lcc_api.outfile = outfile
            num_errs = self.read_file()

        if num_errs > 0:
            sys.exit(1)


if __name__ == "__main__":
    Main().main()
