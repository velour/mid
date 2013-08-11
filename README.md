Building
========

Prerequisites
-------------

You'll need the following tools:
  * sh (or an sh-compatible shell)
  * make (GNU or GNU-compatible)
  * clang (or gcc if you must)
  * ar
  * sed
  * awk

You'll need the following libraries:
  * SDL 2.0 (or 1.2, see below)
  * SDL_image
    * libpng
    * zlib
  * SDL_mixer
    * libogg
    * libvorbis
  * SDL_ttf
    * libfreetype

On OSX, the SDL libraries should be in framework form, under /Library/Frameworks,
and you won't have to worry about the other dependencies.

On Windows you will almost certainly need mingw and SDL libraries compatible with it.
When building SDL in mingw, be sure to configure each like so:

	./configure --prefix=/mingw

In order to get logging to the console (soon we will send it to a file instead of stderr, but until then),
add the "--disable-stdio-redirect" flag to the above command line for libSDL.


Do It
-----

If you're using clang, run "./build" from the source code directory's root.
If it doesn't fail, you can run "cmd/mid/mid"!

If you're using gcc, set the CC and LD environment variables to gcc and then run "./build".

If you don't want to build SDL 2.0 (it's kind of a pain to build all of SDL on Windows,
due to the various dependencies), you can use SDL 1.2 by setting the SDLVER environment
variable to 1 before running "build".
