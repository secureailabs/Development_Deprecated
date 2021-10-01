from .. import SAILPyAPI
import pickle, json, requests, pprint
from concurrent.futures import ThreadPoolExecutor

def connect(serverIP, port):
    return SAILPyAPI.connect(serverIP, port)

def login(email, password):
    return SAILPyAPI.login(email,password)

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
    SAILPyAPI.pushsafeobj(vm, safeobjID)

def submitjob(vm, fnID, jobID):
    return SAILPyAPI.submitjob(vm, fnID, jobID)

# def gettableID(vm):
#     return SAILPyAPI.gettableID(vm)

def registersafeobj(script):
    return SAILPyAPI.registersafeobj(script)

def queryresult(jobid, fnid):
    while(queryjobstatus(jobid)==0):
        #time.sleep(5)
        #print("jobstatus: {} : 0".format(jobid))
        pass
    
    jobstatus = queryjobstatus(jobid)
    if(jobstatus == -1):
        print("job: " + jobid + " is failed")
        return
    
    if(jobstatus == -2):
        class X(str):
            def __repr__(self):
                return "'%s'" % self
    
        errstr = "\x1b[31m Cannot complete the requested job due to a possible privacy violation: too few samples \x1b[0m"
        raise RuntimeError(X(errstr))

    bytelist =  SAILPyAPI.queryresult(jobid, fnid)
    reslist = []
    for buf in bytelist:
        try:
            reslist.append(pickle.loads(buf))
        except pickle.UnpicklingError:
            reslist.append(buf.decode("utf-8"))
    return reslist

def queryresults_parallel(jobids, fnid):
    arglist = []
    for jobid in jobids:
        arglist.append((jobid, fnid))

    results = 0
    with ThreadPoolExecutor() as ex:
        results = ex.map(queryresult, *zip(*arglist))

    return list(results)

def queryjobstatus(jobid):
    result =  SAILPyAPI.queryjobstatus(jobid)
    # if(result==0):
    #     print("job is running")
    # elif(result ==1):
    #     print("job is done")
    # else:
    #     print("job is failed")
    return result

def querydata(vmid):
    return SAILPyAPI.querydata(vmid)

def quit():
    return SAILPyAPI.quit()
    
def create_dummy_data(vm, fnid):
    jobid = newguid()
    #inputs = pushdata(self.vm, [attr])
    #inputs.append(self.data_id)
    #setparameter(self.vm, jobid, self.fns['rdf_getattribute'], inputs)
    submitjob(vm, fnid, jobid)
    pulldata(vm, jobid, fnid)
    result = queryresult(jobid, fnid)
    return result

def get_fns():
    fnsdict = {
        'getitem':'F11C49327A9244A5AEE568B531C6A957',
        'getattr':'9C4019584DB04B1A9BF05EC91836BCB0',
        'series_mean':'0650C80D11A04720BFA8F1693AC292D0',
        'rdf_query':'BF18C294BCCC4B9C94624C79D2506CCC'
    }
    return fnsdict

def VMSetup():

    registersafeobj("/home/jjj/playground/demo/safeobjects/safeobjects")
    print("[P]safe objects registered")

    eosb = login("researcher@researcher.com", "SailPassword@123")
    print("[P]login success")

    url = "https://40.76.22.246:6200/SAIL/VirtualMachineManager/GetRunningVMsIpAdresses?Eosb="+eosb

    payload1 = json.dumps({
        "DigitalContractGuid":"{3ED37E3B-DAC5-472D-9670-2D2A39C6BFF9}"
    })
    payload2 = json.dumps({
        "DigitalContractGuid":"{35703BD8-43F5-4DCC-B536-A2B824A66B79}"
    })
    payload3 = json.dumps({
        "DigitalContractGuid":"{327F1289-3975-4696-9A92-E6066FCB3D05}"
    })
    payloads = [payload1, payload2, payload3]
    headers = {
        'Content-Type': 'application/json'
    }
    
    ips = []
    for payload in payloads:
        response = requests.request("GET", url, headers=headers, data=payload, verify=False)
        response = response.json()
        for key in response['VirtualMachines']:
            ips.append(response['VirtualMachines'][key])
    
    vmids = []
    for ip in ips:
        vmid = connect(ip, 3500)
        vmids.append(vmid)
        print("[P]virtual machine connection to ip: {0} success".format(ip))
    
    fns = get_fns()
    for vm in vmids:
        for key in fns:
            pushsafeobj(vm, fns[key])
    print("[P]safe object pushed to virtual machines")

    table = []
    for vm in vmids:
        tableid = querydata(vm)
        table.append(tableid)
    print("[P]obtain table ids")
    
    from ..data.remote_dataframe import RemoteDataFrame
    demo1= [RemoteDataFrame(vmids[0], table[0]['MGH_biomarker'], fns), 
            RemoteDataFrame(vmids[1], table[1]['BWH_biomarker'], fns),
            RemoteDataFrame(vmids[2], table[2]['BMC_biomaker'], fns)]
    demo2= [RemoteDataFrame(vmids[0], table[0]['MGH_patient'], fns), 
            RemoteDataFrame(vmids[1], table[1]['BWH_patient'], fns),
            RemoteDataFrame(vmids[2], table[2]['BMC_patient'], fns)]
    demo3= [RemoteDataFrame(vmids[0], table[0]['MGH_treatment'], fns), 
            RemoteDataFrame(vmids[1], table[1]['BWH_treatment'], fns),
            RemoteDataFrame(vmids[2], table[2]['BMC_treatment'], fns)]
    demo_data = [demo1, demo3, demo2]
    
    return vmids, demo_data, fns

def dataInfo():
    eosb = login("researcher@researcher.com", "SailPassword@123")

    url = "https://40.76.22.246:6200/SAIL/DigitalContractManager/PullDigitalContract?Eosb="+eosb

    payload1 = json.dumps({
        "DigitalContractGuid":"{3ED37E3B-DAC5-472D-9670-2D2A39C6BFF9}"
    })
    payload2 = json.dumps({
        "DigitalContractGuid":"{35703BD8-43F5-4DCC-B536-A2B824A66B79}"
    })
    payload3 = json.dumps({
        "DigitalContractGuid":"{327F1289-3975-4696-9A92-E6066FCB3D05}"
    })
    payloads = [payload1, payload2, payload3]
    headers = {
        'Content-Type': 'application/json'
    }

    datasetids = []
    for payload in payloads:
        response = requests.request("GET", url, headers=headers, data=payload, verify=False)
        response = response.json()
        datasetids.append(response['DigitalContract']["DatasetGuid"])
    
    dataInfo = []
    for dataset in datasetids:
        tmp_dic = {}
        tmp_dic['DatasetGuid'] = dataset
        payload = json.dumps(tmp_dic)
        data_url = "https://40.76.22.246:6200/SAIL/DatasetManager/PullDataset?Eosb="+eosb
        response = requests.request("GET", data_url, headers=headers, data=payload, verify=False)
        dataInfo.append(response.json())
    
    return dataInfo