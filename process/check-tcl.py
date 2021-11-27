import re

file = "test.tr"
f = open(file, 'r')
for line in f.readlines():
    match = re.match(".*-t ([0-9]\.[0-9]+).*aodvuu.*", line)
    if match is not None:
        # print(line)
        time = match.group(1)
        print(time)