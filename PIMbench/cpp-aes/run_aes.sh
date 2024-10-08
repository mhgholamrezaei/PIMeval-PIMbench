#!/bin/bash
#SBATCH --job-name="aes"
#SBATCH -n 1
#SBATCH -t 3-00:00:00
#SBATCH -p cpu
#SBATCH --cpus-per-task=60
#SBATCH --mem=128GB

# input size = 1 GB
input_size=$((2**20))

# Get the config file path from the first argument
config_file=$1

# Extract the base name of the config file (without path and extension)
config_name=$(basename "$config_file" .cfg)

# Set the output and error log file names based on the config file name
error_file="aes-${config_name}.err"
output_file="aes-${config_name}.out"

# Redirect stdout and stderr to the log files
exec > "$output_file" 2> "$error_file"

# Print Slurm job information
echo "Slurm Job ID: $SLURM_JOB_ID"
echo "Slurm Job Name: $SLURM_JOB_NAME"
echo "Node List: $SLURM_JOB_NODELIST"
echo "Number of Nodes: $SLURM_JOB_NUM_NODES"
echo "CPU Tasks per Node: $SLURM_CPUS_ON_NODE"

# Print the config file being used
echo "Using config file: $config_file"

# Run the aes.out program with the specified config file
./PIM/aes.out -f $config_file -v false -l $input_size


