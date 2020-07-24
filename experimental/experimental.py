#!/usr/bin/env python3

"""
 Script for automated experimental evaluation.
 @title experimental.py
 @author Vojtech Havlena, July 2020
"""

import sys
import getopt
import subprocess
import string
import re
import os
import os.path
import resource
from enum import Enum

STATESLINE = -2
TIMELINE = -1
CHECK = -3
TIMEOUT = 100 #in seconds
AUT = 100000

class ToolType(Enum):
    RNK = 1
    SAFRA = 2
    PITTERMAN = 3



def main():
    #Input parsing
    if len(sys.argv) < 4:
        help_err()
        sys.exit()
    try:
        opts, args = getopt.getopt(sys.argv[3:], "tf:", ["tex", "aut=", "rnk", "safra", "pitterman"])
    except getopt.GetoptError as err:
        help_err()
        sys.exit()

    bin = sys.argv[1]
    formulafolder = sys.argv[2]
    texout = False
    tool = None
    AUT = 100000

    for o, a in opts:
        if o in ("-t", "--tex"):
            texout = True
        if o in ("-a", "--aut"):
            AUT = int(a)
        if o in ("--rnk"):
            tool = ToolType.RNK
        if o in ("--safra"):
            tool = ToolType.SAFRA
        if o in ("--pitterman"):
            tool = ToolType.PITTERMAN

    if tool is None:
        print("Tool must be specified")
        sys.exit()


    print_fnc = None
    parse_fnc = None
    args = []
    if tool == ToolType.RNK:
        print_fnc = print_output_rnk
        parse_fnc = parse_output_rnk
        args = []
    elif tool == ToolType.SAFRA:
        print_fnc = print_output
        parse_fnc = parse_output_goal
        args = []
    elif tool == ToolType.PITTERMAN:
        print_fnc = print_output
        parse_fnc = parse_output_goal
        args = []


    #Experiments

    files = [f for f in os.listdir(formulafolder) \
        if os.path.isfile(os.path.join(formulafolder, f)) and \
            f.endswith(".ba")]
    files.sort()
    files = files[:AUT]

    print_config(AUT)
    print("Aut;states;time;aux")

    for eq_file in files:
        filename = os.path.join(formulafolder, eq_file)

        try:
            output = subprocess.check_output([bin, filename]+args, \
                timeout=TIMEOUT, stderr=subprocess.STDOUT).decode("utf-8")
            parse = parse_fnc(output)
        except subprocess.TimeoutExpired:
            parse = None, None, None

        filename = os.path.basename(filename)
        print_fnc(filename, parse)


def parse_output_rnk(output):
    lines = output.split('\n')
    lines = list(filter(None, lines)) #Remove empty lines
    sat = lines[STATESLINE]
    model_check = str()
    # if sat == "sat":
    #     match = re.search("Model check: ([a-zA-Z]+)", lines[MODEL_CHECK])
    #     model_check = match.group(1)
    match = re.search("Time: ([0-9]+.[0-9]+)", lines[TIMELINE])
    time = round(float(match.group(1)), 2)
    return sat, time, model_check


def print_config(formulas):
    print("#Timeout: {0}".format(TIMEOUT))
    print("#Number of formulas: {0}".format(formulas))


def format_output(parse):
    return "{0}".format("TO" if parse is None else parse)


def print_output_rnk(filename, rnk_parse):
    print("{0};{1};{2};{3}".format(filename, format_output(rnk_parse[0]), \
        format_output(rnk_parse[1]), format_output(rnk_parse[2])))


def print_output(filename, parse):
    print("{0};{1};{2}".format(filename, format_output(parse[0]), format_output(parse[1])))


def parse_output_goal(output):
    lines = output.split('\n')
    lines = list(filter(None, lines)) #Remove empty lines
    sat = lines[0]
    time = None
    for line in lines:
        match = re.search(r':total-time[\s]+([0-9]+.[0-9]+)', line)
        if match is not None:
            time = round(float(match.group(1)), 2)
            break
    return sat, time


def help_err():
    sys.stderr.write("Bad input arguments. \nFormat: ./experimental [bin]"\
        "[automata folder] [--tex] [--automatas=X]\n")


if __name__ == "__main__":
    main()
