from .. import SAILPyAPI
import pickle, json, requests, pprint, time
import pandas
from concurrent.futures import ThreadPoolExecutor

def connect(serverIP, port):
    return SAILPyAPI.connect(serverIP, port)

def login(email, password, port, IP):
    return SAILPyAPI.login(email,password, port, IP)

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
        time.sleep(2)
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

    res =  SAILPyAPI.queryresult(jobid, fnid)
    reslist = []
    for item in res:
        if(item[0]==0):
            fname = "/tmp/"+item[1]
            with open(fname, 'rb') as f:
                reslist.append(pickle.load(f))
        else:
            reslist.append(item[1])
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
        'setitem':'A04E4CC9E3BC4A7B9AFAB6CB3E040FAC',
        'iter':'CC8B5A66C10F47A1A977E2BE2B522768',
        'next':'6D48AC3C3245411992E65E5FF5B90DDB',
        'rdf_describe':'18F2566E7FF34D77A7DE668DD220CEFD',
        'rdf_drop':'9A04D50B403C4716A79CABEF1F90D832',
        'rdf_dropna':'039181877F8A492A86A4BD996D0AD4E2',
        'rdf_merge':'8C938D7D313A49C09509D6A43AAFD096',
        'rdf_groupby':'3E3716FCA7DE4ABEB8A3FAB4DA14EA9D',
        'rdf_query':'BF18C294BCCC4B9C94624C79D2506CCC',
        'rdf_sort_values':'10DCB6636C8A4832AD6CE7BCDB1F9983',
        'rdf_set_index':'09EE6479EE97461DBC4E44D95D77235B',
        'rdf_reset_index':'F408E02C932D47C49BC5DC640C108AEB',
        'rdf_label_encode':'56D864CD0B81459CB0540E364BB49D6F',
        'groupby_agg':'CEFAABE1879741269037050B25A5CCFD',
        'groupby_diff':'9604982079924320B6C7B6DCA27CFC6C',
        'groupby_cumsum':'AB04C77FA5DD4B1B8AAD2867F44BC886',
        'groupby_first':'FF2156013B9A483F9FC8F0DB27E3B74D',
        'series_add':'B13E83BD3F5743BCABE36FDB418F100C',
        'series_and':'3E58E89566AA423093D7184988A60854',
        'series_astype':'F1B6F4B3359D4371A653C395418BF12A',
        'series_eq':'8972411C102743EF9F262DBAEA9E8693',
        'series_fillna':'5C0BC5A7F9714DA79017E3FCF16C26D6',
        'series_ge':'DE9C9AB3D1CB410291ABD038A5A1AF92',
        'series_gt':'34A48D79AC9843F9ACF2B2E3A7C8F4A0',
        'series_invert':'8DE96396431C4DCD897225B94971515B',
        'series_isin':'1463121DB74B467F82B212B4190FAF5C',
        'series_isna':'EAA299EEF38149B58830A8267D1DE5F4',
        'series_le':'BEE2F244260445AC9A634B00CC753630',
        'series_lt':'556B3C62AA88461F977CB55220B79C73',
        'series_map':'757A54DFA63346D28CC39571A63BE6B4',
        'series_max':'76ABB96DA9EB4FAA8E3073EBED270EDC',
        'series_mean':'0650C80D11A04720BFA8F1693AC292D0',
        'series_min':'EE498A68C65347D68ACD6661207EC705',
        'series_or':'8BBCF91120A749AE9CE22E0855B89B60',
        'series_ravel':'2ADA37AD4D9A49689183845183CA3096',
        'series_sub':'0899F5B598904B279AA98ADFD923888F',
        'series_unique':'B6FD9631354A4985A3E674DF28457A5D',
        'series_value_counts':'2275B04657D94BFD8787C519C2C3B8EA',
        'util_get_dummies':'6FB479BE72A04B158C15FD5EBDF875FC',
        'util_to_datetime':'D35FC476B97F42AB9CE45D8E7604DD3C',
        'util_to_numeric':'6299E37D96884EAFB9E21692FB24014B',
        'util_ravel':'43B37087962644229B53B0D7C3A1E386',
        'util_where':'98F83AA9DC3249B983A4262BE0BCEB55',
        'util_train_test_split':'AF83E839A5514B178951B205F5CCB6E5',
        'util_read_csv':'E764BBFDB1C44271837A1384CD40AD94'
    }
    return fnsdict

def VMSetup(contractdict, backendIP, soPath):

    registersafeobj(soPath)
    print("[P]safe objects registered", flush=True)

    eosb = login("jingwei@kpmg.com", "-OJBFE2qw-OJBFE2qw", 6200, backendIP)
    print("[P]login success", flush=True)

    #url = "https://40.76.22.246:6200/SAIL/VirtualMachineManager/GetRunningVMsIpAdresses?Eosb="+eosb
    url = "https://"+backendIP+":6200/SAIL/VirtualMachineManager/GetRunningVMsIpAdresses?Eosb="+eosb
    # payload1 = json.dumps({
    #     "DigitalContractGuid":"{3ED37E3B-DAC5-472D-9670-2D2A39C6BFF9}"
    # })
    # payload2 = json.dumps({
    #     "DigitalContractGuid":"{35703BD8-43F5-4DCC-B536-A2B824A66B79}"
    # })
    # payload3 = json.dumps({
    #     "DigitalContractGuid":"{327F1289-3975-4696-9A92-E6066FCB3D05}"
    # })

    payloads = []
    for item in contractdict:
        temp = json.dumps(item)
        payloads.append(temp)

    headers = {
        'Content-Type': 'application/json'
    }
    
    ips = []
    for payload in payloads:
        print(payload)
        response = requests.request("GET", url, headers=headers, data=payload, verify=False)
        response = response.json()
        for key in response['VirtualMachines']:
            ips.append(response['VirtualMachines'][key])
            print(ips)
    
    vmids = []
    for ip in ips:
        vmid = connect(ip, 3500)
        vmids.append(vmid)
        print("[P]virtual machine connection to ip: {0} success".format(ip), flush=True)
    
    fns = get_fns()
    for vm in vmids:
        for key in fns:
            pushsafeobj(vm, fns[key])
    print("[P]safe object pushed to virtual machines" , flush=True)

    table = []
    for vm in vmids:
        tableid = querydata(vm)
        table.append(tableid)
    print("[P]obtain table ids", flush=True)
    
    # from ..data.remote_dataframe import RemoteDataFrame
    # demo1= [RemoteDataFrame(vmids[0], table[0]['MGH_biomarker'], fns), 
    #         RemoteDataFrame(vmids[1], table[1]['BWH_biomarker'], fns),
    #         RemoteDataFrame(vmids[2], table[2]['BMC_biomaker'], fns)]
    # demo2= [RemoteDataFrame(vmids[0], table[0]['MGH_patient'], fns), 
    #         RemoteDataFrame(vmids[1], table[1]['BWH_patient'], fns),
    #         RemoteDataFrame(vmids[2], table[2]['BMC_patient'], fns)]
    # demo3= [RemoteDataFrame(vmids[0], table[0]['MGH_treatment'], fns), 
    #         RemoteDataFrame(vmids[1], table[1]['BWH_treatment'], fns),
    #         RemoteDataFrame(vmids[2], table[2]['BMC_treatment'], fns)]
    #demo_data = [demo1, demo3, demo2]
    
    return vmids, table, fns

def dataInfo(digitalcontracts, backendIP):
    eosb = login("researcher@researcher.com", "SailPassword@123")

    #url = "https://40.76.22.246:6200/SAIL/DigitalContractManager/PullDigitalContract?Eosb="+eosb
    url = "https://"+backendIP+":6200/SAIL/DigitalContractManager/PullDigitalContract?Eosb="+eosb

    # payload1 = json.dumps({
    #     "DigitalContractGuid":"{3ED37E3B-DAC5-472D-9670-2D2A39C6BFF9}"
    # })
    # payload2 = json.dumps({
    #     "DigitalContractGuid":"{35703BD8-43F5-4DCC-B536-A2B824A66B79}"
    # })
    # payload3 = json.dumps({
    #     "DigitalContractGuid":"{327F1289-3975-4696-9A92-E6066FCB3D05}"
    # })
    #payloads = [payload1, payload2, payload3]
    payloads = []
    for item in digitalcontracts:
        temp = json.dumps(item)
        payloads.append(temp)

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
        #data_url = "https://40.76.22.246:6200/SAIL/DatasetManager/PullDataset?Eosb="+eosb
        data_url = "https://"+backendIP+":6200/SAIL/DatasetManager/PullDataset?Eosb="+eosb
        response = requests.request("GET", data_url, headers=headers, data=payload, verify=False)
        dataInfo.append(response.json())
    
    return dataInfo