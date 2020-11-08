import os
import re

def getFileNames(path):
    #path = "C:/Users/William/Documents/University/Fall 2020/4 Senior Design/JHF"
    #path = "JHF"
    dirs = os.listdir(path)
    return dirs

def fixCountyIds(dirs):
    replaced = 0
    for file in dirs:
        filePath = "JHF/" + file
        with open(filePath, "r+") as fp:
            lines = fp.readlines()
            cnt = 0
            for line in lines:
            #for cnt, line in enumerate(fp):
                if(line.find('[46102') != -1):  #Oglala Lakota
                    #print(line)
                    subbedLineOglala = re.sub("46102", "46113", line)
                    lines[cnt] = subbedLineOglala
                    #fp.write(subbedLineOglala)
                    #print(subbedLineOglala)
                    replaced += 1
                if(line.find('[2158') != -1):   #Kusilvak
                    #print(line)
                    subbedLineKusilvak = re.sub("2158", "2270", line)
                    lines[cnt] = subbedLineKusilvak
                    #fp.write(subbedLineKusilvak)
                    #print(subbedLineKusilvak)
                    replaced += 1
                cnt += 1
            fp.truncate(0)
            fp.seek(0)
            fp.writelines(lines)
    print("Replaced lines: " + str(replaced))

dirs = getFileNames("JHF")
fixCountyIds(dirs)