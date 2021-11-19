from decimal import DivisionImpossible
from ..core import newguid, pushdata, pulldata, submitjob, setparameter, queryresult
from pandas._libs.lib import no_default

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
    
    def train_test_split(self, X, y, test_size=None, train_size=None, random_state=None, shuffle=True, stratify=None):
        jobid = newguid()
        inputs = pushdata(self.vm, [test_size, train_size, random_state, shuffle, stratify])
        inputs.append(X)
        inputs.append(y)
        setparameter(self.vm, jobid, self.fns['util_train_test_splits'], inputs)
        submitjob(self.vm, self.fns['util_train_test_splits'], jobid)
        pulldata(self.vm, jobid, self.fns['util_train_test_splits'])
        result = queryresult(jobid, self.fns['util_train_test_splits'])
        return result
    
    # def read_csv(
    #     self, filepath_or_buffer, sep=no_default, delimiter=None, header='infer', names=no_default, 
    #     index_col=None, usecols=None, squeeze=False, prefix=no_default, mangle_dupe_cols=True, dtype=None, 
    #     engine=None, converters=None, true_values=None, false_values=None, skipinitialspace=False, skiprows=None, 
    #     skipfooter=0, nrows=None, na_values=None, keep_default_na=True, na_filter=True, verbose=False, skip_blank_lines=True, 
    #     parse_dates=False, infer_datetime_format=False, keep_date_col=False, date_parser=None, dayfirst=False, cache_dates=True, 
    #     iterator=False, chunksize=None, compression='infer', thousands=None, decimal='.', lineterminator=None, quotechar='"', 
    #     quoting=0, doublequote=True, escapechar=None, comment=None, encoding=None, encoding_errors='strict', dialect=None, 
    #     error_bad_lines=None, warn_bad_lines=None, on_bad_lines=None, delim_whitespace=False, low_memory=True, memory_map=False, 
    #     float_precision=None, storage_options=None
    # ):
    #     jobid = newguid()
    #     inputs = pushdata(self.vm, [sep, delimiter, header, names, index_col, usecols, squeeze, prefix, mangle_dupe_cols, dtype,
    #                       engine, converters, true_values, false_values, skipinitialspace, skiprows, skipfooter, nrows, na_values,
    #                       keep_default_na, na_filter, verbose, skip_blank_lines, parse_dates, infer_datetime_format, keep_date_col, date_parser,
    #                       dayfirst, cache_dates, iterator, chunksize, compression, thousands, decimal, lineterminator, quotechar,
    #                       quoting, doublequote, escapechar, comment, encoding, encoding_errors, dialect, error_bad_lines,
    #                       warn_bad_lines, on_bad_lines, delim_whitespace, low_memory, memory_map, float_precision, storage_options])
    #     inputs.append(filepath_or_buffer)
    #     setparameter(self.vm, jobid, self.fns['util_read_csv'], inputs)
    #     submitjob(self.vm, self.fns['util_read_csv'], jobid)
    #     pulldata(self.vm, jobid, self.fns['util_read_csv'])
    #     result = queryresult(jobid, self.fns['util_read_csv'])
    #     return result[0]

    def read_csv(self, file):
        jobid = newguid()
        inputs = [file]
        setparameter(self.vm, jobid, self.fns['util_read_csv'], inputs)
        submitjob(self.vm, self.fns['util_read_csv'], jobid)
        pulldata(self.vm, jobid, self.fns['util_read_csv'])
        result = queryresult(jobid, self.fns['util_read_csv'])
        return result[0]
