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

The `make prereqs` command can be used
to easily download and install the build dependencies.

On OSX, the SDL libraries should be in framework form, under /Library/Frameworks,
and you won't have to worry about the other dependencies.

On Windows you will need mingw/msys and Powershell script execution priveledges.
You will at least need to set:

	set-executionpolicy remotesigned

And you may need to "unblock" win/webget.ps1 before running `make prereqs`.


Do It
-----

If you're using clang, run "make" from the source code directory's root.
If it doesn't fail, you can run "cmd/mid/mid"!

If want to use gcc, override the CC and LD variables:

	make CC=gcc LD=gcc
