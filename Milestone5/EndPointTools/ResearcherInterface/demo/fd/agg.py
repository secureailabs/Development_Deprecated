import numpy as np
import xgboost as xgb
import pickle
from sklearn.metrics import confusion_matrix

num_parties = 3

vm1 = sail.connect("127.0.0.1", 7000)
vm2 = sail.connect("127.0.0.1", 7001)
vm3 = sail.connect("127.0.0.1", 7002)

vmlist = [vm1, vm2, vm3]

fn_preprocess = sail.registerfn("sub_preprocessing.py", 1, 3)[0]
fn_handlehash = sail.registerfn("sub_handle_hashfn.py", 2, 1)[0]
fn_train_init = sail.registerfn("sub_training_init.py", 5, 1)[0]
fn_train_update = sail.registerfn("sub_training_one_boost.py", 3, 1)[0]

fn = [fn_preprocess, fn_handlehash, fn_train_init, fn_train_update]

for fn in fnlist:
    for vm in vmlist:
        sail.pushfn(vm, fn)

X=[]
y=[]

for i in range(numb_parties):
    jobid1 = sail.newguid()
    sail.pushdata(vmlist[i], jobid1, fn_preprocess, [0], "/home/jjj/playground")
    sail.execjob(vmlist[i], fn_preprocess, jobid1)
    result = sail.pulldata(vmlist[i], jobid1, fn_preprocess, "/home/jjj/playground")
    X.append(result[0])  #X
    y.append(result[1])  #y

params = {'max_depth': 8 }
dtrain_all = xgb.DMatrix(np.asarray([np.zeros(8)]), label=np.asarray([0]))
model = xgb.Booster(params, [dtrain_all])

print("Sending node IDs to children")
for j in range(num_parties):
    train = {}
    train['training'] = 0
    train['node_id'] = j
    train['num_parties'] = num_parties
    pickle.dump(train, open('%d_train_info.pickle'%j, "wb"))
    send_data("%d_train_info.pickle"%j, j) #Send sub_enclave information to each sub_enclave

print("Sending pickled model and data to nodes")
for i in range(num_parties):
    # send_data("original_data", i)
    send_data("0_xgboost_global_model.pickle", i)

#print("Initializing hash functions")
num_dim = 8
r = 4.0
L = min(40, num_dim-1)
mu = 0.0
sigma = 1.0
hash_functions = []
#print("Setting up hash functions")
for l in range(L):
    hash_function = []
    np.random.seed(0)
    a = np.random.normal(mu, sigma, num_dim)
    b = np.random.uniform(0, r, 1)
    hash_function.append(a)
    hash_function.append(b)
    hash_functions.append(hash_function)
params_hash_functions = {}
params_hash_functions['num_dim'] = num_dim
params_hash_functions['r'] = r
params_hash_functions['L'] = L
params_hash_functions['mu'] = mu
params_hash_functions['sigma'] = sigma
params_hash_functions['hash_functions'] = hash_functions
#print("Pickling hash functions and parameters")
#with open('params_hash_functions.pickle', 'wb') as outfile:
#    pickle.dump(params_hash_functions, outfile)
#print("Sending hash functions to children")
#for i in range(num_parties):
#    send_data("params_hash_functions.pickle", i)
#print("Retrieving hash values from all children")
#for i in range(num_parties):
#    receive_data("%d_hash_values_counters_local.pickle"%i, i)
# for i in range(num_parties):
#     with open("%d_hash_values_counters_local.pickle"%i, 'rb') as f:
#         data = pickle.load(f)
#         all_hashes.append(data['hash_values'])
#         all_counters.append(data['counters'])

print("Collecting hash values")
all_hashes = []
all_counters = []

for i in range(num_parties):
    jobid2 = sail.newguid()
    sail.pushdata(vmlist[i], jobid2, fn_handlehash, [params_hash_functions, X[i]], "/home/jjj/playground")
    sail.execjob(vmlist[i], fn_handlehash, jobid2)
    result = sail.pulldata(vmlist[i], jobid2, fn_handlehash, "/home/jjj/playground")
    all_hashes.append(result['hash_values'])
    all_counters.append(result['counters'])

print("Creating similarity matrices")
hash_tables = []
for m in range(num_parties):
    print(m)
    hash_table = np.zeros((len(all_hashes[m]), num_parties)) #For each party, build a hash table of dim numInstances x numParties.
    for i in range( len(all_hashes[m]) ):#Select an instance x_i in m party
        for j in range(num_parties):  #Select parties to find global instances from
            if j == m:
                hash_table[i][j] = i
            else:
                instance_hash_counts = {}
                for k in range(len(all_counters[j])):   #Select instance k in other parties
                    instance_hash_counts[k] = 0
                    for value in all_counters[m][i].keys():  #Iterate through hash values of x_i^m
                        if all_counters[j][k][value] != 0:
                            instance_hash_counts[k] += min(all_counters[j][k][value], all_counters[m][i][value])
                hash_table[i][j] = max(instance_hash_counts, key=instance_hash_counts.get)
    hash_tables.append(hash_table)

#print("Pickling and distributing similarity matrices to children")
#with open('similarity_matrices.pickle', 'wb') as outfile:
#    pickle.dump(hash_tables, outfile)

#for i in range(num_parties):
#    send_data("similarity_matrices.pickle", i)

print("Starting training")
final_model = " "
for i in range(20):
    print("Training round %d"%i)
    training_node = i % num_parties
    local_gradients=[]
    for j in range(num_parties):
        train = {}
        train['training'] = training_node
        train['node_id'] = j
        train['num_parties'] = num_parties


        jobid3 = sail.newguid()
        sail.pushdata(vmlist[j], jobid3, fn_train_init, [X[i], y[i], train, model, hash_tables], "/home/jjj/playground")
        sail.execjob(vmlist[j], fn_train_init, jobid3)
        result = sail.pulldata(vmlist[j], jobid3, fn_train_init, "/home/jjj/playground")
        local_gradients.append(result[0])
        #pickle.dump(train, open('%d_%d_train_info.pickle'%(j,i), "wb"))

        #send_data("%d_%d_train_info.pickle"%(j,i), j)
    #print("Waiting for local gradients from sub-enclaves")
    #for j in range(num_parties):
    #    receive_data("%d_%d_local_gradients.pickle"%(j,i), j)

    #print("Sending local gradients to training node")
    #for j in range(num_parties):
    #    if j != training_node:
    #        send_data("%d_%d_local_gradients.pickle"%(j,i),j)

    #print("Waiting for updated model from training node")
    #receive_data("%d_xgboost_global_model.pickle"%i,node_id=training_node)
    for j in range(num_parties):

        train = {}
        train['training'] = training_node
        train['node_id'] = j
        train['num_parties'] = num_parties

        jobid4 = sail.newguid()
        sail.pushdata(vmlist[j], jobid4, fn_train_update, [train, local_gradients ,model, hash_tables], "/home/jjj/playground")
        sail.execjob(vmlist[j], fn_train_update, jobid4)
        result = sail.pulldata(vmlist[j], jobid3, fn_train_update, "/home/jjj/playground")
        model = result[0]


    #print("Distributing updated model to all nodes")
    #if i == 19:
    #    final_model = "20_xgboost_global_model.pickle"
    #for j in range(num_parties):
    #    if j != training_node:
    #        send_data("%d_xgboost_global_model.pickle"%i,j)

#final_xgb_model = 0
#with open(final_model, 'rb') as f:
#    final_xgb_model = pickle.load(f)
#print(final_xgb_model)

#with open('original_data', 'rb') as f:
#    original_data = pickle.load(f)
#    holdout_X = original_data['holdout_X']
#    holdout_y = original_data['holdout_y']

#test_X = []
#test_y = []
#for i in range(holdout_X.shape[0]):
#    test_X.append(np.asarray(holdout_X[i].todense())[0])
#    test_y.append(holdout_y[i])

vm4 = sail.connect("127.0.0.1", 7004)
fn_test = sail.registerfn("test.py", 1, 3)[0]
sail.pushfn(vm4, fn_test)
sail.pushfn(vm4, fn_preprocess)

jobid5 = sail.newguid()
sail.pushdata(vm4, jobid5, fn_preprocess, [0], "/home/jjj/playground")
sail.execjob(vm4, fn_preprocess, jobid5)
result = sail.pulldata(vm4, jobid5, fn_preprocess, "/home/jjj/playground")
test_X=result[0]
test_y=result[1]

jobid6 = sail.newguid()
sail.pushdata(vm4, jobid6, fn_test, [test_X, test_Y], "/home/jjj/playground")
sail.execjob(vm4, fn_test, jobid6)
result=sail.pulldata(vm4, jobid6, fn_test, "/home/jjj/playground")
conf_mat = result[0]