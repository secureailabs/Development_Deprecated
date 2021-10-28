from ..core import newguid, pushdata, pulldata, submitjob, pushsafeobj, setparameter, queryresult, queryresults_parallel
import xgboost as xgb
from sklearn.base import BaseEstimator
import numpy as np
import pickle

class fdxgb(BaseEstimator):
    def __init__(self, vms, params=None, feature_num = 0, hash_r = 4.0, hash_mu = 0.0, hash_sigma = 1.0):
        self.vms = vms
        self.params = params
        self.feature_num = feature_num
        self.hash_r = hash_r
        self.hash_mu = hash_mu
        self.hash_sigma = hash_sigma
        self.params_hash_functions =0
        self.hash_tables = 0
        #self.model = self.init_model()
        self.fns = self.setfn()
        self.initvms()
    
    def setfn(self):
        fndict = {}
        fndict['preprocess'] = "53D9F7CF2CE5428D9ECF6C7C00A9C284"
        fndict['test_preprocess'] = "217443048261497ABCB2E058E9FB622E"
        fndict['handlehash'] = "B804C0767CCB4B01A6BAF5A2F782BD31"
        fndict['train_init'] = "D708DABA545346409FB835560140E882"
        fndict['train_update'] = "BE9644CBB2DC4FCD9FD2AF1733550A7E"
        fndict['conf_mat'] = "A5E177C364604D759518CF618CF2F9C1"
        fndict['shap'] = "CA459C663543457FAA0F8F2444D61728"
        fndict['accuracy_score']="BD9552F72CA94350BDF72207B6E33080"
        fndict['aucpr']="D692FF19A7F0416398C9FB28EE36C3A9"
        fndict['test_compare'] = "18DE7CAFB31C410190A04693667CB26B"
        return fndict
    
    def initvms(self):
        for vm in self.vms:
            for key in self.fns:
                pushsafeobj(vm, self.fns[key])

    def get_params(self, deep = True):
        params = super().get_params(deep)
        if self.params is None:
            self.params = {}
        self.params.update(params)
        return self.params
    
    def set_params(self, **params):
        for parameter, value in params.items():
            self.params[parameter] = value
    
    def set_mdoel(self, model):
        self.model = model

    def init_model(self):
        dtrain_all = xgb.DMatrix(np.asarray([np.zeros(self.feature_num)]), label=np.asarray([0]))
        model = xgb.Booster(self.params, [dtrain_all])
        self.trypickle(model)
        return model

    def data_preprocess(self, flag, data):
        res = []
        for i in range(len(self.vms)):
            jobid = newguid()
            inputs = pushdata(self.vms[i], [flag])
            inputs.extend(data[i])
            setparameter(self.vms[i], jobid, self.fns['preprocess'], inputs)
            submitjob(self.vms[i], self.fns['preprocess'], jobid)
            pulldata(self.vms[i], jobid, self.fns['preprocess'])
            result = queryresult(jobid, self.fns['preprocess'])
            res.append(result)
        return res

    def data_test_preprocess(self, flag, data):
        res = []
        for i in range(len(self.vms)):
            jobid = newguid()
            inputs = pushdata(self.vms[i], [flag])
            inputs.extend(data[i])
            setparameter(self.vms[i], jobid, self.fns['test_preprocess'], inputs)
            submitjob(self.vms[i], self.fns['test_preprocess'], jobid)
            pulldata(self.vms[i], jobid, self.fns['test_preprocess'])
            result = queryresult(jobid, self.fns['test_preprocess'])
            res.append(result)
        return res
    
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
    
    def gen_hashtables(self, X):
        all_hashes = []
        all_counters = []

        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [self.params_hash_functions])
            setparameter(self.vms[i], jobid, self.fns['handlehash'], [data_id[0], X[i]])
            submitjob(self.vms[i], self.fns['handlehash'], jobid)
            pulldata(self.vms[i], jobid, self.fns['handlehash'])
        result = queryresults_parallel(jobids, self.fns['handlehash'])
        for i in range(len(result)):
            all_hashes.append(result[i][0]['hash_values'])
            all_counters.append(result[i][0]['counters'])
        
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
    
    def train(self, model, X, y):
        num_of_trees = 0
        if "n_estimators" in self.params:
            num_of_trees = self.params["n_estimators"]
        else:    
            num_of_trees = 20
        for i in range(num_of_trees):
            print("Tree {0}".format(i+1))
            training_node = i % len(self.vms)
            local_gradients=[]
            jobids = []
            for j in range(len(self.vms)):
                train = {}
                train['training'] = training_node
                train['node_id'] = j
                train['num_parties'] = len(self.vms)

                #self.trypickle(model)

                jobid = newguid()
                jobids.append(jobid)
                #print("jobid: {}".format(jobid))
                data_id = pushdata(self.vms[j], [train, model])
                #print("data id: {}".format(data_id))
                setparameter(self.vms[j], jobid, self.fns['train_init'], [data_id[0], data_id[1], X[j], y[j]])
                submitjob(self.vms[j], self.fns['train_init'], jobid)
                pulldata(self.vms[j], jobid, self.fns['train_init'])
            results = queryresults_parallel(jobids, self.fns['train_init'])
            local_gradients = []
            for item in results:
                local_gradients.append(item[0])

            #print("first step done")
    
            for j in range(len(self.vms)):
                train = {}
                train['training'] = training_node
                train['node_id'] = j
                train['num_parties'] = len(self.vms)

                jobid = newguid()
                data_id = pushdata(self.vms[j], [train, model, local_gradients, self.hash_tables])
                setparameter(self.vms[j], jobid, self.fns['train_update'], [data_id[0], data_id[1], data_id[2], data_id[3], X[j], y[j]])
                submitjob(self.vms[j], self.fns['train_update'], jobid)
                pulldata(self.vms[j], jobid, self.fns['train_update'])
                model = queryresult(jobid, self.fns['train_update'])[0]
        return model
    
    def conf_mat(self, X, y):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [self.model])
            setparameter(self.vms[i],  jobid, self.fns['conf_mat'], [data_id[0], X[i], y[i]])
            submitjob(self.vms[i], self.fns['conf_mat'], jobid)
            pulldata(self.vms[i], jobid, self.fns['conf_mat'])
        results = queryresults_parallel(jobids, self.fns['conf_mat'])
        ret = []
        for item in results:
            ret.append(item[0])
        return ret
    
    def test_compare(self, X, y):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            setparameter(self.vms[i],  jobid, self.fns['test_compare'], [X[i], y[i]])
            submitjob(self.vms[i], self.fns['test_compare'], jobid)
            pulldata(self.vms[i], jobid, self.fns['test_compare'])
        results = queryresults_parallel(jobids, self.fns['test_compare'])
        return results

    def shap(self, df):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [self.model])
            setparameter(self.vms[i],  jobid, self.fns['shap'],  [data_id[0], df[i]])
            submitjob(self.vms[i], self.fns['shap'], jobid)
            pulldata(self.vms[i], jobid, self.fns['shap'])
        results = queryresults_parallel(jobids, self.fns['shap'])
        ret = []
        for item in results:
            ret.append(item[0])
        return ret
    
    def fit(self, X, y):
        L = min(40, self.feature_num-1)
        self.gen_hashfn(self.feature_num, self.hash_r, L, self.hash_mu, self.hash_sigma)
        self.gen_hashtables(X)
        self.model = self.init_model()
        model = self.train(self.model, X, y)
        return model
    
    def acc_score(self, X, y):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [self.model])
            setparameter(self.vms[i],  jobid, self.fns['accuracy_score'],  [data_id[0], X[i], y[i]])
            submitjob(self.vms[i], self.fns['accuracy_score'], jobid)
            pulldata(self.vms[i], jobid, self.fns['accuracy_score'])
        results = queryresults_parallel(jobids, self.fns['accuracy_score'])
        print(results)
        ret = []
        for item in results:
            ret.append(item[0])
        import statistics
        meanval = statistics.mean(ret)
        return meanval
    
    def aucpr_score(self, model, X, y):
        jobids = []
        p_thresholds = np.linspace(0, 1, num=100)
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [model, p_thresholds])
            setparameter(self.vms[i],  jobid, self.fns['aucpr'],  [data_id[0], data_id[1], X[i], y[i]])
            submitjob(self.vms[i], self.fns['aucpr'], jobid)
            pulldata(self.vms[i], jobid, self.fns['aucpr'])
        results = queryresults_parallel(jobids, self.fns['aucpr'])
        # print(results)
        # precision = []
        # recall = []
        # for i in range(100):
        #     tmp = [0,0,0,0]
        #     for j in range(len(self.vms)):
        #         tmp[0]+=results[j][0][i].ravel()[0]
        #         tmp[1]+=results[j][0][i].ravel()[1]
        #         tmp[2]+=results[j][0][i].ravel()[2]
        #         tmp[3]+=results[j][0][i].ravel()[3]
        #     prec = tmp[3]/(tmp[3]+tmp[1])
        #     reca = tmp[3]/(tmp[3]+tmp[2])
        #     precision.append(prec)
        #     recall.append(reca)
        
        #return precision, recall
        return results

    
    def cv_score(self, models, X, y):
        scores = []
        for i in range(len(models)):
            jobids = []
            for j in range(len(self.vms)):
                jobid = newguid()
                jobids.append(jobid)
                data_id = pushdata(self.vms[j], [models[i]])
                setparameter(self.vms[j],  jobid, self.fns['accuracy_score'],  [data_id[0], X[j][i], y[j][i]])
                submitjob(self.vms[j], self.fns['accuracy_score'], jobid)
                pulldata(self.vms[j], jobid, self.fns['accuracy_score'])
            results = queryresults_parallel(jobids, self.fns['accuracy_score'])
            ret = []
            for item in results:
                ret.append(item[0])
            import statistics
            meanval = statistics.mean(ret)
            scores.append(meanval)
        return scores


    def predict(self, X):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [self.model])
            setparameter(self.vms[i],  jobid, self.fns['predict'],  [data_id[0], X[i]])
            submitjob(self.vms[i], self.fns['predict'], jobid)
            pulldata(self.vms[i], jobid, self.fns['predict'])
        results = queryresults_parallel(jobids, self.fns['predict'])
        ret = []
        for item in results:
            ret.append(item[0])
        return ret