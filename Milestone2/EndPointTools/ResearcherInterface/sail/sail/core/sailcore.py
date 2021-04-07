from .. import SAILPyAPI
import pickle

def connect(serverIP, port, email, password):
    return SAILPyAPI.connect(serverIP, port, email, password)

def newguid():
    return SAILPyAPI.createguid()

def pushdata(vm, jobID, fnID, inputList, confidentialInputList, home):
    i=0
    for var in inputList:
        pickle.dump(var, open(home+"/"+jobID+"_"+str(i), "wb"))
        i+=1
    SAILPyAPI.pushdata(vm, jobID, fnID, confidentialInputList, home)

def pulldata(vm, jobID, fnID, home):
    vars = []
    varIDs = SAILPyAPI.pulldata(vm, jobID, fnID, home)
    for varID in varIDs[0]:
        filename = home+"/"+jobID+varID
        with open(filename, "rb") as h:
            vars.append(pickle.load(h))
    return [vars, varIDs[1]]

def deletedata(vm, varIDs):
    SAILPyAPI.deletedata(varIDs)

def pushfn(vm, fnID):
    SAILPyAPI.pushfn(vm, fnID)

def execjob(vm, fnID, jobID):
    SAILPyAPI.execjob(vm, fnID, jobID)

def gettableID(vm):
    return SAILPyAPI.gettableID(vm)

def registerfn(script, inputnumber, confidentialInputNumber, outputnumber, confidentialOutputNumber):
    return SAILPyAPI.registerfn(script, inputnumber, outputnumber, confidentialInputNumber, confidentialOutputNumber)

def quit():
    SAILPyAPI.quit()
    
def spawnvms(numberOfVMs):
    vms = []
    for i in range(numberOfVMs):
        vm = SAILPyAPI.connect("127.0.0.1", 7001+i, "marine@terran.com", "sailpassword")
        vms.append(vm)
    return vms

def connectToVMs(config):
    f = open(config, 'r')
    ips = []
    usernames = []
    passwords = []
    for line in f:
        arr = line.split(',')
        ips.append(arr[0])
        usernames.append(arr[1])
        passwords.append(arr[2])
    print(ips)
    vms = []
    for i in range(len(ips)):
        vm = SAILPyAPI.connect(ips[i], 7000, usernames[i], passwords[i])
        vms.append(vm)
        print(vm)
    return vms