import sys
import xml.etree.ElementTree as ET

def main():
    if len(sys.argv) != 2:
        sys.stderr.write("Missing arguments")
        sys.exit()
    root = ET.parse(sys.argv[1]).getroot()
    root = toParseable(root)
    for iniSet in root.findall('initialstateset/stateid'):
        print(baState(iniSet.text))
    for tr in root.findall('transitionset/transition'):
        lb = tr.find("read")
        if lb is None:
            lb = tr.find("label").text
        else:
            lb = lb.text
        print("a{0},{1}->{2}".format(lb, baState(tr.find("from").text), baState(tr.find("to").text)))
    for fin in root.findall('acc/stateid'):
        print(baState(fin.text))


def baState(st):
    return "[" + st + "]"


def toParseable(tree):
    t = ET.tostring(tree)
    t = t.lower()
    return ET.fromstring(t)

if __name__ == "__main__":
    main()
