from ..core import newguid, pushdata, pulldata, submitjob, setparameter, queryresult

class RemoteSeries:
    def __init__(self, vm, data_id, fns):
        self.vm = vm
        self.data_id = data_id
        self.fns = fns
    
    def __sub__(self, other):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['series_sub'], [self.data_id, other.data_id])
        submitjob(self.vm, self.fns['series_sub'], jobid)
        pulldata(self.vm, jobid, self.fns['series_sub'])
        result = queryresult(jobid, self.fns['series_sub'])
        return result[0]
    
    def __add__(self, other):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['series_add'], [self.data_id, other.data_id])
        submitjob(self.vm, self.fns['series_add'], jobid)
        pulldata(self.vm, jobid, self.fns['series_add'])
        result = queryresult(jobid, self.fns['series_add'])
        return result[0]
    
    def __getattr__(self, attr):
        jobid = newguid()
        inputs = pushdata(self.vm, [attr])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['getattr'], inputs)
        submitjob(self.vm, self.fns['getattr'], jobid)
        pulldata(self.vm, jobid, self.fns['getattr'])
        result = queryresult(jobid, self.fns['getattr'])
        return result[0]
    
    def astype(self, dtype, copy=True, errors='raise'):
        jobid = newguid()
        inputs = pushdata(self.vm, [dtype, copy, errors])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_astype'], inputs)
        submitjob(self.vm, self.fns['series_astype'], jobid)
        pulldata(self.vm, jobid, self.fns['series_astype'])
        result = queryresult(jobid, self.fns['series_astype'])
        return result[0]
    
    def map(self, arg, na_action=None):
        jobid = newguid()
        inputs = pushdata(self.vm, [arg, na_action])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_map'], inputs)
        submitjob(self.vm, self.fns['series_map'], jobid)
        pulldata(self.vm, jobid, self.fns['series_map'])
        result = queryresult(jobid, self.fns['series_map'])
        return result[0]
    
    def to_datetime(self, arg, errors='raise', dayfirst=False, yearfirst=False, utc=None, format=None, exact=True, unit=None, infer_datetime_format=False, origin='unix', cache=True):
        jobid = newguid()
        inputs = pushdata(self.vm, [errors, dayfirst, yearfirst, utc, format, exact, unit, infer_datetime_format, origin, cache])
        inputs.append(arg)
        setparameter(self.vm, jobid, self.fns['to_datetime'], inputs)
        submitjob(self.vm, self.fns['to_datetime'], jobid)
        pulldata(self.vm, jobid, self.fns['to_datetime'])
        result = queryresult(jobid, self.fns['to_datetime'])
        self.data_id = result[0]
    
    def mean(self, axis=None, skipna=None, level=None, numeric_only=None, **kwargs):
        jobid = newguid()
        inputs = pushdata(self.vm, [axis, skipna, level, numeric_only, kwargs])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_mean'], inputs)
        submitjob(self.vm, self.fns['series_mean'], jobid)
        pulldata(self.vm, jobid, self.fns['series_mean'])
        result = queryresult(jobid, self.fns['series_mean'])
        return result[0]