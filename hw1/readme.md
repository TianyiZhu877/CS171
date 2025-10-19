### How to use

In hw1 dir:
```bash
mkdir build; cd build
cmake ..
cmake --build .
./wireframe [scene_description_file.txt] [xres] [yres]
``` 
Exmaple:
```
./wireframe ../data/scene_bunny1.txt 800 800 | display -
```

The anti-aliasing is implemented. Your can go to utils/include/scene.h to switch the ENABLE_ANTIALIASING  

### Explanations

The Bresenham algorithms was exteneded to support any arbitrary slopes of the lines. I implemented it by **remapping** them back to the 0-1 range allowed by the version introduced in course notes. Here are the steps I used:
1. Map the slope to ranges originally in +/- 1 to inf back to +/- 0 to 1 if needed. This is done by swapping the two axis x and y. (Notice the axises for filling the image below should also be changed correspondingly)
2. Swap the newly selected x axis such that x0 is the smaller than x1 (after step 1) to ensure the original assumption that we are iterating from lower to higher x still holds. y0 and y1 are swapped together with them to be consistent.
3. The y could be increasing or decreasing then, but the original algorithm could be adaptive to that. One thing to noticed we should determine the direction of where it steps by whether checking whether y1 (end) is greator than y0 (start). 

After this remapping, the original algorithm could be reused.

Most key functionalities are implemented in utils as libaries and classes. The scene.h is the key component that parse the scene description file, keep track of the verteices and surfaces, as well as providing a rendering pipeline. The actual implementations of the rendering algos are in rendering.h. The function and names in the headers are mostly self-explanatory. Some comments are added in key components.





