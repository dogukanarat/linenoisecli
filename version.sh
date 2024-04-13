#!/bin/bash

if [ $# -ne 1 ] && [ $# -ne 2 ]
  then
    echo "usage: ./version.sh <version_file> [major/minor/build]"
    exit -1
fi

if [ ! -f $1 ]
then
    echo "File does not exist"
    exit -1
fi

version=`cat $1`
major=0
minor=0
build=0

regex="([0-9]+).([0-9]+).([0-9]+)"
if [[ $version =~ $regex ]]; then
  major="${BASH_REMATCH[1]}"
  minor="${BASH_REMATCH[2]}"
  build="${BASH_REMATCH[3]}"
fi

if [[ "$2" == "major" ]]; then
  major=$(echo $major + 1 | bc)
  minor=0
  build=0
elif [[ "$2" == "minor" ]]; then
  minor=$(echo $minor + 1 | bc)
  build=0
elif [[ "$2" == "build" ]]; then
  build=$(echo $build + 1 | bc)
fi

printf "${major}.${minor}.${build}" > $1
printf "${major}.${minor}.${build}"