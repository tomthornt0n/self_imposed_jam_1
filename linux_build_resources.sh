#!/bin/sh

build_dir="build"
if [ ! -d "$build_dir" ]; then
	mkdir $build_dir
fi

pushd $build_dir > /dev/null

if [ ! -f "resembed" ]; then
	echo '~~~ compiling resembed ~~~'
	gcc -g ../source/resembed.c -lm -o resembed
fi

echo '~~~ generating resource files ~~~'
./resembed

echo '~~~ compiling resources ~~~'
gcc resources.gen.c -c -o resources.o

popd > /dev/null
