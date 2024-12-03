#!/bin/bash

# User-specified variables
BENCHMARK_PATH=./PIMbench   # The path to the benchmark directory
STATS_PATH=./stats      # The path to save stats.json files
APPS=$(find "$BENCHMARK_PATH" -mindepth 1 -maxdepth 1 -type d -exec basename {} \;)
CONFIG_FILE="../../../configs/PIMeval_BitSimdV.cfg"
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
echo "Full path of the script: $SCRIPT_DIR"
PIMEVAL_CONFIG_PATH="../../../configs/"
PIMEVAL_CONFIG_SIM="PIMeval_BitSimdV.cfg"
export PIMEVAL_CONFIG_PATH
export PIMEVAL_CONFIG_SIM

cd $BENCHMARK_PA#TH/aes/PIM/ && ./aes.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/axpy/PIM && ./axpy.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/brightness/PIM && ./brightness.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/filter-by-key/PIM && ./filter.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/gemm/PIM && ./gemm.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/gemv/PIM && ./gemv.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/histogram/PIM && ./hist.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/image-downsampling/PIM && ./image-downsampling.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/kmeans/PIM && ./km.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/knn/PIM && ./knn.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/linear-regression/PIM && ./lr.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/pooling/PIM && ./pool.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/radix-sort/PIM && ./radix-sort.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/relu/PIM && ./relu.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/triangle-count/PIM && ./tc.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/vec-add/PIM && ./vec-add.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/convolution/PIM && ./conv.out $CONFIG_FILE && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/vgg13/PIM && ./vgg13.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/vgg16/PIM && ./vgg16.out && cd $SCRIPT_DIR
#cd $BENCHMARK_PATH/vgg19/PIM && ./vgg19.out && cd $SCRIPT_DIR

# Iterate through the list of applications
for app in $APPS; do
  # Derive benchmark directory from the app name
  BENCHMARK_DIR=$(dirname "$BENCHMARK_PATH/$app")

  # Derive stats.json path
  STATS_FILE="$BENCHMARK_PATH/$app/PIM/stats.json"

  # Check if stats.json exists and copy it to the destination
  if [ -f "$STATS_FILE" ]; then
    echo "Info: Copying stats.json for $app..."
    cp "$STATS_FILE" "$STATS_PATH/$app.json"
  else
    echo "Warning: stats.json not found for $app"
  fi
done

echo "All applications processed."

