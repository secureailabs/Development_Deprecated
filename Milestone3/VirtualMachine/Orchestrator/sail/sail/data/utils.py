from ..core import newguid, pushdata, pulldata, submitjob, setparameter, queryresult

class Utils:
    def __init__(self, vm, fns):
        self.vm = vm
        self.fns = fns
    
    def get_dummies(self, data, prefix=None, prefix_sep='_', dummy_na=False, columns=None, sparse=False, drop_first=False, dtype=None):
        jobid = newguid()
        inputs = pushdata(self.vm, [prefix, prefix_sep, dummy_na, columns, sparse, drop_first, dtype])
        inputs.append(data)
        setparameter(self.vm, jobid, self.fns['utils_get_dummies'], inputs)
        submitjob(self.vm, self.fns['utils_get_dummies'], jobid)
        pulldata(self.vm, jobid, self.fns['utils_get_dummies'])
        result = queryresult(jobid, self.fns['utils_get_dummies'])
        return result[0]

    def to_datetime(self, arg, errors='raise', dayfirst=False, yearfirst=False, utc=None, format=None, exact=True, unit=None, infer_datetime_format=False, origin='unix', cache=True):
        jobid = newguid()
        inputs = pushdata(self.vm, [errors, dayfirst, yearfirst, utc, format, exact, unit, infer_datetime_format, origin, cache])
        inputs.append(arg)
        setparameter(self.vm, jobid, self.fns['utils_to_datetime'], inputs)
        submitjob(self.vm, self.fns['utils_to_datetime'], jobid)
        pulldata(self.vm, jobid, self.fns['utils_to_datetime'])
        result = queryresult(jobid, self.fns['utils_to_datetime'])
        return result[0]
    
    def date_range(self, start=None, end=None, periods=None, freq=None, tz=None, normalize=False, name=None, closed=None, **kwargs):
        jobid = newguid()
        inputs = pushdata(self.vm, [start, end, periods, freq, tz, normalize, name, closed, kwargs])
        #inputs.append(arg)
        setparameter(self.vm, jobid, self.fns['utils_date_range'], inputs)
        submitjob(self.vm, self.fns['utils_date_range'], jobid)
        pulldata(self.vm, jobid, self.fns['utils_date_range'])
        result = queryresult(jobid, self.fns['utils_date_range'])
        return result[0]
    
    def to_numeric(self, arg, errors='raise', downcast=None):
        jobid = newguid()
        inputs = pushdata(self.vm, [errors, downcast])
        inputs.append(arg)
        setparameter(self.vm, jobid, self.fns['utils_to_numeric'], inputs)
        submitjob(self.vm, self.fns['utils_to_numeric'], jobid)
        pulldata(self.vm, jobid, self.fns['utils_to_numeric'])
        result = queryresult(jobid, self.fns['utils_to_numeric'])
        return result[0]
    
    def where(self, condition, x, y):
        jobid = newguid()
        inputs = pushdata(self.vm, [x, y])
        inputs.append(condition)
        setparameter(self.vm, jobid, self.fns['utils_where'], inputs)
        submitjob(self.vm, self.fns['utils_where'], jobid)
        pulldata(self.vm, jobid, self.fns['utils_where'])
        result = queryresult(jobid, self.fns['utils_where'])
        return result[0]
    
    def ravel(self, a, order='C'):
        jobid = newguid()
        inputs = pushdata(self.vm, [order])
        inputs.append(a)
        setparameter(self.vm, jobid, self.fns['utils_ravel'], inputs)
        submitjob(self.vm, self.fns['utils_ravel'], jobid)
        pulldata(self.vm, jobid, self.fns['utils_ravel'])
        result = queryresult(jobid, self.fns['utils_ravel'])
        return result[0]
