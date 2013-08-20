#!/bin/sh
#
# Downloads and installs SDL2, SDL2_image, SDL2_mixer, and SDL2_ttf from source.
#
#

J=$(cat /proc/cpuinfo | grep cpuid | wc -l)

#
# SDL2
#

test -e SDL2-2.0.0 || {
	wget http://www.libsdl.org/release/SDL2-2.0.0.tar.gz || exit 1
	tar -xzvf SDL2-2.0.0.tar.gz
}
cd SDL2-2.0.0 || exit 1
./configure && make -j $J && sudo make install || exit 1
cd ..

#
# SDL2_image
#

test -e SDL2_image-2.0.0 || {
	wget http://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.0.tar.gz || exit 1
	tar -xzvf SDL2_image-2.0.0.tar.gz
}
cd SDL2_image-2.0.0 || exit 1
./configure && make -j $J && sudo make install || exit 1
cd ..

#
# SDL2_mixer
#

test -e SDL2_mixer-2.0.0 || {
	wget http://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.0.tar.gz || exit 1
	tar -xzvf SDL2_mixer-2.0.0.tar.gz
}
cd SDL2_mixer-2.0.0 || exit 1
./configure && make -j $J && sudo make install || exit 1
cd ..

#
# SDL2_ttf
#

test -e SDL2_ttf-2.0.12 || {
	wget http://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.12.tar.gz || exit 1
	tar -xzvf SDL2_ttf-2.0.12.tar.gz
}
cd SDL2_ttf-2.0.12 || exit 1
./configure && make -j $J && sudo make install || exit 1
cd ..
