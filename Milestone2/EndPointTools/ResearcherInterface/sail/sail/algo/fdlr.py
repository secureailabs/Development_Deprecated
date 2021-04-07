from .fdcore import Algorithm
from ..core import newguid, pushdata, pulldata, pushfn, execjob, registerfn
import torch.nn as nn
import time

class Fdlr(Algorithm):
    def __init__(self, vms, vmagg, data, workspace):
        super().__init__(vms, vmagg, data, workspace)
        self.model = 0
        self.parties = 0
        self.fns = self.setfn()
        self.initvms()
    
    def setfn(self):
        fndict = {}
        fndict['init_submodel'] = registerfn("fn_modelinit.py", 1, 2, 0, 1)[0]
        fndict['get_grad'] = registerfn("fn_getGrad.py", 0, 1, 1, 0)[0]
        fndict['update_grad'] = registerfn("fn_updateGradients.py", 1, 1, 0, 1)[0]
        fndict['agg'] = registerfn("fn_agg.py", 1, 0, 1, 0)[0]
        fndict['test'] = registerfn("fn_test.py", 1, 1, 1, 0)[0]
        fndict['mae'] = registerfn("fn_mae.py", 0, 3, 1, 0)[0]
        
        return fndict
    
    def initvms(self):
        for vm in self.vms:
            for key in self.fns:
                pushfn(vm, self.fns[key])
        pushfn(self.vmagg, self.fns['agg'])
    
    def initmodel(self, dimx, dimy):
        self.model = nn.Linear(dimx, dimy)
    
    def initsubmodels(self):
        party_models = []
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['init_submodel'], [self.model], [self.data['X_train'][i], self.data['y_train'][i]], self.workspace)
            execjob(self.vms[i], self.fns['init_submodel'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['init_submodel'], self.workspace)
            party_models.append(result[1][0])
        self.parties = party_models
    
    def train(self, epochs):
        for epoch in range(epochs):
            if(epoch%9==0):
                print("processing round: "+str(epoch+1))
            gradlist = []
            time.sleep(0.1)
            for i in range(len(self.vms)):
                jobid = newguid()
                pushdata(self.vms[i], jobid, self.fns['get_grad'], [], [self.parties[i]], self.workspace)
                execjob(self.vms[i], self.fns['get_grad'], jobid)
                result = pulldata(self.vms[i], jobid, self.fns['get_grad'], self.workspace)
                gradlist.append(result[0][0])
    
            jobid = newguid()
            pushdata(self.vmagg, jobid, self.fns['agg'], [gradlist], [], self.workspace)
            execjob(self.vmagg, self.fns['agg'], jobid)
            result = pulldata(self.vmagg, jobid, self.fns['agg'], self.workspace)
            newgrad = result[0][0]
    
            for i in range(len(self.vms)):
                jobid = newguid()
                pushdata(self.vms[i], jobid, self.fns['update_grad'], [newgrad], [self.parties[i]], self.workspace)
                execjob(self.vms[i], self.fns['update_grad'], jobid)
                result = pulldata(self.vms[i], jobid, self.fns['update_grad'], self.workspace)
                self.parties[i] = result[1][0]
    
    def mae(self):
        mae_errs = []
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['mae'], [], [self.parties[i], self.data['X_test'][i], self.data['y_test'][i]], self.workspace)
            execjob(self.vms[i], self.fns['mae'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['mae'], self.workspace)
            mae_errs.append(result[0][0])
        return mae_errs
            
    def predict(self, all_inputs):
        preds = []
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['test'], [all_inputs], [self.parties[i]], self.workspace)
            execjob(self.vms[i], self.fns['test'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['test'], self.workspace)
            preds.append(result[0][0])
        
        return preds
