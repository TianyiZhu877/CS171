mkdir build
cd build
cmake ..
cmake --build .
./wireframe [scene_description_file.txt] [xres] [yres] 
exmaple:
./wireframe ../data/scene_bunny1.txt 800 800 | display -

