from ..core import newguid, pushdata, pulldata, submitjob, setparameter, queryresult
from .remote_series import RemoteSeries
from .remote_groupby import _Group

def get_data(vm, fnid):
    jobid = newguid()
    submitjob(vm, fnid, jobid)
    pulldata(vm, jobid, fnid)
    result = queryresult(jobid, fnid)
    return result[0]

class RemoteDataFrame:
    def __init__(self, vm, data_id, fnsdict):
        self.vm = vm
        self.data_id = data_id
        self.fns = fnsdict
    
    @property
    def loc(self):
        return _Loc(self.vm, self.data_id, self.fns)
    
    def __getattr__(self, attr):
        jobid = newguid()
        inputs = pushdata(self.vm, [attr])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['getattr'], inputs)
        submitjob(self.vm, self.fns['getattr'], jobid)
        pulldata(self.vm, jobid, self.fns['getattr'])
        result = queryresult(jobid, self.fns['getattr'])
        return result[0]
    
    # def __delitem__(self, key):
    #     jobid = newguid()
    #     inputs = pushdata(self.vm, [key])
    #     inputs.append(self.data_id)
    #     setparameter(self.vm, jobid, self.fns['delietm'], inputs)
    #     submitjob(self.vm, self.fns['delitem'], jobid)
    #     pulldata(self.vm, jobid, self.fns['delitem'])
    #     result = queryresult(jobid, self.fns['delitem'])
    #     self.data_id = result[0]
    #     #return result[0]

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
        setparameter(self.vm, jobid, self.fns['rdf_describe'], inputs)
        submitjob(self.vm, self.fns['rdf_describe'], jobid)
        pulldata(self.vm, jobid, self.fns['rdf_describe'])
        result = queryresult(jobid, self.fns['rdf_describe'])
        return result[0]
    
    def drop(self, labels=None, axis=0, index=None, columns=None, level=None, inplace=False, errors='raise'):
        jobid = newguid()
        inputs = pushdata(self.vm, [labels, axis, index, columns, level, inplace, errors])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['rdf_drop'], inputs)
        submitjob(self.vm, self.fns['rdf_drop'], jobid)
        pulldata(self.vm, jobid, self.fns['rdf_drop'])
        result = queryresult(jobid, self.fns['rdf_drop'])
        return result[0]
    
    def dropna(self, axis=0, how='any', thresh=None, subset=None, inplace=False):
        jobid = newguid()
        inputs = pushdata(self.vm, [axis, how, thresh, subset, inplace])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['rdf_dropna'], inputs)
        submitjob(self.vm, self.fns['rdf_dropna'], jobid)
        pulldata(self.vm, jobid, self.fns['rdf_dropna'])
        result = queryresult(jobid, self.fns['rdf_dropna'])
        return result[0]
    
    def merge(self, right, how='inner', on=None, left_on=None, right_on=None, left_index=False, right_index=False, sort=False, suffixes=('_x', '_y'), copy=True, indicator=False, validate=None):
        if self.vm != right.vm:
            print("Invalid operation: merging must happen on the same VM")
            return
        
        jobid = newguid()
        inputs = pushdata(self.vm, [how, on, left_on, right_on, left_index, right_index, sort, suffixes, copy, indicator, validate])
        inputs.extend([self.data_id, right.data_id])
        setparameter(self.vm, jobid, self.fns['rdf_merge'], inputs)
        submitjob(self.vm, self.fns['rdf_merge'], jobid)
        pulldata(self.vm, jobid, self.fns['rdf_merge'])
        result = queryresult(jobid, self.fns['rdf_merge'])
        return result[0]
    
    def groupby(self, by=None, axis=0, level=None, as_index=True, sort=True, group_keys=True, observed=False, dropna=True):
        jobid = newguid()
        inputs = pushdata(self.vm, [by, axis, level, as_index, sort, group_keys, observed, dropna])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['rdf_groupby'], inputs)
        submitjob(self.vm, self.fns['rdf_groupby'], jobid)
        pulldata(self.vm, jobid, self.fns['rdf_groupby'])
        result = queryresult(jobid, self.fns['rdf_groupby'])
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
    
    def sort_values(self, by, axis=0, ascending=True, inplace=False, kind='quicksort', na_position='last', ignore_index=False, key=None):
        jobid = newguid()
        inputs = pushdata(self.vm, [by, axis, ascending, inplace, kind, na_position, ignore_index, key])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['rdf_sort_values'], inputs)
        submitjob(self.vm, self.fns['rdf_sort_values'], jobid)
        pulldata(self.vm, jobid, self.fns['rdf_sort_values'])
        result = queryresult(jobid, self.fns['rdf_sort_values'])
        return result[0]
    
    def set_index(self, keys, drop=True, append=False, inplace=False, verify_integrity=False):
        jobid = newguid()
        inputs = pushdata(self.vm, [keys, drop, append, inplace, verify_integrity])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['rdf_set_index'], inputs)
        submitjob(self.vm, self.fns['rdf_set_index'], jobid)
        pulldata(self.vm, jobid, self.fns['rdf_set_index'])
        result = queryresult(jobid, self.fns['rdf_set_index'])
        return result[0]
    
    def reset_index(self, level=None, drop=False, inplace=False, col_level=0, col_fill=''):
        jobid = newguid()
        inputs = pushdata(self.vm, [level, drop, inplace, col_level, col_fill])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['rdf_reset_index'], inputs)
        submitjob(self.vm, self.fns['rdf_reset_index'], jobid)
        pulldata(self.vm, jobid, self.fns['rdf_reset_index'])
        result = queryresult(jobid, self.fns['rdf_reset_index'])
        return result[0]

class _Loc:
    def __init__(self, vm, data_id, fns):
        self.vm = vm
        self.data_id = data_id
        self.fns = fns

    # def __delitem__(self, key):
    #     jobid = newguid()
    #     inputs = pushdata(self.vm, [key])
    #     inputs.append(self.data_id)
    #     setparameter(self.vm, jobid, self.fns['loc_delietm'], inputs)
    #     submitjob(self.vm, self.fns['loc_delitem'], jobid)
    #     pulldata(self.vm, jobid, self.fns['loc_delitem'])
    #     result = queryresult(jobid, self.fns['loc_delitem'])
    #     self.data_id = result[0]
    #     #return result[0]

    def __getitem__(self, key):
        jobid = newguid()
        inputs = pushdata(self.vm, [key])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['getitem'], inputs)
        submitjob(self.vm, self.fns['getitem'], jobid)
        pulldata(self.vm, jobid, self.fns['getitem'])
        result = queryresult(jobid, self.fns['getitem'])
        return result[0]

    def __setitem__(self, key, value):
        jobid = newguid()
        inputs = pushdata(self.vm, [key, value])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['setitem'], inputs)
        submitjob(self.vm, self.fns['setitem'], jobid)
        pulldata(self.vm, jobid, self.fns['setitem'])
        result = queryresult(jobid, self.fns['setitem'])
        self.data_id = result[0]