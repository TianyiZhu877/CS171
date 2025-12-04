How to run:
Under hw5 directory
$ mkdir build; cd build
$ cmake ..
$ make -j[number of threads]
$ ./smooth [scene_description_file.txt] [xres] [yres] [h]

Example:
./smooth ../data/scene_armadillo.txt 720 720 0.0001

!!!NOTE: Fairing is not applied right after launching the program, press 'r' to compute the fairing.
Keys:
  -- 'q' - quit the program
  -- 'd' - double the h value
  -- 'c' - halve the h value
  -- 'r' - reset the h value to the original value

For the laplacian operator L, each row of the corresponding matrix M could be expressed as:
Assuming this row index is 1, and it has edge 1-3 and 1-5 (all zero-indexed)
        0    sum_of_rest_of_this_row   0    cot(alpha_3)+cot(beta_3)            0           cot(alpha_5)+cot(beta_5)
index   0         1                   2                     3                    4                  5
Also they need to be divided by the area of faces A as described in the lecture notes.
This will excatly get the expression of the laplacian operator.
And then convert it to F = I - hL, we just by doing I - hM. See utils/include/models.h:205 for the implementation.

See comments about the code and functions in the headers under utils/include
