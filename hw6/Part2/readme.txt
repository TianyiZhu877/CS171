How to run:
Under hw2 directory
$ mkdir build; cd build
$ cmake ..
$ cmake --build .
$ ./shaded_renderer [scene_description_file.txt] [xres] [yres] [mode]

Example:
./shaded_renderer ../data/scene_cube2.txt 1200 1200 1 | display -

New codes added in hw2:
scene.h: Organize the scene such as models, lights, camera, and pass the data to the rendering.
    - Functions to look at: SceneFile::render()
rendering.h: actual implementation of the rastering: compute lighting and rendering objects
    - Functions to look at: lighting(), render_object()
shader.h: the implementation of the gouraud and phong shading
    - All derived from the base class Shader, allowing passing to the render_object() function
