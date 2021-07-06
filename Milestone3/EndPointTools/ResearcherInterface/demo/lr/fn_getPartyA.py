import numpy as np
import torch
import torch.nn as nn
import copy
import torch.nn.functional as F
import sys
import pickle

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

loss_fn = F.mse_loss

model = 0
with open(sys.argv[0], 'rb') as f:
    model = pickle.load(f)

inputs = np.array([[73, 67, 43], 
                   [91, 88, 64], 
                   [87, 134, 58]], dtype='float32')

# Targets (apples, oranges)
targets = np.array([[56, 70], 
                    [81, 101], 
                    [119, 133]], dtype='float32')

inputs = torch.from_numpy(inputs)
targets = torch.from_numpy(targets)

opt = torch.optim.SGD(model.parameters(), lr=1e-5)

party_a = Party(inputs, targets, model, opt, loss_fn)

with open(sys.argv[1], 'wb') as f:
    pickle.dump(party_a, f)
    f.flush()
