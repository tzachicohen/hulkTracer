
import os.path
import re

matcher = re.compile(r".*cl")

for dirpath,dirname,filenames in os.walk(os.path.join(os.getcwd(),"src")):
	for file in filenames:
		if matcher.match(file):
			f = open(os.path.join('src',file),"r")

			sourceLines = f.readlines();

			# 'includeFound' is set to true just so that
			# we enter the loop in the first time
			includeFound = True
			# we process the '#include' directive until
			# we can't find any
			while includeFound : 
				includeFound = False
				for line in range(0,len(sourceLines)):
					if -1 != sourceLines[line].find("#include"):
						fileName = sourceLines[line][sourceLines[line].find('"')+1:sourceLines[line].rfind('"')]
						path = os.path.join("src\\",fileName)
						if os.path.exists(path):
							includeFound = True
							newFile = open(path,"r")
							newLines = newFile.readlines()
							sourceLines = sourceLines[:line] + newLines +  sourceLines[line+1:]
							break
						path = os.path.join("public",fileName)
						if os.path.exists(path):
							includeFound = True
							newFile = open(path,"r")
							newLines = newFile.readlines()
							sourceLines = sourceLines[:line] + newLines +  sourceLines[line+1:]
							break
						# if the include file was not found we remov the line.
						sourceLines[line] = ""

			f = open(file[:-3]+"_full.cl","w")
			f.writelines(sourceLines)
			f.close()