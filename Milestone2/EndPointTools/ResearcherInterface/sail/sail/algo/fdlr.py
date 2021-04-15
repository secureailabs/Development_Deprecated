from .fdcore import Algorithm
from ..core import newguid, pushdata, pulldata, pushfn, execjob, registerfn
import torch.nn as nn
import optuna
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
        fndict['init_submodel'] = registerfn("fn_modelinit.py", 2, 2, 0, 1)[0]
        fndict['get_grad'] = registerfn("fn_getGrad.py", 0, 1, 1, 0)[0]
        fndict['update_grad'] = registerfn("fn_updateGradients.py", 1, 1, 0, 1)[0]
        fndict['agg'] = registerfn("fn_agg.py", 1, 0, 1, 0)[0]
        fndict['test'] = registerfn("fn_test.py", 1, 1, 1, 0)[0]
        fndict['mae'] = registerfn("fn_mae.py", 0, 3, 2, 0)[0]
        fndict['cross_val_score'] = registerfn("fn_cross_val_score.py", 3, 2, 1, 0)[0]
        
        return fndict
    
    def initvms(self):
        for vm in self.vms:
            for key in self.fns:
                pushfn(vm, self.fns[key])
        pushfn(self.vmagg, self.fns['agg'])
    
    def initmodel(self, dimx, dimy, lr):
        self.model = nn.Linear(dimx, dimy)
        party_models = []
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['init_submodel'], [self.model, lr], [self.data['X_train'][i], self.data['y_train'][i]], self.workspace)
            execjob(self.vms[i], self.fns['init_submodel'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['init_submodel'], self.workspace)
            party_models.append(result[1][0])
        self.parties = party_models
    
    def fit(self, epochs):
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
    
    def mae(self, X, y):
        mae_errs = []
        ele_len = []
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['mae'], [], [self.parties[i], X[i], y[i]], self.workspace)
            execjob(self.vms[i], self.fns['mae'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['mae'], self.workspace)
            mae_errs.append(result[0][0])
            ele_len.append(result[0][1])
        sumitem = 0
        sumerr = 0
        for i in range(len(self.vms)):
            sumerr += mae_errs[i]*ele_len[i]
            sumitem+= ele_len[i]
        return sumerr/sumitem
            
    def predict(self, all_inputs):
        preds = []
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['test'], [all_inputs], [self.parties[i]], self.workspace)
            execjob(self.vms[i], self.fns['test'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['test'], self.workspace)
            preds.append(result[0][0])
        
        return preds
    
    # def score(self, score, cv):
    #     scores = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         pushdata(self.vms[i], jobid, self.fns['cross_val_score'], [score, cv, self.model], [self.data['X_train'][i], self.data['y_train'][i]], self.workspace)
    #         execjob(self.vms[i], self.fns['cross_val_score'], jobid)
    #         result = pulldata(self.vms[i], jobid, self.fns['cross_val_score'], self.workspace)
    #         scores.append(result[0][0])
    #     return sum(scores)/len(scores)

    def score(self):
        scores = self.mae(self.data['X_train'], self.data['y_train'])
        return sum(scores)/len(scores)