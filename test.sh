#!/bin/sh

file_list=`find bin -name "*_unittest"`

IFS=$'\n'
for file_path in $file_list
do 
    echo ">> $file_path"
    $file_path
    [ $? != 0 ] && echo "Test fail" && exit 1
done
