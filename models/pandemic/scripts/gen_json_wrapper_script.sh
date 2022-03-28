#!/bin/bash

DATE='2020-05-01'

bgTask=0

for i in {561..600}
do
  CURR_DATE=$(date +%m-%d-%Y -d "$DATE + $i day")
  echo "CURR_DATE: $CURR_DATE"
  ./prepare_dataset.py --date $CURR_DATE --graph_type ws &

  bgTask=$((bgTask+1))

  # process tasks in batches of 7 at a time
  if [[ $bgTask -eq 7 ]]; then
    echo -e "⌛Waiting ...\n"
    wait
    bgTask=0
  fi

done

# add wait here??
echo -e "⌛Waiting ...\n"
wait
echo -e "\n🔔All finished. Check logs for any errors!"
