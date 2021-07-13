from .. import SAILPyAPI
import pickle

def connect(serverIP, port, email, password):
    return SAILPyAPI.connect(serverIP, port, email, password)

def newguid():
    return SAILPyAPI.createguid()

def pushdata(vm, jobID, fnID, inputList):
    inputs = []
    for var in inputList:
        inputs.append(pickle.dumps(var))
    SAILPyAPI.pushdata(vm, jobID, fnID, inputs)

def pulldata(vm, jobID, fnID):
    # outputs = []
    # OutputList = 
    return SAILPyAPI.pulldata(vm, jobID, fnID)
    # for var in OutputList:
    #     outputs.append(pickle.loads(var))
    # return outputs
    

def setparameter(vm, jobID, fnID, newParams, oldParams):
    return SAILPyAPI.setparameter(vm, jobID, fnID, newParams, oldParams)

# def deletedata(vm, varIDs):
#     SAILPyAPI.deletedata(varIDs)

def pushsafeobj(vm, safeobjID):
    return SAILPyAPI.pushfn(vm, safeobjID)

def submitjob(vm, fnID, jobID):
    return SAILPyAPI.submitjob(vm, fnID, jobID)

# def gettableID(vm):
#     return SAILPyAPI.gettableID(vm)

def registersafeobj(script, inputnumber, outputnumber):
    return SAILPyAPI.registerfn(script, inputnumber, outputnumber)

def queryresult(jobid):
    return SAILPyAPI.queryresult(jobid)

def queryjobstatus(jobid):
    return SAILPyAPI.queryjobstatus(jobid)

def quit():
    return SAILPyAPI.quit()
    
def spawnvms(numberOfVMs):
    vms = []
    for i in range(numberOfVMs):
        vm = SAILPyAPI.connect("127.0.0.1", 7001+i, "marine@terran.com", "sailpassword")
        vms.append(vm)
    return vms

def configVMs(config):
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