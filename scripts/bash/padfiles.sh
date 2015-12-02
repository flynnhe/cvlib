#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 path_to_scripts" >&2
  exit 1
fi

basepath=$1
find . -name '*.png' | while read line; do
  path=$(dirname "$line")
  fname=$(basename $line)
  mv $path/$fname $path/`$basepath/zeropad.sh $fname`
done
