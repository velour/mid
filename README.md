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
  * pkg-config (on Linux only)

You'll need the following libraries:
  * SDL 2.0
  * SDL2_image
    * libpng
    * zlib
  * SDL2_mixer
    * libogg
    * libvorbis
  * SDL2_ttf
    * libfreetype

The `make prereqs` command can be used on OSX and Linux (Windows forthcoming)
to easily download and install the build dependencies.

On OSX, the SDL libraries should be in framework form, under /Library/Frameworks,
and you won't have to worry about the other dependencies.

On Windows you will almost certainly need mingw and SDL libraries compatible with it.
When building SDL in mingw, be sure to configure each like so:

	./configure --prefix=/mingw

In order to get logging to the console (soon we will send it to a file instead of stderr, but until then),
add the "--disable-stdio-redirect" flag to the above command line for libSDL.


Do It
-----

If you're using clang, run "make" from the source code directory's root.
If it doesn't fail, you can run "cmd/mid/mid"!

If want to use gcc, override the CC and LD variables:

	make CC=gcc LD=gcc
