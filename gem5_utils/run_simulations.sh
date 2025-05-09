#!/bin/bash

# Define the gem5 executable path
GEM5_OPT="build/X86/gem5.opt"

# Define the base directory for cache configurations and neural networks
CACHE_CONFIG_DIR="cache_configs"
NN_DIR="mnist_networks"

# Define the cache configurations
l1_sizes=("16" "32" "64")  # in kB
l2_sizes=("32" "64" "128") # in kB, should correspond to l1_sizes
associativities=(2 4 8)

# Define the neural network configurations
nn_hidden_nodes=("64" "128" "256")
nn_activations=("relu" "sigmoid" "tanh")

# Check if the gem5 executable exists
if ! [ -x "$GEM5_OPT" ]; then
  echo "Error: gem5 executable not found at $GEM5_OPT"
  echo "Please make sure you have compiled gem5 and set the GEM5_OPT variable correctly."
  exit 1
fi

# Check if the cache config and NN directories exist
if ! [ -d "$CACHE_CONFIG_DIR" ]; then
  echo "Error: Cache configuration directory not found at $CACHE_CONFIG_DIR"
  exit 1
fi

if ! [ -d "$NN_DIR" ]; then
  echo "Error: Neural network directory not found at $NN_DIR"
  exit 1
fi


# Loop through all cache configurations and neural network setups
for i in "${!l1_sizes[@]}"; do
  l1_size="${l1_sizes[$i]}"
  l2_size="${l2_sizes[$i]}"  # Ensure L2 size matches the L1 size
  for assoc in "${associativities[@]}"; do
    for node in "${nn_hidden_nodes[@]}"; do
      for act in "${nn_activations[@]}"; do
        # Construct the configuration and network file names
        config_file="${CACHE_CONFIG_DIR}/config_L1_${l1_size}_L2_${l2_size}_assoc_${assoc}.py"
        network_file="${NN_DIR}/nn_${node}_${act}"  # No .c, we pass the dir and base name

        # Construct the output directory name.  Use a unique name.
        output_dir="results/L1_${l1_size}kB_L2_${l2_size}kB_assoc_${assoc}_NN_${node}_${act}"

        # Check if the configuration and network files exist
        if ! [ -f "$config_file" ]; then
          echo "Warning: Configuration file not found: $config_file. Skipping."
          continue
        fi
        if ! [ -f "$network_file" ]; then #check for the compiled binary
          echo "Warning: Neural network binary not found: $network_file. Skipping."
          continue
        fi

        # Create the output directory
        mkdir -p "$output_dir"

        # Construct the gem5 command
        gem5_command="$GEM5_OPT --outdir=$output_dir $config_file $network_file"

        # Print the command and execute it
        echo "Running simulation with: $gem5_command"
        eval "$gem5_command" # Use eval to execute the command string

        # Check the exit status of the gem5 simulation
        if [ $? -eq 0 ]; then
          echo "Simulation completed successfully. Results in $output_dir"
        else
          echo "Simulation failed. Check $output_dir for errors."
        fi
        echo "--------------------------------------------------------"
      done
    done
  done
done
echo "All simulations completed."
