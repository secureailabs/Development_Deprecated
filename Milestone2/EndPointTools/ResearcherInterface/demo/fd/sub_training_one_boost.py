import pickle
import sys
import numpy as np
import xgboost as xgb


#function node has 4 input and 1 output
#input1: train info
#input2: the grad and hess matrix
#input3: old model
#input4: hash tables
#output: updated model

X = 0
y = 0
with open(sys.argv[0], "rb") as f:
    X = pickle.load(f)
with open(sys.argv[1], "rb") as f:
    y = pickle.load(f)

xgb_local = xgb.DMatrix(np.asarray(X), label=np.asarray(y))

node_id = 0
training_node_id = 0
num_parties = 0

with open(sys.argv[2], "rb") as f:
    info = pickle.load(f)
    training_node_id = info['training']
    node_id = info['node_id']
    num_parties = info['num_parties']

model = 0
with open(sys.argv[3], 'rb') as f:
    model = pickle.load(f)

if node_id == training_node_id :
    #print("Waiting for local gradients from other sub-enclaves")
    #for i in range(num_parties):
    #    receive_data()
    #print("Received all gradients")
    #all_grad = []
    #all_hess = []
    #for i in range(num_parties):
    #    with open('%d_%d_local_gradients.pickle'%(node_id,yolo), 'rb') as f:
    #        data = pickle.load(f)
    #        all_grad.append(data['grad'])
    #        all_hess.append(data['hess'])
    all_grad = []
    all_hess = []
    with open(sys.argv[4], 'rb') as f:
        data = pickle.load(f)
        for i in range(num_parties):
            all_grad.append(data[i]['grad'])
            all_hess.append(data[i]['hess'])

    hash_tables = 0
    with open(sys.argv[5], 'rb') as f:
        hash_tables = pickle.load(f)
    
    G = []
    H = []
    for i in range(num_parties):
        G.append([])
        H.append([])
    # for m in tqdm(range(20)): #Select central processing party that receives gradients from others
    #     m_ind = m % num_parties
    for i in range(num_parties):
        if i != node_id:
            Gi = np.zeros((num_parties, len(hash_tables[node_id])))
            Hi = np.zeros((num_parties, len(hash_tables[node_id])))
            #print("Len of hashes:", len(hash_tables[i]))
            #print("Len of grads: ", len(all_grad[i]))
            for q in range(len(hash_tables[i])):
                s = int(hash_tables[i][q][node_id])
                # dtrain = xgb.DMatrix(np.asarray([vectors[i][0][q]]), label=np.asarray([vectors[i][1][q]]))
                # dpred_inst = model.predict(dtrain)
                # grad, hess = cust_obj(dpred_inst, dtrain)
                q = min(q, len(hash_tables[node_id])-1)
                Gi[node_id][s] += all_grad[i][q]
                Hi[node_id][s] += all_hess[i][q]
            G[i] = Gi
            H[i] = Hi
    Gm = np.zeros((num_parties, len(hash_tables[node_id])))
    Hm = np.zeros((num_parties, len(hash_tables[node_id])))
    # Gm = np.zeros((len(vectors), len(vectors[m_ind][0])))
    # Hm = np.zeros((len(vectors), len(vectors[m_ind][0])))
    for q in range(len(hash_tables[node_id])):
        for i in range(num_parties):
            if i == node_id:
                # dtrain = xgb.DMatrix(np.asarray([vectors[m_ind][0][q]]), label=np.asarray([vectors[m_ind][1][q]]))
                # dpred_inst = model.predict(dtrain)
                # grad, hess = cust_obj(dpred_inst, dtrain)
                Gm[node_id][q] += all_grad[node_id][q]
                Hm[node_id][q] += all_hess[node_id][q]
            else:
                Gm[node_id][q] += G[i][node_id][q]
                Hm[node_id][q] += H[i][node_id][q]
    G[node_id] = Gm
    H[node_id] = Hm

    model.boost(xgb_local, Gm[node_id], Hm[node_id])
    #print("Finished boosting with aggregated gradients")
    #print("Sending model to hub enclave")
with open(sys.argv[6], "wb") as f:
    pickle.dump(model, f)
    #send_data("%d_xgboost_global_model.pickle"%(yolo+1))
    #print("Sent model")