from ..core import newguid, pushdata, pulldata, submitjob, setparameter, queryresult

class _Group:
    def __init__(self, vm, group_id, fns):
        self.vm = vm
        self.data_id = group_id
        self.fns = fns
    
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
        return _Group(self.vm, result[0], self.fns)

    def __setitem__(self, key, value):
        jobid = newguid()
        inputs = pushdata(self.vm, [key, value])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['setitem'], inputs)
        submitjob(self.vm, self.fns['setitem'], jobid)
        pulldata(self.vm, jobid, self.fns['setitem'])
        result = queryresult(jobid, self.fns['setitem'])
        self.data_id = result[0]
        #return result[0]
    
    def __iter__(self):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['iter'], [self.data_id])
        submitjob(self.vm, self.fns['iter'], jobid)
        pulldata(self.vm, jobid, self.fns['iter'])
        result = queryresult(jobid, self.fns['iter'])
        self.it = result[0]
    
    def __next__(self):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['next'], [self.it])
        submitjob(self.vm, self.fns['next'], jobid)
        pulldata(self.vm, jobid, self.fns['next'])
        result = queryresult(jobid, self.fns['next'])
        return result[0]
    
    def agg(self, arg, *args, **kwargs):
        jobid = newguid()
        inputs = pushdata(self.vm, [arg, args, kwargs])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['groupby_agg'], inputs)
        submitjob(self.vm, self.fns['groupby_agg'], jobid)
        pulldata(self.vm, jobid, self.fns['groupby_agg'])
        result = queryresult(jobid, self.fns['groupby_agg'])
        return result[0]
    
    # def min(self, numeric_only=False, min_count=-1):
    #     jobid = newguid()
    #     inputs = pushdata(self.vm, [numeric_only, min_count])
    #     inputs.append(self.data_id)
    #     setparameter(self.vm, jobid, self.fns['groupby_min'], inputs)
    #     submitjob(self.vm, self.fns['groupby_min'], jobid)
    #     pulldata(self.vm, jobid, self.fns['groupby_min'])
    #     result = queryresult(jobid, self.fns['groupby_min'])
    #     return result[0]
    
    def diff(self, axis = 0, periods = 1):
        jobid = newguid()
        inputs = pushdata(self.vm, [axis, periods])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['groupby_diff'], inputs)
        submitjob(self.vm, self.fns['groupby_diff'], jobid)
        pulldata(self.vm, jobid, self.fns['groupby_diff'])
        result = queryresult(jobid, self.fns['groupby_diff'])
        return result[0]
    
    def cumsum(self, axis=0, *args, **kwargs):
        jobid = newguid()
        inputs = pushdata(self.vm, [axis, args, kwargs])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['groupby_cumsum'], inputs)
        submitjob(self.vm, self.fns['groupby_cumsum'], jobid)
        pulldata(self.vm, jobid, self.fns['groupby_cumsum'])
        result = queryresult(jobid, self.fns['groupby_cumsum'])
        return result[0]
    
    def first(self, numeric_only=False, min_count=-1):
        jobid = newguid()
        inputs = pushdata(self.vm, [numeric_only, min_count])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['groupby_first'], inputs)
        submitjob(self.vm, self.fns['groupby_first'], jobid)
        pulldata(self.vm, jobid, self.fns['groupby_first'])
        result = queryresult(jobid, self.fns['groupby_first'])
        return result[0]
    
    #how to pickle lambda functions?
    def apply(self, func, *args, **kwargs):
        jobid = newguid()
        inputs = pushdata(self.vm, [func, args, kwargs])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['groupby_apply'], inputs)
        submitjob(self.vm, self.fns['groupby_apply'], jobid)
        pulldata(self.vm, jobid, self.fns['groupby_apply'])
        result = queryresult(jobid, self.fns['groupby_apply'])
        return result[0]