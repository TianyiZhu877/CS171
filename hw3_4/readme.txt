How to run:
Under hw3_4 directory
$ mkdir build; cd build
$ cmake ..
$ make -j[number of threads]

** Part 1 **
$ ./opengl_renderer [scene_description_file.txt] [xres] [yres] [mode: 0 for Gouraud, 1 for Phong = 0]
Example:
./opengl_renderer ../data/scene_armadillo.txt 720 720 1

** Part 2 **
$ ./opengl_renderer [color_texture.png] [normal_map.png] (optional: [xres] [yres])
Example:
./opengl_renderer ../data/lion.png ../data/lion-normals.png 720 720
Remember to include ../data/ in the path of the texture files.
You may also change the scene lighting and material properties in the ../data/part2_hardcoded.txt file.

The two parts use the same executable. It can take in different arguments to determine which part to run.
See comments about the code and functions in the headers under utils/include
