from ..core import newguid, pushdata, pulldata, submitjob, pushsafeobj, setparameter, queryresults_parallel
import numpy as np

class DataFrameGroup:
    def __init__(self, vms, data_ids):
        self.vms = vms
        self.col_label = []
        self.data_ids = data_ids
        self.shape = []
        self.fns = self.setfn()
        self.initvms()
        self.import_data()
    
    def setfn(self):
        fndict = {}
        fndict['import'] = "32501FA4883C4B3493773D5F8E73D88E"
        fndict['col_des'] = "B2654A3C8D91436585EF158F0A661686"
        #fndict['describe'] = ""
        #fndict['std_trans'] = ""
        #fndict['norm_trans'] = ""
        #fndict['log_trans'] = ""
        fndict['label_encode'] = "56D864CD0B81459CB0540E364BB49D6F"
        #fndict['mean'] = ""
        #fndict['std'] = ""
        #fndict['minmax'] = ""
        #fndict['skew'] = ""
        fndict['train_test'] = "3301AFCAC86F412F8D52EB8CC17C0C2A"
        fndict['KFold_split'] = "EE604CB3DBBE477C81D2764230ABFBAE" #"1CE0BF1818B2467B87F7EBDF2D1E60A6"
        fndict['get_col'] = "63CBDC2EB27844928713DA54B9446FED"
        #fndict['onehot'] = ""
        #fndict['concat'] = ""
        fndict['drop'] = "8ECA2ACB296D433DA56BD471D0D66F0E"
        #fndict['dropna'] = ""
        #fndict['merge'] = ""
        #fndict['astype']= ""
        #fndict['apply']= ""
        #fndict['apply_and_change'] = ""
        #fndict['apply_and_append'] = ""
        #fndict['value_counts'] = ""
        #fndict['get_Dmat'] = ""
        fndict['dtypes'] = "409E935F2ED942C587228A7E32F18AC1"
        fndict['to_numpy'] = "9663AC9C5195448286384A73ECBD774C"
        fndict['to_df'] = "017C3BCA23F0407883E16F101842CF38"
        #fndict['private_get'] = ""
        #fndict['droprow'] = ""
        #fndict['pearson'] = ""
        return fndict

    def initvms(self):
        for vm in self.vms:
            for key in self.fns:
                pushsafeobj(vm, self.fns[key])
    
    def import_data(self):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            #print("jobid:{}".format(jobid))
            jobids.append(jobid)
            setparameter(self.vms[i], jobid, self.fns['import'], [self.data_ids[i]])
            submitjob(self.vms[i], self.fns['import'], jobid)
            pulldata(self.vms[i], jobid, self.fns['import'])
        result = queryresults_parallel(jobids, self.fns['import'])
        for i in range(len(self.vms)):
            self.shape.append(result[i][0])
            self.col_label.append(result[i][1])

    # def sample(self, vm, data):
    #     jobid = newguid()
    #     pushdata(vm, jobid, self.fns['sample'], [], [data], self.workspace)
    #     submitjob(vm, self.fns['sample'], jobid)
    #     pulldata(vm, jobid, self.fns['sample'], self.workspace)
    #     return jobid
    
    def dtypes(self):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            setparameter(self.vms[i], jobid, self.fns['dtypes'], [self.data_ids[i]])
            submitjob(self.vms[i], self.fns['dtypes'], jobid)
            pulldata(self.vms[i], jobid, self.fns['dtypes'])
        result = queryresults_parallel(jobids, self.fns['dtypes'])
        ret = []
        for item in result:
            ret.append(item[0])
        return ret
    
    def col_describe(self, col_id):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [col_id])
            setparameter(self.vms[i], jobid, self.fns['col_des'], [data_id[0], self.data_ids[i]])
            submitjob(self.vms[i], self.fns['col_des'], jobid)
            pulldata(self.vms[i], jobid, self.fns['col_des'])
        result = queryresults_parallel(jobids, self.fns['col_des'])
        ret = []
        for item in result:
            ret.append(item[0])
        return ret
    
    def describe(self, df, percentiles=None, include=None, exclude=None, datetime_is_numeric=False):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [percentiles, include, exclude, datetime_is_numeric])
            setparameter(self.vms[i], jobid, self.fns['describe'], [data_id[0], data_id[1], data_id[2], data_id[3], self.data_ids[i]])
            submitjob(self.vms[i], self.fns['describe'], jobid)
            pulldata(self.vms[i], jobid, self.fns['describe'])
        result = queryresults_parallel(jobids, self.fns['describe'])
        ret = []
        for item in result:
            ret.append(item[0])
        return ret

    # def log_transform(self, col_id, newcol_id):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['log_trans'], [col_id, newcol_id])
    #         setparameter(self.vms[i], jobid, self.fns['log_trans'], [data_id[0], data_id[1], self.data_ids[i]])
    #         submitjob(self.vms[i], self.fns['log_trans'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['log_trans'])
    #     queryresults_parallel(jobids, self.fns['log_trans'])
   
    # def std_transform(self, col_id, mean, stdev):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id=pushdata(self.vms[i], jobid, self.fns['std_trans'], [col_id, mean, stdev])
    #         setparameter(self.vms[i], jobid, self.fns['std_trans'], [data_id[0], data_id[1], data_id[2], self.data_ids[i]])
    #         submitjob(self.vms[i], self.fns['std_trans'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['std_trans'])
    #     queryresults_parallel(jobids, self.fns['std_trans'])
            
    # def norm_transform(self, col_id, df):
    #     results = self.limit(col_id, df)
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['norm_trans'], [col_id])
    #         setparameter(self.vms[i], jobid, self.fns['std_trans'], [data_id[0], df[i]])
    #         submitjob(self.vms[i], self.fns['norm_trans'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['norm_trans'])
    #     queryresults_parallel(jobids, self.fns['norm_trans'])
    
    def label_encode(self, col, df):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [col])
            setparameter(self.vms[i], jobid, self.fns['label_encode'], [data_id[0], df[i]])
            submitjob(self.vms[i], self.fns['label_encode'], jobid)
            pulldata(self.vms[i], jobid, self.fns['label_encode'])
        result = queryresults_parallel(jobids, self.fns['label_encode'])
        ret = []
        for item in result:
            ret.append(item[0])
        return ret
                
    # def mean(self, col_id, df):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['mean'], [col_id])
    #         setparameter(self.vms[i], jobid, self.fns['mean'], [data_id[0], df[i]])
    #         submitjob(self.vms[i], self.fns['mean'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['mean'])
    #     results = queryresults_parallel(jobids, self.fns['mean'])
        
    #     sumval = 0
    #     sumelement = 0
    #     for i in range(len(self.vms)):
    #         sumval += results[i]*self.shape[i][0]
    #         sumelement += self.shape[i][0]
    #     mean = sumval/sumelement
        
    #     return mean
    
    # def standard_deviation(self, col_id, mean, df):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['std'], [col_id, mean])
    #         setparameter(self.vms[i], jobid, self.fns['std'], [data_id[0], data_id[1], df[i]])
    #         submitjob(self.vms[i], self.fns['std'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['std'])
    #     results = queryresults_parallel(jobids, self.fns['std'])

    #     sumdev = 0
    #     sumelement = 0
    #     for i in range(len(self.vms)):
    #         sumdev += results[i]
    #         sumelement += self.shape[i][0]
    #     stddev = (sumdev/sumelement)**0.5
    #     return stddev
    
    # def limit(self, col_id, df):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['minmax'], [col_id])
    #         setparameter(self.vms[i], jobid, self.fns['minmax'], [data_id[0], df[i]])
    #         submitjob(self.vms[i], self.fns['minmax'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['minmax'])
    #     result = queryresults_parallel(jobids, self.fns['minmax'])
    #     return result

    # def skew(self, col_id, mean, std):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['skew'], [col_id, mean])
    #         setparameter(self.vms[i], jobid, self.fns['skew'], [data_id[0], data_id[1], self.df[i]])
    #         submitjob(self.vms[i], self.fns['skew'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['skew'])
    #     result = queryresults_parallel(jobids, self.fns['skew'])
    #     N = sum(i for i, _ in self.shape)
    #     skew = (N*(N-1))**0.5*sum(result)/N/(N-2)/(std**3)
    #     return skew
    
    def train_test_split(self, X, y, testsize, randomstate):
        X_train = []
        X_test = []
        y_train = []
        y_test = []
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [testsize, randomstate])
            setparameter(self.vms[i], jobid, self.fns['train_test'], [data_id[0], data_id[1], X[i], y[i]])
            submitjob(self.vms[i], self.fns['train_test'], jobid)
            pulldata(self.vms[i], jobid, self.fns['train_test'])
        result = queryresults_parallel(jobids, self.fns['train_test'])
        for i in range(len(self.vms)):
            X_train.append(result[i][0])
            X_test.append(result[i][1])
            y_train.append(result[i][2])
            y_test.append(result[i][3])
        return X_train, X_test, y_train, y_test
    
    def KFold_split(self, X, y, K=5):
        X_train = []
        X_test = []
        y_train = []
        y_test = []
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [K])
            setparameter(self.vms[i], jobid, self.fns['KFold_split'], [data_id[0], X[i], y[i]])
            submitjob(self.vms[i], self.fns['KFold_split'], jobid)
            pulldata(self.vms[i], jobid, self.fns['KFold_split'])
        result = queryresults_parallel(jobids, self.fns['KFold_split'])
        for i in range(len(self.vms)):
            X_train.append(result[i][0:5])
            y_train.append(result[i][5:10])
            X_test.append(result[i][10:15])
            y_test.append(result[i][15:20])
        return X_train, X_test, y_train, y_test
    
    def get_col(self, label, df):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [label])
            setparameter(self.vms[i], jobid, self.fns['get_col'], [data_id[0], df[i]])
            submitjob(self.vms[i], self.fns['get_col'], jobid)
            pulldata(self.vms[i], jobid, self.fns['get_col'])
        result = queryresults_parallel(jobids, self.fns['get_col'])
        ret = []
        for item in result:
            ret.append(item[0])
        return ret
    
    # def onehot(self, label, df):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['onehot'], [label[i]])
    #         setparameter(self.vms[i], jobid, self.fns['onehot'], [data_id[0], df[i]])
    #         submitjob(self.vms[i], self.fns['onehot'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['onehot'])
    #     result = queryresults_parallel(jobids, self.fns['onehot'])
    #     return result
    
    # def concat(self, df1, df2, axis):
    #     jobids =[]
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['concat'], [axis])
    #         setparameter(self.vms[i], jobid, self.fns['concat'], [data_id[0], df1[i], df2[i]])
    #         submitjob(self.vms[i], self.fns['concat'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['concat'])
    #     result = queryresults_parallel(jobids, self.fns['concat'])
    #     return result
    
    def drop(self, df, labels=None, axis=0, index=None, columns=None, level=None, inplace=False, errors='raise'):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [labels, axis, index, columns, level, inplace, errors])
            data_id.append(df[i])
            setparameter(self.vms[i], jobid, self.fns['drop'], data_id)
            submitjob(self.vms[i], self.fns['drop'], jobid)
            pulldata(self.vms[i], jobid, self.fns['drop'])
        result = queryresults_parallel(jobids, self.fns['drop'])
        ret = []
        for item in result:
            ret.append(item[0])
        return ret
    
    def dropna(self, df, axis=0, how='any', thresh=None, subset=None, inplace=False):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [axis, how, thresh, subset, inplace])
            setparameter(self.vms[i], jobid, self.fns['drop'], data_id.append(df[i]))
            submitjob(self.vms[i], self.fns['drop'], jobid)
            pulldata(self.vms[i], jobid, self.fns['drop'])
        result = queryresults_parallel(jobids, self.fns['drop'])
        ret = []
        for item in result:
            ret.append(item[0])
        return ret
    
    # def droprow(self, vm, index, df):
    #     jobid = newguid()
    #     data_id = pushdata(vm, jobid, self.fns['droprow'], [index])
    #     setparameter(self.vm, jobid, self.fns['droprow'], [data_id[0], df])
    #     submitjob(vm, self.fns['droprow'], jobid)
    #     pulldata(vm, jobid, self.fns['droprow'])
    #     result = queryresults_parallel([jobid], self.fns['droprow'])
    #     return result

    # def astype(self, mtype, df):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['astype'], [mtype])
    #         setparameter(self.vms[i], jobid, self.fns['astype'], [data_id[0], df[i]])
    #         submitjob(self.vms[i], self.fns['astype'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['astype'])
    #     result = queryresults_parallel(jobids, self.fns['astype'])
    #     return result

    # def apply(self, func, df):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['apply'], [func[i]])
    #         setparameter(self.vms[i], jobid, self.fns['apply'], [data_id[0], df[i]])
    #         submitjob(self.vms[i], self.fns['apply'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['apply'])
    #     result = queryresults_parallel(jobids, self.fns['apply'])
    #     return result
    
    # def apply_and_change(self, func, label, df):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['apply_and_change'], [func[i], label[i]])
    #         setparameter(self.vms[i], jobid, self.fns['apply_and_change'], [data_id[0], data_id[1], df[i]])
    #         submitjob(self.vms[i], self.fns['apply_and_change'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['apply_and_change'])
    #     result = queryresults_parallel(jobids, self.fns['apply_and_change'])
    #     return result

    # def apply_and_append(self, label, newlabel, df):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['apply_and_append'], [label[i], newlabel[i]])
    #         setparameter(self.vms[i], jobid, self.fns['apply_and_append'], [data_id[0], data_id[1], df[i]])
    #         submitjob(self.vms[i], self.fns['apply_and_append'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['apply_and_append'])
    #     result = queryresults_parallel(jobids, self.fns['apply_and_append'])
    #     return result

    # def value_counts(self, label, df):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         data_id = pushdata(self.vms[i], jobid, self.fns['value_counts'], [label[i]])
    #         setparameter(self.vms[i], jobid, self.fns['value_counts'], [data_id[0], df[i]])
    #         submitjob(self.vms[i], self.fns['value_counts'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['value_counts'])
    #     result = queryresults_parallel(jobids, self.fns['value_counts'])
    #     return result

    # def to_Dmatrix(self, X, y):
    #     jobids = []
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         jobids.append(jobid)
    #         setparameter(self.vms[i], jobid, self.fns['get_Dmat'], [X[i], y[i]])
    #         submitjob(self.vms[i], self.fns['get_Dmat'], jobid)
    #         pulldata(self.vms[i], jobid, self.fns['get_Dmat'])
    #     result = queryresults_parallel(jobids, self.fns['get_Dmat'])
    #     return result
    
    def to_numpy(self, df):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            setparameter(self.vms[i], jobid, self.fns['to_numpy'], [df[i]])
            submitjob(self.vms[i], self.fns['to_numpy'], jobid)
            pulldata(self.vms[i], jobid, self.fns['to_numpy'])
        result = queryresults_parallel(jobids, self.fns['to_numpy'])
        ret = []
        for item in result:
            ret.append(item[0])
        return ret

    def to_df(self, col, arr):
        jobids = []
        for i in range(len(self.vms)):
            jobid = newguid()
            jobids.append(jobid)
            data_id = pushdata(self.vms[i], [col])
            setparameter(self.vms[i], jobid, self.fns['to_df'], [data_id[0], arr[i]])
            submitjob(self.vms[i], self.fns['to_df'], jobid)
            pulldata(self.vms[i], jobid, self.fns['to_df'])
        result = queryresults_parallel(jobids, self.fns['to_df'])
        ret = []
        for item in result:
            ret.append(item[0])
        return ret
    
    # def private_intersect(self, vmid1, vmid2, df1, df2, col_id):
    #     x=0
    #     y=0

    #     jobid = newguid()
    #     pushdata(vmid1, jobid, self.fns['private_get'], [col_id], [df1], self.workspace)
    #     execjob(vmid1, self.fns['private_get'], jobid)
    #     result = pulldata(vmid1, jobid, self.fns['private_get'], self.workspace)
    #     x=result[0][0]

    #     jobid = newguid()
    #     pushdata(vmid2, jobid, self.fns['private_get'], [col_id], [df2], self.workspace)
    #     execjob(vmid2, self.fns['private_get'], jobid)
    #     result = pulldata(vmid2, jobid, self.fns['private_get'], self.workspace)
    #     y=result[0][0]

    #     result1 = np.where(np.in1d(x, y))[0]
    #     result2 = np.where(np.in1d(y, x))[0]
    #     return [result1, result2]

    # def pearson_corr(self, xlabel, ylabel, df):
    #     xmean = self.mean(xlabel, df)
    #     ymean = self.mean(ylabel, df)

    #     sumprod = 0
    #     sumsx = 0
    #     sumsy = 0
    #     for i in range(len(self.vms)):
    #         jobid = newguid()
    #         pushdata(self.vms[i], jobid, self.fns['pearson'], [xlabel, ylabel, xmean, ymean], [df[i]], self.workspace)
    #         execjob(self.vms[i], self.fns['pearson'], jobid)
    #         result = pulldata(self.vms[i], jobid, self.fns['pearson'], self.workspace)
    #         sumprod+=result[0][0]
    #         sumsx += result[0][1]
    #         sumsy += result[0][2]
    #     return sumprod/((sumsx*sumsy)**0.5)