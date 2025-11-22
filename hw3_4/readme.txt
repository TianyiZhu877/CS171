How to run:
Under hw3 directory
$ mkdir build; cd build
$ cmake ..
$ make -j[number of threads]
$ ./opengl_renderer [scene_description_file.txt] [xres] [yres] [mode: 0 for Gouraud, 1 for Phong = 0]

Example:
./opengl_renderer ../data/scene_armadillo.txt 720 720

See comments about the code and functions in the headers under utils/include
