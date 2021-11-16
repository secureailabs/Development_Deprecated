import json
from StructuredBuffer import StructuredBuffer
from StructuredBuffer import PyGenerateParamterGuid
import sys
import git

def VerifyJson(strSafeObjectDescriptionFile):
    return True

def CreateSafeObjectStructuredBuffer(oSafeObjectJson, strSafeFunction):
    repo = git.Repo(search_parent_directories=True)
    sha = repo.head.object.hexsha
    short_sha = repo.git.rev_parse(sha, short=5)

    oSafeObject = StructuredBuffer()
    oSafeObject.PutString("Version", oSafeObjectJson['Version']+"."+short_sha)
    oSafeObject.PutString("Title", oSafeObjectJson['Title'])
    oSafeObject.PutString("Description", oSafeObjectJson['Description'])
    oSafeObject.PutString("Uuid", oSafeObjectJson['Uuid'])
    oSafeObject.PutString("Payload", strSafeFunction)

    print(oSafeObject.ToString())

def CreateSafeObjectCode(oSafeObjectJson):
    strTemplateFilename = "SafeObjectTemplate"
    strTemplate = ""
    with open(strTemplateFilename) as fp:
        strTemplate = fp.read()

    # Add the safe object UUID to the file
    strTemplate = strTemplate.replace('{{safeObjectId}}', oSafeObjectJson['Uuid'])

    #  Add the description of the safeObject
    strTemplate = strTemplate.replace('{{Description}}', oSafeObjectJson['Description'])

    # Add all the imports to the safeObject
    strImportsList = oSafeObjectJson['Imports']
    strAllImport = ""
    for strImport in strImportsList:
        strAllImport += strImport + '\n'
    strTemplate = strTemplate.replace('{{Imports}}', strAllImport)

    # Read the file which contains the actual safe function code
    with open(oSafeObjectJson["CodeFile"]) as fp:
        strTemplate = strTemplate.replace('{{Code}}', fp.read().replace("\n", "\n        "))

    # Add the input parameters
    nNumberOfInputs = len(oSafeObjectJson["Inputs"])
    for i in oSafeObjectJson["Inputs"]:
        strGuid = PyGenerateParamterGuid()
        strInputDescription = oSafeObjectJson["Inputs"][i]
        print(strInputDescription)
        strTemplate = strTemplate.replace(i, "self.m_"+strGuid)
        strGetParameterFromFile = "self.m_Sb_"+ strGuid + " = StructuredBuffer(oInputParameters[\""+ strGuid +"\"][\"0\"])"
        strValidateParameter = "if self.m_Sb_"+ strGuid + ".GetString(\"Type\") not in [" + "]:\n            return False"
        nNumberOfInputs -= 1
        if (0 != nNumberOfInputs):
            strGetParameterFromFile += "\n        {{ParamterSet}}\n"
            strValidateParameter += "\n        {{ValidateParams}}\n"
        strTemplate = strTemplate.replace("{{ParamterSet}}", strGetParameterFromFile)
        strTemplate = strTemplate.replace("{{ValidateParams}}", strValidateParameter)

    # Add the output parameters
    nNumberOfOutputs = len(oSafeObjectJson["Outputs"])
    for i in oSafeObjectJson["Outputs"]:
        strGuid = PyGenerateParamterGuid()
        strOutputDescription = oSafeObjectJson["Outputs"][i]
        strTemplate = strTemplate.replace(i, "self.m_"+strGuid)
        strStringToSetParameterFile = "with open(self.m_JobIdentifier+\"." + strGuid + "\",\"wb\") as ofp:\n            pickle.dump(self.m_" + strGuid + ", ofp)\n        with open(\"DataSignals/\" + self.m_JobIdentifier + \"." + strGuid + "\", 'w') as fp:\n            pass"
        nNumberOfOutputs -= 1
        if (0 != nNumberOfOutputs):
            strStringToSetParameterFile += "\n        {{WriteOutputToFile}}\n"
        strTemplate = strTemplate.replace("{{WriteOutputToFile}}", strStringToSetParameterFile)

    return strTemplate

if __name__ == "__main__":
    commandLineArguments = sys.argv[1:]
    if len(commandLineArguments) == 1:
        strSafeObjectDescriptionFile = commandLineArguments[0]
        if True == VerifyJson(strSafeObjectDescriptionFile):
            jsonFilePointer = open(strSafeObjectDescriptionFile)
            oSafeObjectJson = json.load(jsonFilePointer)
            strSafeFunction = CreateSafeObjectCode(oSafeObjectJson)
            if 0 < len(strSafeFunction):
                CreateSafeObjectStructuredBuffer(oSafeObjectJson, strSafeFunction)
                print("Safe Object Created.")
