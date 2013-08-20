#!/bin/sh

pushd $HOME/Downloads

get_it() {
	lib=$1
	url=$2

	if [ -e $lib.dmg ]; then
		return
	fi

	curl -s -S $url -o $lib.dmg || exit 1
	hdiutil attach $lib.dmg || exit 2
	sudo cp -rf /Volumes/$lib/$lib.framework /Library/Frameworks/ || exit 3
	hdiutil detach /Volumes/$lib || exit 4
}

sdlproj=http://www.libsdl.org/projects

get_it SDL2 http://www.libsdl.org/release/SDL2-2.0.0.dmg
get_it SDL2_image $sdlproj/SDL_image/release/SDL2_image-2.0.0.dmg
get_it SDL2_mixer $sdlproj/SDL_mixer/release/SDL2_mixer-2.0.0.dmg
get_it SDL2_ttf $sdlproj/SDL_ttf/release/SDL2_ttf-2.0.12.dmg

popd
