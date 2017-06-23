#!/bin/sh

seq 1 $1 | xargs -n 1 -P $1 ./run1.sh "$2"

