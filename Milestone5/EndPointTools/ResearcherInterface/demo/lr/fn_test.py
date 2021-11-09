import torch
import sys
import pickle
import torch.nn as nn

class Party:
    def __init__(self, inputs, targets, model, opt, loss_fn):
        self.inputs = inputs
        self.targets = targets
        self.model = model
        self.opt = opt
        self.loss_fn = loss_fn
        
    # make prediction, compute cost, compute gradients    
    def get_gradients(self):
        pred = self.model(self.inputs)
        loss = self.loss_fn(pred, self.targets)
        loss.backward()
        return list(p.grad for p in self.model.parameters())

    # adjust gradients with aggregated one (fed by aggregator)
    def set_gradients(self, gradients):
        for i, p in enumerate(self.model.parameters()):
            p.grad = copy.deepcopy(gradients[i])  # or whatever other operation
            
    # adjust weights/biases and reset gradient 
    def step_opt(self):
        self.opt.step()
        self.opt.zero_grad()

party = 0
allinput = 0
with open(sys.argv[0], 'rb') as f:
    allinput = pickle.load(f)
with open(sys.argv[1], 'rb') as f:
    party = pickle.load(f)

preds = party.model(allinput)

with open(sys.argv[2], 'wb') as f:
    pickle.dump(preds, f)
