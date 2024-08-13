#!/bin/bash
set -x

config_files=(
  "../../configs/iiswc/PIMeval_Bank_Rank16.cfg"
  "../../configs/iiswc/PIMeval_Bank_Rank1.cfg"
  "../../configs/iiswc/PIMeval_Bank_Rank32.cfg"
  "../../configs/iiswc/PIMeval_Bank_Rank4.cfg"
  "../../configs/iiswc/PIMeval_Bank_Rank8.cfg"
  "../../configs/iiswc/PIMeval_BitSerial_Rank16.cfg"
  "../../configs/iiswc/PIMeval_BitSerial_Rank1.cfg"
  "../../configs/iiswc/PIMeval_BitSerial_Rank32.cfg"
  "../../configs/iiswc/PIMeval_BitSerial_Rank4.cfg"
  "../../configs/iiswc/PIMeval_BitSerial_Rank8.cfg"
  "../../configs/iiswc/PIMeval_Fulcrum_Rank16.cfg"
  "../../configs/iiswc/PIMeval_Fulcrum_Rank1.cfg"
  "../../configs/iiswc/PIMeval_Fulcrum_Rank32.cfg"
  "../../configs/iiswc/PIMeval_Fulcrum_Rank4.cfg"
  "../../configs/iiswc/PIMeval_Fulcrum_Rank8.cfg"
)

for config in "${config_files[@]}"
do
  # Call the Slurm script for each config file, running each in the background
  bash run_aes.sh $config
done

# Wait for all background jobs to finish
wait

