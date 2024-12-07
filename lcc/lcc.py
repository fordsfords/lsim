#!/usr/bin/env python3
# lcc.py
"""Program to provide a higher-level language for lsim."""

from abc import ABC, abstractmethod
import importlib
import os
import argparse
import fileinput
import sys
from contextlib import nullcontext


class LccApi:
    """The API that plugins use to interact with the main program"""

    def __init__(self, args):
        """Init LCC API."""
        self.symtab = {}
        self.args = args
        self._outfile = None  # set as a property.

    @property
    def outfile(self):
        """Getter for outfile: file handle for lcc output."""
        if self._outfile is None:
            raise ValueError("outfile hasn't been initialized yet")
        return self._outfile

    @outfile.setter
    def outfile(self, value):
        """Setter for outfile: file handle for lcc output."""
        self._outfile = value

    def write(self, message):
        """Use instead of print for lcc output."""
        print(message, file=self.outfile)

    def error(self, message):
        """Use instead of print to standard error."""
        self.outfile.flush()
        print(message, file=sys.stderr)

    def update_data(self, key, value):
        """Example method - replace with your actual data structure updates."""
        self.write(f"Updating data: {key} = {value}")
        self.symtab[key] = value

    def register_variable(self, key):
        """Example method for registering variables."""
        self.write(f"Registering variable {key}")
        self.symtab[key] = ""


class DevPlugin(ABC):
    """Base class that all command plugins must inherit from"""

    def __init__(self, lcc_api):
        self.lcc_api = lcc_api

    @abstractmethod
    def parse_command(self, ref_line, fields):
        """Parse the command parameters and use the API to update program state"""

    @property
    @abstractmethod
    def command_name(self):
        """The name of the command this plugin handles"""


class Main:
    """
    Main program.
    """

    def __init__(self):
        parser = argparse.ArgumentParser(
            description="Usage: lcc.py [-v] files...",
        )
        parser.add_argument("-o", "--output", help="Output file")
        parser.add_argument("files", nargs="*", help="Input files (use - for stdin)")

        self.args = parser.parse_args()
        self.lcc_api = LccApi(self.args)
        self.plugins = {}

    def load_plugins(self, plugin_dir):
        """Load plugins."""
        for filename in os.listdir(plugin_dir):
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
                        plugin = item_obj(self.lcc_api)
                        self.plugins[plugin.command_name] = plugin
                        found += 1
                if found < 1:
                    self.lcc_api.error(f"ERROR, plugin {mod_name} needs class based on DevPlugin")
            except ImportError as ex:
                self.lcc_api.error(f"Failed to load plugin {mod_name}: {ex}")

    def process_line(self, ref_line, line):
        """Simple command parsing: semicolon-termianted command, parameters."""
        fields = line.split("#", 1)
        no_comment = fields[0].strip()
        if len(no_comment) == 0:
            return 1

        if not no_comment.endswith(";"):
            self.lcc_api.error(f'ERROR, line must end with semicolon "{ref_line}"')
            return 1
        no_comment = no_comment[:-1]  # Remove final semicolon.

        fields = no_comment.split(";")

        command = fields[0]

        if command in self.plugins:
            return self.plugins[command].parse_command(ref_line, fields)

        self.lcc_api.error(f'ERROR, unrecognized command {command} "{ref_line}"')
        return 1

    def read_file(self):
        """Read input file and process."""
        # Similar to Perl's 'while (<>) {' diamond operator.
        with fileinput.input(files=self.args.files) as file_input:
            for line in file_input:
                ref_line = f"[{file_input.filename()}:{file_input.filelineno()}] {line}"
                self.process_line(ref_line.strip(), line.strip())

    def main(self):
        """Main program."""

        self.load_plugins("plugins")

        # If output is None, this returns sys.stdout in a dummy context manager
        output = self.args.output
        with open(output, "w", encoding="utf-8") if output else nullcontext(sys.stdout) as outfile:
            self.lcc_api.outfile = outfile
            self.read_file()


if __name__ == "__main__":
    Main().main()
