#!/bin/sh

file_list=`find bin -name "*_unittest"`

IFS=$'\n'
for file_paht in $file_list
do 
    $file_paht
    [ $? != 0 ] && echo "Test fail" && exit 1
done
