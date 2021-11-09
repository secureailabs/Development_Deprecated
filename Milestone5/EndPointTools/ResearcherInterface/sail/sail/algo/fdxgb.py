from .fdcore import Algorithm
from ..core import newguid, pushdata, pulldata, pushfn, execjob, registerfn
import xgboost as xgb
import numpy as np
import pickle

class fdxgb(Algorithm):
    def __init__(self, vms, vmagg, data, workspace):
        super().__init__(vms, vmagg, data, workspace)
        self.fns = self.setfn()
        self.model = 0
        self.params_hash_functions =0
        self.hash_tables = 0
        self.initvms()
    
    def setfn(self):
        fndict = {}
        fndict['handlehash'] = registerfn("sub_handle_hashfn.py", 1, 1, 1, 0)[0]
        fndict['train_init'] = registerfn("sub_training_init.py", 3, 2, 1, 0)[0]
        fndict['train_update'] = registerfn("sub_training_one_boost.py", 4, 2, 1, 0)[0]
        fndict['test'] = registerfn("test.py", 1, 3, 3, 0)[0]
        return fndict
    
    def initvms(self):
        for vm in self.vms:
            for key in self.fns:
                pushfn(vm, self.fns[key])
        pushfn(self.vmagg, self.fns['test'])
    
    def init_model(self, params, feature_dim):
        dtrain_all = xgb.DMatrix(np.asarray([np.zeros(feature_dim)]), label=np.asarray([0]))
        model = xgb.Booster(params, [dtrain_all])
        self.trypickle(model)
        return model
    
    def gen_hashfn(self, num_dim, r, L, mu, sigma):
        hash_functions = []
        for l in range(L):
            hash_function = []
            np.random.seed(0)
            a = np.random.normal(mu, sigma, num_dim)
            b = np.random.uniform(0, r, 1)
            hash_function.append(a)
            hash_function.append(b)
            hash_functions.append(hash_function)
        params_hash_functions = {}
        params_hash_functions['num_dim'] = num_dim
        params_hash_functions['r'] = r
        params_hash_functions['L'] = L
        params_hash_functions['mu'] = mu
        params_hash_functions['sigma'] = sigma
        params_hash_functions['hash_functions'] = hash_functions
        
        self.params_hash_functions = params_hash_functions
    
    def gen_hashtables(self):
        all_hashes = []
        all_counters = []

        for i in range(len(self.vms)):
            jobid = newguid()
            pushdata(self.vms[i], jobid, self.fns['handlehash'], [self.params_hash_functions], [self.data['X'][i]], self.workspace)
            execjob(self.vms[i], self.fns['handlehash'], jobid)
            result = pulldata(self.vms[i], jobid, self.fns['handlehash'], self.workspace)
            all_hashes.append(result[0][0]['hash_values'])
            all_counters.append(result[0][0]['counters'])
        
        hash_tables = []
        for m in range(len(self.vms)):
            hash_table = np.zeros((len(all_hashes[m]),len(self.vms))) #For each party, build a hash table of dim numInstances x numParties.
            for i in range( len(all_hashes[m]) ):#Select an instance x_i in m party
                for j in range(len(self.vms)):  #Select parties to find global instances from
                    if j == m:
                        hash_table[i][j] = i
                    else:
                        instance_hash_counts = {}
                        for k in range(len(all_counters[j])):   #Select instance k in other parties
                            instance_hash_counts[k] = 0
                            for value in all_counters[m][i].keys():  #Iterate through hash values of x_i^m
                                if all_counters[j][k][value] != 0:
                                    instance_hash_counts[k] += min(all_counters[j][k][value], all_counters[m][i][value])
                        hash_table[i][j] = max(instance_hash_counts, key=instance_hash_counts.get)
            hash_tables.append(hash_table)
        
        self.hash_tables = hash_tables
    
    def trypickle(self, model):
        pickle.dump(model, open("test.pkl", 'wb'))
    
    def train(self, model):
        for i in range(20):
            print("Training round %d"%i)
            training_node = i % len(self.vms)
            local_gradients=[]
            for j in range(len(self.vms)):
                train = {}
                train['training'] = training_node
                train['node_id'] = j
                train['num_parties'] = len(self.vms)

                #self.trypickle(model)

                jobid = newguid()
                pushdata(self.vms[j], jobid, self.fns['train_init'], [train, model, self.hash_tables], [self.data['X'][j], self.data['y'][j]], self.workspace)
                execjob(self.vms[j], self.fns['train_init'], jobid)
                result = pulldata(self.vms[j], jobid, self.fns['train_init'], self.workspace)
                local_gradients.append(result[0][0])
    
            for j in range(len(self.vms)):
                train = {}
                train['training'] = training_node
                train['node_id'] = j
                train['num_parties'] = len(self.vms)

                jobid = newguid()
                pushdata(self.vms[j], jobid, self.fns['train_update'], [train, model, local_gradients, self.hash_tables], [self.data['X'][j], self.data['y'][j]], self.workspace)
                execjob(self.vms[j], self.fns['train_update'], jobid)
                result = pulldata(self.vms[j], jobid, self.fns['train_update'], self.workspace)
                model = result[0][0]
        return model
    
    def test(self, model):

        jobid = newguid()
        print("pushing data")
        pushdata(self.vmagg, jobid, self.fns['test'], [model], [self.data['X_test'], self.data['y_test'], self.data['df_test']], self.workspace)
        print("exec job")
        execjob(self.vmagg, self.fns['test'], jobid)
        print("pulldata")
        result=pulldata(self.vmagg, jobid, self.fns['test'], self.workspace)
        conf_mat = result[0][0]
        errors = result[0][1]
        fig = result[0][2]
        
        return conf_mat, errors, fig
