#!/bin/bash
PATH="/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:$PATH"

while getopts ":rd:" flag;
do
    case "${flag}" in
        d) extra=${OPTARG};;
        r) rebuild="1";;
    esac
done

if [ -z ${rebuild+x} ]; then
	cd build
	make -j8
else
	rm -fR build
	mkdir build
	cd build
	cmake .. $extra
fi
