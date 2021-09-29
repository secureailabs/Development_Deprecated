import pandas as pd
import json

def PrivacyValidation(data, strJobIdentifier):

    with open('PrivacySentinelPolicy.json', 'r') as openfile:
        json_object = json.load(openfile)

    # perform the check here
    fIsAccessSafe = False
    strViolationMessage = ""

    if True == isinstance(data, pd.DataFrame):
        if data.shape[0] > json_object['rules']['k']:
            fIsAccessSafe = True
        else:
            strViolationMessage = "Number of enteries in the request is lesser than the minimum threshold set in the Digital Contract"
    elif True == isinstance(data, str):
        if len(data) > 10:
            fIsAccessSafe = True
        else:
            strViolationMessage = "String too small to operate."
    else:
        strViolationMessage = "Unknown data type!!"

    if False == fIsAccessSafe:
        strFileName = strJobIdentifier+".error"
        with open(strFileName,"w") as fp:
            fp.write("Cannot complete the requested job due to a possible policy violation. " + strViolationMessage )

        with open("DataSignals/" + strFileName, 'w') as fp:
            pass

        exit(123)
