#!/bin/sh

build_dir="build"
if [ ! -d "$build_dir" ]; then
	mkdir $build_dir
fi


if [ ! -f "build/resources.o" ]; then
	./linux_build_resources.sh
fi

pushd $build_dir > /dev/null

echo '~~~ building game ~~~'
gcc -O2 ../source/linux_jam_game_platform_impl.c ../source/jam_game.c resources.o -lX11 -o jam_game
echo 'done'

popd > /dev/null