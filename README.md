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

On Windows you will need mingw/msys and Powershell script execution privileges.
You will at least need to set:

	set-executionpolicy remotesigned

And you may need to "unblock" win/webget.ps1 before running `make prereqs`.


Do It
-----

If you're using clang, run "make" from the source code directory's root.
If it doesn't fail, you can run "cmd/mid/mid"!

If want to use gcc, override the CC and LD variables:

	make CC=gcc LD=gcc

Installers
--------

On OSX and Windows, you can run `make installer` to create Mid.app (on OSX) and a Mid folder (on Windows).
These can be distributed to other machines and should work without installing any prerequites.

Playing
=======

Run `cmd/mid/mid` from the dev area, or click on Mid.app on OSX, or mid.exe in the Windows Mid folder.

The default controls are ESDF-style for movement, `j` to swing the sword, `u` to perform general actions
like opening doors, `n` to access the inventory, and `k` to cast a spell if you have one.
You can change the controls in the options menu, or
by providing a keymap file to the `-k` flag of the mid executable. For an example, look at dvorak.km.

Debugging
---------

The `-p` flag to mid allows it to read a level description via standard input, rather than executing its
own pipeline of the various -gen programs. This, combined with the "cur.lvl" and "debug.log" files that
are saved on each run, can be used to easily reproduce issues.
