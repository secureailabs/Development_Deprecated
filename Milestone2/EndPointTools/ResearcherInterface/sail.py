import SAILPyAPI as sail
import pickle

def connect(serverIP, port):
    return sail.connect(serverIP, port)

def newguid():
    return sail.createguid()

def pushdata(vm, jobID, fnID, vars, home):
    i=0
    for var in vars:
        pickle.dump(var, open(home+"/"+jobID+"_"+str(i), "wb"))
        i+=1
    sail.pushdata(vm, jobID, fnID, home)

def pulldata(vm, jobID, fnID, home):
    vars = []
    varIDs = sail.pulldata(vm, jobID, fnID, home)
    for varID in varIDs:
        vars.append(pickle.load(open(home+"/"+jobID+varID, "rb")))
    return vars

def deletedata(vm, varIDs):
    sail.deletedata(varIDs)

def pushfn(vm, fnID):
    sail.pushfn(vm, fnID)

def execjob(vm, fnID, jobID):
    sail.execjob(vm, fnID, jobID)

def gettableID(vm):
    return sail.gettableID(vm)

def registerfn(script, inputnumber, outputnumber):
    return sail.registerfn(script, inputnumber, outputnumber)

def quit():
    sail.quit()
