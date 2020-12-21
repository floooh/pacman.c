# pacman.c 

A Pacman clone written in C99 with minimal dependencies for Windows, macOS, Linux and WASM.

[WASM version](https://floooh.github.com/pacman.c/pacman.html)

For implementation details see comments in the pacman.c source file (I've tried
to structure the source code so that it can be read from top to bottom).

## Clone, Build and Run (Linux, macOS, Windows)

On the command line:

```
git clone https://github.com/floooh/pacman.c
cd pacman.c
mkdir build
cd build
cmake ..
cmake --build .
```

> NOTE: on Linux you'll need to install the OpenGL and X11 development packages (e.g. mesa-common-dev and libx11-dev).

On Mac and Linux this will create an executable called 'pacman'
in the build directory:

```
./pacman
```

On Windows, the executable is in a subdirectory:

```
Debug/pacman.exe
```

## Build and Run WASM/HTML version via Emscripten (Linux, macOS)

Setup the emscripten SDK as described here:

https://emscripten.org/docs/getting_started/downloads.html#installation-instructions

Don't forget to run ```source ./emsdk_env.sh``` after activating the SDK.

And then in the pacman.c directory:

```
mkdir build
cd build
emcmake cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
cmake --build .
```

To run the compilation result in the system web browser:

```
> emrun pacman.html
```

(this procedure should also work on Windows with ```make``` in the path, but
is currently untested)

## IDE Support

On Windows, cmake will automatically create a **Visual Studio** solution file which can be opened with the ```start``` command:
```
cd build
cmake ..
start pacman.sln
```

On macOS, the cmake **Xcode** generator can be used to create an
Xcode project which can be opened with the ```open``` command:
```
cd build
cmake -GXcode ..
open pacman.xcodeproj
```

On all platforms with **Visual Studio Code** and the Microsoft C/C++ and
CMake Tools extensions, simply open VSCode in the root directory of the
project. The CMake Tools extension will detect the CMakeLists.txt file and
take over from there:
```
cd pacman.c
code .
```
