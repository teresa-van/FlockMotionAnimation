Mass Spring Systems Animation
Author: Teresa Van
Date: March 26th, 2019
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

./a3

**USER INTREFACE**

1 - Single mass
2 - Chain pendulum
3 - Cube of jelly
4 - Hanging cloth
5 - Cloth falling onto table

SPACE - Pause/unpause

ESC	- Quit

## Description for tablecloth scene:
To simulate a cloth falling onto the table, I set up my springs and masses like I did for the hanging cloth scene, except without any fixed masses.
During the force calculations for each mass, I first check if the y position of the particle is less than the y position of the table,
The problem specifications didn't state what shape the table was. 
I initially tried a circle, but the edges looked weird due to the resolution of the cloth.
So I changed it to a rectangular table instead. However, I included the calculations for a circular table (commented out).
For a circle table: Plug in x and z positions of the mass into equation of a circle (x^2 + z^2 = r^2), if the result is less than r^2, then the mass is in contact with the table.
For a rectangular table: Check if the x position of the mass is greater than the x position of the left side of the table, and less than the right side.
                         Do the same for the z-position. If both satisfy, then the mass is in contact with the table.
If the mass is in contact with the table, then I change that mass to a fixed mass.
For reference, all calculations for this part happen between lines 357-366.
