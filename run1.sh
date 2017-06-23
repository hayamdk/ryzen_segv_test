#!/bin/sh

COUNT=0
while :
do
	COUNT=$(( COUNT + 1 ))
	./ryzen_segv_test $1 >/dev/null 2>>log.txt
	if [ "$?" -eq 0 ]
	then
		echo "${COUNT}: $(date): OK" >>log.txt
		#:
	else
		echo "${COUNT}: $(date): NG" >>log.txt
	fi
done

