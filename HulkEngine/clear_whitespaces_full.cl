
import os
import sys
import subprocess

if len(sys.argv) != 2:
    print "clear_whitespaces.py"
    print "--------------"
    print "\tusage: clear_whitespaces.py <fileName>"
    quit()

# command = ["p4","describe",str( sys.argv[1] )]
# text = ""
# try:
    # text = subprocess.check_output(command)
# except:
    # print "p4 not found or returned error code"
    # exit(0)
    
# print text;

f = open(sys.argv[1],"r");
lines = f.readlines();
f.close()

whiteSpaceDetected = 0
tabsDetected = 0

for lineNum in range(0,len(lines)):
    if  len(lines[lineNum]) > 1 and lines[lineNum][-2].isspace():
        whiteSpaceDetected += 1
        print "Trailing white spaces detected in line " + str(lineNum) + ":" + lines[lineNum]
        lines[lineNum] = lines[lineNum].rstrip() + "\n"
    if -1 != lines[lineNum].find("\t"):
        tabsDetected += 1
        lines[lineNum] = lines[lineNum].replace("\t","    ")
        print "Tabs detected in line " + str(lineNum) + ":" + lines[lineNum]


if whiteSpaceDetected > 0 or tabsDetected > 0 :
    print str(whiteSpaceDetected+tabsDetected) + " lines fixed"
    f = open(sys.argv[1],"w")
    f.writelines(lines)
    f.close()
else:
    print "No trailing white spaces or tabs found"
