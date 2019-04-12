Schools, Flocks, & Herds Animation
Author: Teresa Van
Date: April 12th, 2019
Copyright (2019)

Note:
This version has no dependencies on C++17, nor std::optional.

The project now includes its own version of glad AND glfw that need to be
compiled with project. This should happen automatically with any of the options
below for installation.

**INSTALL**

Option 1:

1) Load up project in QT Creator (clicking on CMakeLists.txt)
2) click Desktop, then build (on left hand sideside)

Option 2 (untested, but typical for cmake):

1) From the project director in terminal

mkdir build
cd build

#Unix makefile (Tested)
cmake ..

#Mac OSX (Tested)
cmake -G "Xcode" ..

Note: You may have to set you working director of your project manually to the directory where
your curves and shaders folders are inorder for the executable to find them.

#Microsoft Windows (Tested)
cmake -G "Visual Studio 15" ..
cmake -G "Visual Studio 15 Win64" ..

**USAGE**

./a4

**USER INTERFACE**

The default number of boids and obstacles is 300 and 8 respectively.
You can edit 'initialstate.txt' to change these variables before running the program.

Make sure 'initialstate.txt' and 'fishred.obj' are in the same folder as the program.

KEY CONTROLS:
    - SPACE - Toggle interactively guiding the fish using mouse
    - ESC	- Quit
