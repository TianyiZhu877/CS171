How to run:
### For Part 1:
Go to every directory and run $ make
The abs() function in Elasticity/utils.h was disabled otherwise wouldn't compile on ubuntu.


### For Part 2:
$ mkdir build; cd build
$ cmake ..
$ make -j[num_threads]
$ ./bunny_frames [optional --export]
$ ./i_bar [script file] [xres] [yres]
Both of the executable are compiled with single make command.

Example:
./i_bar ../data/test.script 720 720
For i_bar, press 'f' to move to the next frame and 'd' for the previous frame.
For bunny_frames, it loads the expected file and compares the result with the computed one. If you
add the --export flag, it will export the result to the Bunny_Frames/interpolate_results folder.

About how my data structure for spline interpolfation works:
The interpolation uses a 4×N matrix where rows represent 4 consecutive keyframes and columns 
represent N flattened attributes in parallel. For bunny_frames, columns are the flattened xyz 
coordinates of all vertices. For i_bar, columns are flattened attributes (pos x,y,z, scale 
x,y,z, quaternion w,x,y,z). This datastructure is compatible to the Catmull-Rom spline matrix 
multiplication operation: each_frame_result = u_vector x B x window_matrix. This  interpolates 
all N attributes simultaneously in a single matrix multiplication.
