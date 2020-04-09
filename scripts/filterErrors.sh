#!/bin/bash

DIR=$1
cd $DIR
for file in *
do
	if [[ $file == *.err ]]
	then
		test_num=${file%.*.*}
		constraint="$test_num.kquery"
		cat $constraint
	fi
done
cd -
	
