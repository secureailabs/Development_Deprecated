from .. import SAILPyAPI
import pickle
from concurrent.futures import ProcessPoolExecutor

def connect(serverIP, port, email, password):
    return SAILPyAPI.connect(serverIP, port, email, password)

def newguid():
    return SAILPyAPI.createguid()

def pushdata(vm, inputList):
    inputs = []
    dataid = []
    for var in inputList:
        inputs.append(pickle.dumps(var))
        dataid.append(newguid())
    SAILPyAPI.pushdata(vm, dataid, inputs)
    return dataid

def pulldata(vm, jobID, fnID):
    # outputs = []
    # OutputList = 
    return SAILPyAPI.pulldata(vm, jobID, fnID)
    # for var in OutputList:
    #     outputs.append(pickle.loads(var))
    # return outputs
    

def setparameter(vm, jobID, fnID, parameterId):
    return SAILPyAPI.setparameter(vm, jobID, fnID, parameterId)

# def deletedata(vm, varIDs):
#     SAILPyAPI.deletedata(varIDs)

def pushsafeobj(vm, safeobjID):
    return SAILPyAPI.pushsafeobj(vm, safeobjID)

def submitjob(vm, fnID, jobID):
    return SAILPyAPI.submitjob(vm, fnID, jobID)

# def gettableID(vm):
#     return SAILPyAPI.gettableID(vm)

def registersafeobj(script):
    return SAILPyAPI.registersafeobj(script)

def queryresult(jobid, fnid):
    while(queryjobstatus(jobid)==0):
        pass
    
    jobstatus = queryjobstatus(jobid)
    if(jobstatus == -1):
        print("job: " + jobid + " is failed")

    bytelist =  SAILPyAPI.queryresult(jobid, fnid)
    reslist = []
    for buf in bytelist:
        reslist.append(pickle.loads(buf))
    return reslist

def queryresults_parallel(jobids, fnid):
    arglist = []
    for jobid in jobids:
        arglist.append((jobid, fnid))
    
    results = 0
    with ProcessPoolExecutor() as executor:
        results = executor.map(queryresult, arglist)
    
    return results

def queryjobstatus(jobid):
    result =  SAILPyAPI.queryjobstatus(jobid)
    if(result==0):
        print("job is running")
    elif(result ==1):
        print("job is done")
    else:
        print("job is failed")
    return result

def querydata(vmid):
    return SAILPyAPI.querydata(vmid)

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