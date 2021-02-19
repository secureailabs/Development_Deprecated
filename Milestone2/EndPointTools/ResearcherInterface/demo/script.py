import sail

vm=sail.connect("127.0.0.1", 7000)
fn = sail.registerfn("/home/jjj/Downloads/demoFN.py", 2, 4)
fnid = fn[0]
jobid = sail.newguid()

model_param = {'max_depth': 8, 'n_estimators': 100, 'colsample_bytree': 0.8, 'subsample': 0.8, 'nthread': 10, 'learning_rate': 0.1}

sail.pushfn(vm, fnid)
sail.pushdata(vm, jobid, fnid, [0, model_param], "/home/jjj/playground")
sail.execjob(vm, fnid, jobid)
result = sail.pulldata(vm, jobid, fnid, "/home/jjj/playground")

#result[0] #model
result[1] #confusion matrix
result[2] #errors
result[3].fig #feature importance picture

fn2 = sail.registerfn("/home/jjj/Downloads/predFN.py", 2, 1)
fnid[2] = fn2[0]
jobid2 = sail.newguid()
sail.pushfn(vm, fnid2)
sail.pushdata(vm, jobid2, fnid2, [1, result[0]], "/home/jjj/playground")
sail.execjob(vm, fnid2, jobid2)
result2 = sail.pulldata(vm, jobid2, fnid2, "/home/jjj/playground")

resutl[0]
