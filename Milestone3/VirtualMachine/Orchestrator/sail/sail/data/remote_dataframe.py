from ..core import newguid, pushdata, pulldata, submitjob, setparameter, queryresult
from .remote_series import RemoteSeries

class RemoteDataFrame:
    def __init__(self, vm, data_id, fnsdict):
        self.vm = vm
        self.data_id = data_id
        self.fns = fnsdict
    
    @property
    def loc(self):
        return _Loc(self.vm, self.data_id, self.fns)
    
    @property
    def dt(self):
        return _DT(self.vm, self.data_id, self.fns)
    
    def __getattr__(self, attr):
        jobid = newguid()
        inputs = pushdata(self.vm, [attr])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['getattr'], inputs)
        submitjob(self.vm, self.fns['getattr'], jobid)
        pulldata(self.vm, jobid, self.fns['getattr'])
        result = queryresult(jobid, self.fns['getattr'])
        return result[0]
    
    def __delitem__(self, key):
        jobid = newguid()
        inputs = pushdata(self.vm, [key])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['delietm'], inputs)
        submitjob(self.vm, self.fns['delitem'], jobid)
        pulldata(self.vm, jobid, self.fns['delitem'])
        result = queryresult(jobid, self.fns['delitem'])
        self.data_id = result[0]
        #return result[0]

    def __getitem__(self, key):
        jobid = newguid()
        inputs = pushdata(self.vm, [key])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['getitem'], inputs)
        submitjob(self.vm, self.fns['getitem'], jobid)
        pulldata(self.vm, jobid, self.fns['getitem'])
        result = queryresult(jobid, self.fns['getitem'])
        #return result[0]
        return RemoteSeries(self.vm, result[0], self.fns)

    def __setitem__(self, key, value):
        jobid = newguid()
        inputs = pushdata(self.vm, [key, value])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['setietm'], inputs)
        submitjob(self.vm, self.fns['setitem'], jobid)
        pulldata(self.vm, jobid, self.fns['setitem'])
        result = queryresult(jobid, self.fns['setitem'])
        self.data_id = result[0]
        #return result[0]
    
    def describe(self, percentiles=None, include=None, exclude=None, datetime_is_numeric=False):
        jobid = newguid()
        inputs = pushdata(self.vm, [percentiles, include, exclude, datetime_is_numeric])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['describe'], inputs)
        submitjob(self.vm, self.fns['describe'], jobid)
        pulldata(self.vm, jobid, self.fns['describe'])
        result = queryresult(jobid, self.fns['describe'])
        return result[0]
    
    def drop(self, labels=None, axis=0, index=None, columns=None, level=None, inplace=False, errors='raise'):
        jobid = newguid()
        inputs = pushdata(self.vm, [labels, axis, index, columns, level, inplace, errors])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['drop'], inputs)
        submitjob(self.vm, self.fns['drop'], jobid)
        pulldata(self.vm, jobid, self.fns['drop'])
        result = queryresult(jobid, self.fns['drop'])
        return result[0]
    
    def dropna(self, axis=0, how='any', thresh=None, subset=None, inplace=False):
        jobid = newguid()
        inputs = pushdata(self.vm, [axis, how, thresh, subset, inplace])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['dropna'], inputs)
        submitjob(self.vm, self.fns['dropna'], jobid)
        pulldata(self.vm, jobid, self.fns['dropna'])
        result = queryresult(jobid, self.fns['dropna'])
        return result[0]
    
    def merge(self, right, how='inner', on=None, left_on=None, right_on=None, left_index=False, right_index=False, sort=False, suffixes=('_x', '_y'), copy=True, indicator=False, validate=None):
        if self.vm != right.vm:
            print("Invalid operation: merging must happen on the same VM")
            return
        
        jobid = newguid()
        inputs = pushdata(self.vm, [how, on, left_on, right_on, left_index, right_index, sort, suffixes, copy, indicator, validate])
        inputs.extend([self.data_id, right.data_id])
        setparameter(self.vm, jobid, self.fns['merge'], inputs)
        submitjob(self.vm, self.fns['merge'], jobid)
        pulldata(self.vm, jobid, self.fns['merge'])
        result = queryresult(jobid, self.fns['merge'])
        return result[0]
    
    def get_dummies(self, data, prefix=None, prefix_sep='_', dummy_na=False, columns=None, sparse=False, drop_first=False, dtype=None):
        jobid = newguid()
        inputs = pushdata(self.vm, [prefix, prefix_sep, dummy_na, columns, sparse, drop_first, dtype])
        inputs.append(data)
        setparameter(self.vm, jobid, self.fns['get_dummies'], inputs)
        submitjob(self.vm, self.fns['get_dummies'], jobid)
        pulldata(self.vm, jobid, self.fns['get_dummies'])
        result = queryresult(jobid, self.fns['get_dummies'])
        return result[0]
    
    def groupby(self, by=None, axis=0, level=None, as_index=True, sort=True, group_keys=True, observed=False, dropna=True):
        jobid = newguid()
        inputs = pushdata(self.vm, [by, axis, level, as_index, sort, group_keys, observed, dropna])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['groupby'], inputs)
        submitjob(self.vm, self.fns['groupby'], jobid)
        pulldata(self.vm, jobid, self.fns['groupby'])
        result = queryresult(jobid, self.fns['groupby'])
        return _Group(self.vm, result[0], self.fns)
    
    def query(self, expr):
        jobid = newguid()
        inputs = pushdata(self.vm, [expr])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['rdf_query'], inputs)
        submitjob(self.vm, self.fns['rdf_query'], jobid)
        pulldata(self.vm, jobid, self.fns['rdf_query'])
        result = queryresult(jobid, self.fns['rdf_query'])
        return result[0]

class _Loc:
    def __init__(self, vm, data_id, fns):
        self.vm = vm
        self.data_id = data_id
        self.fns = fns

    def __delitem__(self, key):
        jobid = newguid()
        inputs = pushdata(self.vm, [key])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['loc_delietm'], inputs)
        submitjob(self.vm, self.fns['loc_delitem'], jobid)
        pulldata(self.vm, jobid, self.fns['loc_delitem'])
        result = queryresult(jobid, self.fns['loc_delitem'])
        self.data_id = result[0]
        #return result[0]

    def __getitem__(self, key):
        jobid = newguid()
        inputs = pushdata(self.vm, [key])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['loc_getietm'], inputs)
        submitjob(self.vm, self.fns['loc_getitem'], jobid)
        pulldata(self.vm, jobid, self.fns['loc_getitem'])
        result = queryresult(jobid, self.fns['loc_getitem'])
        return result[0]

    def __setitem__(self, key, value):
        jobid = newguid()
        inputs = pushdata(self.vm, [key, value])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['loc_setietm'], inputs)
        submitjob(self.vm, self.fns['loc_setitem'], jobid)
        pulldata(self.vm, jobid, self.fns['loc_setitem'])
        result = queryresult(jobid, self.fns['loc_setitem'])
        self.data_id = result[0]

class _Group:
    def __init__(self, vm, group_id, fns):
        self.vm = vm
        self.group_id = group_id
        self.fns = fns
    
    def agg(self, arg, *args, **kwargs):
        jobid = newguid()
        inputs = pushdata(self.vm, [arg, args, kwargs])
        inputs.append(self.group_id)
        setparameter(self.vm, jobid, self.fns['groupby'], inputs)
        submitjob(self.vm, self.fns['groupby'], jobid)
        pulldata(self.vm, jobid, self.fns['groupby'])
        result = queryresult(jobid, self.fns['groupby'])
        return result[0]

class _DT:
    def __init__(self, vm, dt_id, fns):
        self.vm = vm
        self.dt_id = dt_id
        self.fns = fns

    def __getattribute__(self, attr):
        jobid = newguid()
        inputs = pushdata(self.vm, [attr])
        inputs.append(self.dt_id)
        setparameter(self.vm, jobid, self.fns['dt_getattribute'], inputs)
        submitjob(self.vm, self.fns['dt_getattribute'], jobid)
        pulldata(self.vm, jobid, self.fns['dt_getattribute'])
        result = queryresult(jobid, self.fns['dt_getattribute'])
        return RemoteSeries(self.vm, result[0], self.fns)