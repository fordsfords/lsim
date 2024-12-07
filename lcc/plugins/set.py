"""Example plugin implementation"""
# plugins/set.py

from lcc import DevPlugin


class SetCommand(DevPlugin):
    """Parse the 'set' command."""

    @property
    def command_name(self):
        """Tell main which command is being parsed."""
        return "set"

    def parse_command(self, ref_line, fields):
        """Example parsing: set varname = value."""
        if len(fields) != 3:
            self.lcc_api.error(f'ERROR, set needs 3 fields "{ref_line}"')
            return 1

        var_name = fields[1].strip()
        value_part = fields[2].strip()

        # Register the variable first
        self.lcc_api.register_variable(var_name)
        # Then update its value
        self.lcc_api.update_data(var_name, value_part.strip())

        return 0
