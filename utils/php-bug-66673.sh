#!/bin/bash

if [[ -z $1 || ! -d "$1" ]]; then
    echo "Dir '$1' not exist."
    exit 0
fi

set -x

function bug_66673(){
    for file in `ls -a $1`
    do
        if [ -d $1"/"$file ]
        then
            if [[ $file != '.' && $file != '..' ]]
            then
                bug_66673 $1"/"$file
            fi
        else
            PATTERN1="^(.*)\/.libs\/([A-Za-z0-9_-]*).o$"
            originfile=$1"/"$file
            if [[ $originfile =~ $PATTERN1 ]]; then
                #echo $originfile
		newfile=`echo $originfile | sed 's/^\(.*\).libs\/\([A-Za-z0-9_-]*.o\)$/\1\2/'`
		#echo $newfile
		`cp $originfile $newfile`
            fi
        fi
    done
}

bug_66673 $1
