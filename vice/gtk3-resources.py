#!/usr/bin/env python3
"""
Parse all Gtk3 sources and look for resource declarations

A resource declaration should be in the form:

    "$VICERES <resource-name> [<supported-emus>]"

Where <supported-emus> can be:

* Empty or 'all' to indicate all emus
* A whitespace separated list of emu names (ie 'x64 x64sc x128')
* A whitespace separared list of emu names prefixed with '-', meaning all emus
  except the ones prefixed with '-' (ie '-vsid -scpu64)
"""


import os
import os.path
import re
import pprint


# Directory with gtk3 sources to parse
GTK3_SOURCES = "src/arch/gtk3"

# Extensions of gtk3 sources to parse
GTK3_EXTENSIONS = ('.c')

# List of emus
ALL_EMUS = [ "x64", "x64sc", "xscpu64", "x64dtv", "x128", "xvic", "xpet",
        "xcbm5x0", "xcbm2", "vsid" ]


# Precompile regex to make stuff run a bit faster
regex = re.compile(r"\$VICERES\s+(\w+)\s+(.*)")


def iterate_sources():
    """
    A generator that looks up gtk3 source files

    :yields:    path to gtk3 source file
    """
    for root, dirs, files in os.walk(GTK3_SOURCES):
        for name in files:
            if name.endswith(GTK3_EXTENSIONS):
                yield os.path.join(root, name)


def parse_source(path):
    """
    Parse a gtk3 source file for resource declarations

    :param path: path to gtk3 source file
    :returns: tuple of (resource-name, tuple of emus))
    """

    resources = []

    with open(path, "r") as infile:
        for line in infile:
            result = regex.search(line)
            if result:
                if result.group(2):
                    # TODO: handle the -emu things
                    resources.append((result.group(1), result.group(2).split()))
                else:
                    resources.append((result.group(1), ['all']))
    return resources


def main():
    """
    Main driver, just for testing the code, for now
    """

    for source in iterate_sources():
        resources = parse_source(source)
        if resources:
            print(source[len(GTK3_SOURCES) + 1:])
            pprint.pprint(resources)
            print("")


# include guard: on;y run the code when called as a program, allowing including
# this file in another file as a module
if __name__ == "__main__":
    main()
