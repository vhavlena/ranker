#!/usr/bin/env python3

"""
 Script for automated results processing.
 @title csv-eval.py
 @author Vojtech Havlena, August 2020
"""

import sys
import getopt
import subprocess
import string
import re
import os
import os.path
import resource
import csv
import xml.etree.ElementTree as ET
from enum import Enum

FILTERROW = ["rnk", "piterman", "check"]

def main():
    if len(sys.argv) < 2:
        help_err()
        sys.exit()

    files = sys.argv[1:]
    filesfd = []
    filesfdr = []
    for f in files:
        try:
            ftmp = open(f, "r")
            filesfd.append(ftmp)
        except IOError:
            sys.stderr.write("Cannot open the file {0}".format(f))
            close_files(filesfd)

    header = "aut;"
    for fd in filesfd:
        name = fd.readline().strip()
        header += name[1:] + ";"
        filesfdr.append(csv.DictReader(filter(lambda row: row[0] != "#", fd), delimiter=";"))

    fltrows = dict()
    for rd in filesfdr:
        for row in rd:
            autname = row["aut"] #os.path.splitext(row["aut"])[0]
            item = []
            for fl in FILTERROW:
                try:
                    item.append(row[fl])
                except KeyError:
                    continue
            try:
                fltrows[autname] += item
            except KeyError:
                fltrows[autname] = item

    print(header)
    for k, v in fltrows.items():
        print(format_row(k, v))
    close_files(filesfd)


def help_err():
    sys.stderr.write("Bad input arguments.\n")


def close_files(fds):
    for f in fds:
        f.close()


def format_row(name, vals):
    s = ""
    s += name + ";"
    for v in vals:
        s += str(v) + ";"
    return s


if __name__ == "__main__":
    main()
