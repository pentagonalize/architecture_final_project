#!/bin/bash

mkdir -p mlp_models

for num_layers in {1..4}; do
  for width in 2 4 6 8; do
    filename="mlp_models/mlp_${num_layers}layers_${width}width.py"

    cat > "$filename" << EOF
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset

# Dummy data
X = torch.randn(100, 10)
y = torch.randint(0, 2, (100,))

dataset = TensorDataset(X, y)
loader = DataLoader(dataset, batch_size=16, shuffle=True)

# Define MLP
class MLP(nn.Module):
    def __init__(self):
        super().__init__()
        layers = []
        layers.append(nn.Linear(10, $width))
EOF

    for ((i=1; i<$num_layers; i++)); do
      echo "        layers.append(nn.ReLU())" >> "$filename"
      echo "        layers.append(nn.Linear($width, $width))" >> "$filename"
    done

    cat >> "$filename" << EOF
        layers.append(nn.ReLU())
        layers.append(nn.Linear($width, 2))  # Output layer for binary classification
        self.model = nn.Sequential(*layers)

    def forward(self, x):
        return self.model(x)

model = MLP()
criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(model.parameters(), lr=0.01)

# Training loop
for epoch in range(10):
    for batch_X, batch_y in loader:
        optimizer.zero_grad()
        outputs = model(batch_X)
        loss = criterion(outputs, batch_y)
        loss.backward()
        optimizer.step()
    print(f"Epoch {epoch+1}, Loss: {loss.item():.4f}")
EOF

    echo "Created $filename"
  done
done
