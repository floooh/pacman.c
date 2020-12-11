# pacman.c (very WIP)

A Pacman clone written in C99 with minimal dependencies for Windows, macOS and Linux.

For implementation details see comments in the pacman.c source file (I've tried
to structure the source code so that it can be read from top to bottom).

## Clone and Build and Run

On the command line:

```
git clone https://github.com/floooh/pacman.c
cd pacman.c
mkdir build
cd build
cmake ..
cmake --build .
```

On Mac and Linux this will create an executable called 'pacman'
in the build directory:

```
./pacman
```

On Windows, the executable is in a subdirectory:

```
Debug/pacman.exe
```

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
