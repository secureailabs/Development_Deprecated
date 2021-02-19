import sail

#Job 1, use xgboost algorithm to train Telco dataset

#connet to the computationVM
vm=sail.connect("127.0.0.1", 7000)
#register a function node, retrieve the function node id
fn = sail.registerfn("/home/jjj/Downloads/demoFN.py", 2, 4)
fnid = fn[0]
#Create job id
jobid = sail.newguid()

model_param = {'max_depth': 8, 'n_estimators': 100, 'colsample_bytree': 0.8, 'subsample': 0.8, 'nthread': 10, 'learning_rate': 0.1}

#push the function node to computationVM
sail.pushfn(vm, fnid)
#push the input of the function to computationVM
sail.pushdata(vm, jobid, fnid, [0, model_param], "/home/jjj/playground")
#run the job
sail.execjob(vm, fnid, jobid)
#retrieve the result
result = sail.pulldata(vm, jobid, fnid, "/home/jjj/playground")

#result[0] #model
result[1] #confusion matrix
result[2] #errors
result[3].figure #feature importance picture

#Job2, use the model obtained from job 1 to make prediction on new data set

#register the prediction function node, get new function node id and job id
fn2 = sail.registerfn("/home/jjj/Downloads/predFN.py", 2, 1)
fnid2 = fn2[0]
jobid2 = sail.newguid()

#push the prediction function node to ComputationVM, run the job and retrieve the result
sail.pushfn(vm, fnid2)
sail.pushdata(vm, jobid2, fnid2, [1, result[0]], "/home/jjj/playground")
sail.execjob(vm, fnid2, jobid2)
result2 = sail.pulldata(vm, jobid2, fnid2, "/home/jjj/playground")

#Precision, Recall, Accuracy of the prediction.
result2[0]
