import SAILPyAPI as sail
import pickle

def connect(serverIP, port, email, password):
    return sail.connect(serverIP, port, email, password)

def newguid():
    return sail.createguid()

def pushdata(vm, jobID, fnID, inputList, confidentialInputList, home):
    i=0
    for var in inputList:
        pickle.dump(var, open(home+"/"+jobID+"_"+str(i), "wb"))
        i+=1
    sail.pushdata(vm, jobID, fnID, confidentialInputList, home)

def pulldata(vm, jobID, fnID, home):
    vars = []
    varIDs = sail.pulldata(vm, jobID, fnID, home)
    for varID in varIDs[0]:
        filename = home+"/"+jobID+varID
        with open(filename, "rb") as h:
            vars.append(pickle.load(h))
    return [vars, varIDs[1]]

def deletedata(vm, varIDs):
    sail.deletedata(varIDs)

def pushfn(vm, fnID):
    sail.pushfn(vm, fnID)

def execjob(vm, fnID, jobID):
    sail.execjob(vm, fnID, jobID)

def gettableID(vm):
    return sail.gettableID(vm)

def registerfn(script, inputnumber, confidentialInputNumber, outputnumber, confidentialOutputNumber):
    return sail.registerfn(script, inputnumber, outputnumber, confidentialInputNumber, confidentialOutputNumber)

def quit():
    sail.quit()
