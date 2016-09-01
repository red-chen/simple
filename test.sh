#!/bin/sh

find bin -name "*_unittest" > bin/cases

IFS=$'\n'
for file_path in `cat bin/cases`
do 
    echo ">> $file_path"
    [ $? != 0 ] && echo "Test fail" && exit 1
done
