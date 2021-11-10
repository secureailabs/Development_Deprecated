from ..core import newguid, pushdata, pulldata, submitjob, setparameter, queryresult

class RemoteSeries:
    def __init__(self, vm, data_id, fns):
        self.vm = vm
        self.data_id = data_id
        self.fns = fns
    
    @property
    def dt(self):
        return _DT(self.vm, self.data_id, self.fns)
    
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
    
    def __le__(self, other):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['series_le'], [self.data_id, other.data_id])
        submitjob(self.vm, self.fns['series_le'], jobid)
        pulldata(self.vm, jobid, self.fns['series_le'])
        result = queryresult(jobid, self.fns['series_le'])
        return result[0]

    def __lt__(self, other):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['series_lt'], [self.data_id, other.data_id])
        submitjob(self.vm, self.fns['series_lt'], jobid)
        pulldata(self.vm, jobid, self.fns['series_lt'])
        result = queryresult(jobid, self.fns['series_lt'])
        return result[0]

    def __ge__(self, other):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['series_ge'], [self.data_id, other.data_id])
        submitjob(self.vm, self.fns['series_ge'], jobid)
        pulldata(self.vm, jobid, self.fns['series_ge'])
        result = queryresult(jobid, self.fns['series_ge'])
        return result[0]

    def __gt__(self, other):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['series_gt'], [self.data_id, other.data_id])
        submitjob(self.vm, self.fns['series_gt'], jobid)
        pulldata(self.vm, jobid, self.fns['series_gt'])
        result = queryresult(jobid, self.fns['series_gt'])
        return result[0]

    def __eq__(self, other):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['series_eq'], [self.data_id, other.data_id])
        submitjob(self.vm, self.fns['series_eq'], jobid)
        pulldata(self.vm, jobid, self.fns['series_eq'])
        result = queryresult(jobid, self.fns['series_eq'])
        return result[0]
    
    def __and__(self, other):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['series_and'], [self.data_id, other.data_id])
        submitjob(self.vm, self.fns['series_and'], jobid)
        pulldata(self.vm, jobid, self.fns['series_and'])
        result = queryresult(jobid, self.fns['series_and'])
        return result[0]
    
    def __invert__(self):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['series_invert'], [self.data_id])
        submitjob(self.vm, self.fns['series_invert'], jobid)
        pulldata(self.vm, jobid, self.fns['series_invert'])
        result = queryresult(jobid, self.fns['series_invert'])
        return result[0]
    
    def __or__(self, other):
        jobid = newguid()
        setparameter(self.vm, jobid, self.fns['series_or'], [self.data_id, other.data_id])
        submitjob(self.vm, self.fns['series_or'], jobid)
        pulldata(self.vm, jobid, self.fns['series_or'])
        result = queryresult(jobid, self.fns['series_or'])
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
    
    def value_counts(self, normalize=False, sort=True, ascending=False, bins=None, dropna=True):
        jobid = newguid()
        inputs = pushdata(self.vm, [normalize, sort, ascending, bins, dropna])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_value_counts'], inputs)
        submitjob(self.vm, self.fns['series_value_counts'], jobid)
        pulldata(self.vm, jobid, self.fns['series_value_counts'])
        result = queryresult(jobid, self.fns['series_value_counts'])
        return result[0]
    
    def isin(self, value):
        jobid = newguid()
        inputs = pushdata(self.vm, [value])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_isin'], inputs)
        submitjob(self.vm, self.fns['series_isin'], jobid)
        pulldata(self.vm, jobid, self.fns['series_isin'])
        result = queryresult(jobid, self.fns['series_isin'])
        return result[0]
    
    def unique(self):
        jobid = newguid()
        inputs = []
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_unique'], inputs)
        submitjob(self.vm, self.fns['series_unique'], jobid)
        pulldata(self.vm, jobid, self.fns['series_unique'])
        result = queryresult(jobid, self.fns['series_unique'])
        return result[0]
    
    def isna(self):
        jobid = newguid()
        inputs = []
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_isna'], inputs)
        submitjob(self.vm, self.fns['series_sina'], jobid)
        pulldata(self.vm, jobid, self.fns['series_isna'])
        result = queryresult(jobid, self.fns['series_isna'])
        return result[0]
    
    def ravel(self, order='C'):
        jobid = newguid()
        inputs = pushdata(self.vm, [order])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_ravel'], inputs)
        submitjob(self.vm, self.fns['series_ravel'], jobid)
        pulldata(self.vm, jobid, self.fns['series_ravel'])
        result = queryresult(jobid, self.fns['series_ravel'])
        return result[0]

    def mean(self, axis=None, skipna=None, level=None, numeric_only=None, **kwargs):
        jobid = newguid()
        inputs = pushdata(self.vm, [axis, skipna, level, numeric_only, kwargs])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_mean'], inputs)
        submitjob(self.vm, self.fns['series_mean'], jobid)
        pulldata(self.vm, jobid, self.fns['series_mean'])
        result = queryresult(jobid, self.fns['series_mean'])
        return result[0]
    
    def min(self, axis=None, skipna=None, level=None, numeric_only=None, **kwargs):
        jobid = newguid()
        inputs = pushdata(self.vm, [axis, skipna, level, numeric_only, kwargs])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_min'], inputs)
        submitjob(self.vm, self.fns['series_min'], jobid)
        pulldata(self.vm, jobid, self.fns['series_min'])
        result = queryresult(jobid, self.fns['series_min'])
        return result[0]

    def max(self, axis=None, skipna=None, level=None, numeric_only=None, **kwargs):
        jobid = newguid()
        inputs = pushdata(self.vm, [axis, skipna, level, numeric_only, kwargs])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_max'], inputs)
        submitjob(self.vm, self.fns['series_max'], jobid)
        pulldata(self.vm, jobid, self.fns['series_max'])
        result = queryresult(jobid, self.fns['series_max'])
        return result[0]
    
    def fillna(self, value=None, method=None, axis=None, inplace=False, limit=None, downcast=None):
        jobid = newguid()
        inputs = pushdata(self.vm, [value, method, axis, inplace, limit, downcast])
        inputs.append(self.data_id)
        setparameter(self.vm, jobid, self.fns['series_fillna'], inputs)
        submitjob(self.vm, self.fns['series_fillna'], jobid)
        pulldata(self.vm, jobid, self.fns['series_fillna'])
        result = queryresult(jobid, self.fns['series_fillna'])
        return result[0]

class _DT:
    def __init__(self, vm, dt_id, fns):
        self.vm = vm
        self.dt_id = dt_id
        self.fns = fns

    def __getattr__(self, attr):
        jobid = newguid()
        inputs = pushdata(self.vm, [attr])
        inputs.append(self.dt_id)
        setparameter(self.vm, jobid, self.fns['getattr'], inputs)
        submitjob(self.vm, self.fns['getattr'], jobid)
        pulldata(self.vm, jobid, self.fns['getattr'])
        result = queryresult(jobid, self.fns['getattr'])
        return RemoteSeries(self.vm, result[0], self.fns)