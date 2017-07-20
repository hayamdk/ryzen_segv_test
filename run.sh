#!/bin/sh

seq 0 $((${1}-1)) | xargs -n 1 -P $1 -I CPU taskset -c CPU ./run1.sh $2

