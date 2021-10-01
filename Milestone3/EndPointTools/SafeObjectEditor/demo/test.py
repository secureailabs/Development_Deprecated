import requests
import json
import pprint
from sail.core import login


eosb = login("researcher@researcher.com", "SailPassword@123")
print(eosb)
#url = "https://40.76.22.246:6200/SAIL/VirtualMachineManager/GetRunningVMsIpAdresses?Eosb="+eosb
#url = "https://40.76.22.246:6200/SAIL/DatasetManager/PullDataset?Eosb="+eosb
url = "https://40.76.22.246:6200/SAIL/DigitalContractManager/PullDigitalContract?Eosb="+eosb

#payload = json.dumps({
#  "DatasetGuid": "{3ED37E3B-DAC5-472D-9670-2D2A39C6BFF9}"
#}

#headers = {
#  'Content-Type': 'application/json'
#}

payload = json.dumps({
    "DigitalContractGuid":"{3ED37E3B-DAC5-472D-9670-2D2A39C6BFF9}"
})
headers = {
    'Content-Type': 'application/json'
}

response = requests.request("GET", url, headers=headers, data=payload, verify=False)

res = response.json()
pprint.pprint(res)
#print(res['VirtualMachines'])
#dic = res['VirtualMachines']
#for key in res['VirtualMachines']:
#    print(key, dic[key])
print(res['DigitalContract']["DatasetGuid"])
