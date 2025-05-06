#!/bin/bash

# Requires have python environment with pytorch and pyinstaller 
# pip install torch
# pip install -pyinstaller

echo "Creating python scripts"
sh mlp_create_loop_pytorch.sh
echo "Converting python scripts into exe files"
sh compile_py_folder.sh ./mlp_models/
