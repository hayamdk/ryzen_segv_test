#!/bin/sh

seq 1 16 | xargs -n 1 -P 16 ./run1.sh $1

