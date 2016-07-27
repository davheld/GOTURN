#!/bin/bash

BASE=$1 
FILES=$BASE/*.tar
OUTPUT_BASE=$2 
for f in $FILES
do
  echo "Processing file $f"
  # Skip the "extra" files that only contain negative examples
  if [[ "$f" =~ "extra" ]]; then
    echo "Skipping $f"
    continue
  fi
  OUTPUT=$OUTPUT_BASE
  echo $OUTPUT
  mkdir -p $OUTPUT
  # take action on each file. $f store current file name
  tar -xf $f -C $OUTPUT
done

