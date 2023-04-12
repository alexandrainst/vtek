#!/bin/bash

# storing initial directory
initial_dir="$(pwd)"

# travelling back until we are at VV/src/ directory
while [ "$(pwd | awk -F/ '{print $NF}')" != "vclab-vtek" ];
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
#files="$(find -type f | egrep -v 'bin/|obj/|vendor/|Makefile|premake5.lua|.gitignore'# | \
#    egrep '.hpp$|.cpp$')"
files="$(find include/ src/ -type f)"

# go through all the sources files
for f in $files;
do
    # check if the file contains a todo. If not, continue the loop
    todos="$(printf $f | xargs egrep -n 'TODO: ')"
    [ -z "$todos" ] && continue

    # file did contain one or more todos. Print the file name
    printf "$f\n" | cut -d'/' -f 2-

    # go through all todos and print them nicely formatted
    while IFS= read -r line;
    do
	echo "$line" | sed -r "s/([0-9]+:)([ \t]*)/  \1 /"
    done <<< "$todos"
    printf "\n"
done

# travel back to starting directory
cd "$initial_dir"
