#!/bin/sh

file_list=`find bin -name "*_unittest"`

for file_path in $file_list
do 
    $file_path
    [ $? != 0 ] && echo "Test fail" && exit 1
done
