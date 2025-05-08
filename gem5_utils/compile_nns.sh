#!/bin/bash

# Script to compile all neural network variants in the nn_variants directory

echo "Compiling all neural network variants..."

# Navigate to the directory containing the source files
cd mnist_networks || exit 1

# Loop through all .c files in the current directory
for c_file in *.c; do
  if [[ "$c_file" == "common_functions.c" ]]; then
    continue # Skip the common functions file if it exists
  fi
  # Extract the base name of the file (without the .c extension)
  output_name="${c_file%.c}"

  # Compile the C file
  echo "Compiling $c_file to $output_name..."
  gcc -o "$output_name" "$c_file" -lm
  if [ $? -eq 0 ]; then
    echo "Compilation of $c_file successful."
  else
    echo "Compilation of $c_file failed."
  fi
done

echo "Finished compiling all neural network variants."
echo "The compiled executables are in the 'nn_variants' directory."
echo "You can now run them directly, e.g., ./nn_64_sigmoid"
