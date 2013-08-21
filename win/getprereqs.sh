#!/bin/sh

get_it() {
	lib=$1
	url=$2

	if [ -e $lib ]; then
		return
	fi

	powershell.exe ../win/webget.ps1 $url $lib.tar.gz
	tar -xzvf $lib.tar.gz
	pushd $lib
	./configure --prefix=/mingw && make && make install || exit 1
	popd
}

mkdir -p prereqs && pushd prereqs

sdlproj=http://www.libsdl.org/projects

get_it SDL2-2.0.0 http://www.libsdl.org/release/SDL2-2.0.0.tar.gz
get_it SDL2_image-2.0.0 $sdlproj/SDL_image/release/SDL2_image-2.0.0.tar.gz 
get_it SDL2_mixer-2.0.0 $sdlproj/SDL_mixer/release/SDL2_mixer-2.0.0.tar.gz
get_it SDL2_ttf-2.0.12 $sdlproj/SDL_ttf/release/SDL2_ttf-2.0.12.tar.gz

popd
