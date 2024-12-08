"""Example plugin implementation"""
# plugins/nand.py

from typing import List
from lcc import DevPlugin


class NandDev(DevPlugin):
    """Parse the 'nand' command."""

    @property
    def dev_type_name(self) -> str:
        """Tell main which command is being parsed."""
        return "nand"

    def parse_dev(self, ref_line: str, fields: List[str]) -> int:
        """Example parsing: d;nand;dev_name;num_inputs;"""
        if len(fields) != 4:
            self.lcc_api.error(f'ERROR, {self.dev_type_name} needs 4 fields "{ref_line}"')
            return 1

        cmd, dev_type, dev_name, num_inputs = fields[0:4]
        self.lcc_api.write(f"{cmd};{dev_type};{dev_name};{num_inputs};")

        return 0
