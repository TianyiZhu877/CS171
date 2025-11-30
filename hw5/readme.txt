How to run:
Under hw3 directory
$ mkdir build; cd build
$ cmake ..
$ make -j[number of threads]
$ ./opengl_renderer [scene_description_file.txt] [xres] [yres] 

Example:
./opengl_renderer ../data/scene_armadillo.txt 720 720

See comments about the code and functions in the headers under utils/include
