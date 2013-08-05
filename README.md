Building
--------

Run the build command from the project root:

	./build

If you get an error in SDL headers, you may not have SDL 2.0 on your system. Either install it or set the SDLVER environment variable:

	export SDLVER=12 ./build
