#!/bin/bash

DATE='2020-05-01'

bgTask=0

for i in {287..351}
do
  NEW_DATE=$(date +%m-%d-%Y -d "$DATE + $i day")
  echo "NEW_DATE: $NEW_DATE"
  ./prepare_dataset.py --date $NEW_DATE --graph_type ws &

  bgTask=$((bgTask+1))
  
  if [[ $bgTask -eq 7 ]]; then
    wait
    bgTask=0
  fi

done
