#!/bin/bash

# storing initial directory
initial_dir="$(pwd)"

# travelling back until we are at VV/src/ directory
while [ "$(pwd | awk -F/ '{print $NF}')" != "vtek" ];
do
    cd ..
    if [ "$(pwd)" == "/" ];
    then
        echo "Please enter vclab-vtek folder before running the script."
        cd "$initial_dir"
        exit 1
    fi
done

# output line numbers of all code files included in projects
find include/ src/ -type f | xargs wc -l

# travel back to starting directory
cd "$initial_dir"
