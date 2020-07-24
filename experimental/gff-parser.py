import sys
import xml.etree.ElementTree as ET

def main():
    if len(sys.argv) != 2:
        sys.exit()
    root = ET.parse(sys.argv[1]).getroot()
    for iniSet in root.findall('initialStateSet/stateID'):
        print(baState(iniSet.text))
    for tr in root.findall('transitionSet/transition'):
        print("a{0},{1}->{2}".format(tr.find("read").text, baState(tr.find("from").text), baState(tr.find("to").text)))
    for fin in root.findall('acc/stateID'):
        print(baState(fin.text))


def baState(st):
    return "[" + st + "]"

if __name__ == "__main__":
    main()
