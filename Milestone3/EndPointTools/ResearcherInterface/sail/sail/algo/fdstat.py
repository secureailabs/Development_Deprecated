from .fdcore import Algorithm
from ..core import newguid, pushdata, pulldata, pushfn, execjob, registerfn

class fdstat(Algorithm):
    def __init__(self, vms, vmagg, data, workspace):
        super().__init__(vms, vmagg, data, workspace)
        self.fns = self.setfn()
        self.initvms()
        self.get_data()
    
    def setfn(self):
        fn1 = registerfn("fn_data_gen.py", 0, 0, 0, 1)
        fn2 = registerfn("fn_fd_mean.py", 1, 1, 2, 0)
        fn3 = registerfn("fn_fd_sdeviation.py", 2, 1, 2, 0)
        fn4 = registerfn("fn_fd_min.py", 1, 1, 1, 0)
        fn5 = registerfn("fn_fd_max.py", 1, 1, 1, 0)
        fn6 = registerfn("stand_fd.py", 3, 1, 0, 1)
        fn7 = registerfn("normal_fd.py", 3, 1, 0, 1)
        fn8 = registerfn("fn_logtrans.py", 1, 1, 0, 1)
        
        fndict = {}
        fndict['data_gen'] = fn1[0]
        fndict['mean'] = fn2[0]
        fndict['std'] = fn3[0]
        fndict['min'] = fn4[0]
        fndict['max'] = fn5[0]
        fndict['std_trans'] = fn6[0]
        fndict['norm_trans'] = fn7[0]
        fndict['log_trans'] = fn8[0]
        
        return fndict
    
    def initvms(self):
        for vm in self.vms:
            for key in self.fns:
                pushfn(vm, self.fns[key])
    
    def get_data(self):
        data = []
        for vm in self.vms:
            jobid = newguid()
            execjob(vm, self.fns['data_gen'] , jobid)
            result = pulldata(vm, jobid, self.fns['data_gen'] , self.workspace)
            data.append(result[1][0])
        self.data = data
                
    def mean(self, col_num):
        sizelist = []
        meanlist = []
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['mean'], [col_num], [self.data[i]], self.workspace)
            execjob(self.vms[i], self.fns['mean'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['mean'], self.workspace)
            sizelist.append(result[0][0])
            meanlist.append(result[0][1])
        
        summean = 0
        for i in range(len(self.vms)):
            summean += meanlist[i]*sizelist[i]
            mean = summean/sum(sizelist)
        
        return mean
    
    def standard_deviation(self, col_num, mean):
        sumdevlist = []
        sizelist = []
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['std'], [col_num, mean], [self.data[i]], self.workspace)
            execjob(self.vms[i], self.fns['std'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['std'], self.workspace)
            sumdevlist.append(result[0][0])
            sizelist.append(result[0][1])
        stdev = sum(sumdevlist)/sum(sizelist)
        return stdev
    
    def limit(self, col_num):
        minlist = []
        maxlist = []
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['min'], [col_num], [self.data[i]], self.workspace)
            execjob(self.vms[i], self.fns['min'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['min'], self.workspace)
            minlist.append(result[0][0])
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['max'], [col_num], [self.data[i]], self.workspace)
            execjob(self.vms[i], self.fns['max'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['max'], self.workspace)
            maxlist.append(result[0][0])
        minval = min(minlist)
        maxval = max(maxlist)
        
        return minval, maxval
        
    def log_transform(self, col_num):
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['log_trans'], [col_num], [self.data[i]], self.workspace)
            execjob(self.vms[i], self.fns['log_trans'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['log_trans'], self.workspace)
   
    def std_transform(self, col_num, mean, stdev):
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['std_trans'], [col_num, mean, stdev], [self.data[i]], self.workspace)
            execjob(self.vms[i], self.fns['std_trans'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['std_trans'], self.workspace)
            
    def norm_transform(self, col_num, minval, maxval):
        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['norm_trans'], [col_num, minval, maxval], [self.data[i]], self.workspace)
            execjob(self.vms[i], self.fns['norm_trans'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['norm_trans'], self.workspace)
