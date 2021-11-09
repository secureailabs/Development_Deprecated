from SafeObjectAPI import newguid, writeSafeObject
import json
import os

safeObjectFolder = ""

def readSafeObjects(filepath):
    resultdict={}
    for filename in os.listdir(filepath):
        if filename.endswith(".py"):
            fullpath = os.path.join(filepath, filename)
            conffile = os.path.splittext(fullpath)[0]
            f= open(fullpath, "rb")
            result['script'] = f.read()
            f.close()
            f = open(conffile, "rb")
            jdict = readjson(f)
            f.close()
            resultdict = {**resultdict, **jdict}
            
            
            

def readjson(file):
    jdict = json.load(file)
    return jdict

def generateGuid(jdict):

    #resultDict["Title"] = self.title
    #resultDict["Description"] = self.description
    jDict["uuid"] = newguid()
    inputLen = jdict["numberOfInput"]
    outputLen = jdict["numberOfOutput"]
    
    inputParams = []
    for i in range(inputTableLen):
        tmpTableArr = []
        tmpTableArr.append(newguid())
        for j in range(4):
            tmpTableArr.append(self.inputParameterArea.getTableContent(i, j))
        inputParams.append(tmpTableArr)
    resultDict["input"] = inputParams
        
    outputParams = []
    for i in range(outputTableLen):
        tmpTableArr = []
        tmpTableArr.append(newguid())
        for j in range(4):
            tmpTableArr.append(self.outputParameterArea.getTableContent(i, j))
        outputParams.append(tmpTableArr)
    resultDict["output"] = outputParams

    generateSafeObject(resultDict)

def generateSafeObject(jdict):
    f = open("SafeObjectTemplate", "r")
    template = f.read()

    codeScript = jdict['script']
    codeScript = codeScript.replace("\n", "\n        ")
    template = template.replace("{{code}}", codeScript)
    template = template.replace("{{safeObjectId}}", jdict['safeObjectGuid'])

    for i in range(jdict["numberOfInput"]):
        strParamGuid = jdict["input"][i][0]
        template = template.replace(jdict["input"][i][1], "self.m_"+strParamGuid)
        strStringToGetParameterFromFile = "with open(oInputParameters[\""+ strParamGuid +"\"][\"0\"], 'rb') as ifp:\n            self.m_"+ strParamGuid + " = pickle.load(ifp)"
        if (i != (jdict["numberOfInput"]-1)):
            strStringToGetParameterFromFile += "\n        {{ParamterSet}}\n"
        template = template.replace("{{ParamterSet}}", strStringToGetParameterFromFile)

    for i in range(jdict["numberOfOutput"]):
        strOutputParamterGuid = jdict["output"][i][0]
        template = template.replace(jdict["output"][i][1], "self.m_" + strOutputParamterGuid)
        strStringToSetParameterFile = "with open(self.m_JobIdentifier+\"." + strOutputParamterGuid + "\",\"wb\") as ofp:\n            pickle.dump(self.m_" + strOutputParamterGuid + ", ofp)\n        with open(\"DataSignals/\" + self.m_JobIdentifier + \"." + strOutputParamterGuid + "\", 'w') as fp:\n            pass"
        if (i != (jdict["numberOfOutput"]-1)):
            strStringToSetParameterFile += "\n        {{WriteOutputToFile}}\n"
        template = template.replace("{{WriteOutputToFile}}", strStringToSetParameterFile)
        
    jdict["body"] = template
    #print(template)
    writeSafeObject(jdict)
    f.close()
