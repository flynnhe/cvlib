#!/bin/bash
find . -name '*.png' | while read line; do
	path=$(dirname "$line")
	fname=$(basename $line)
	mv $path/$fname $path/`./zeropad.sh $fname`
done
