#ifndef PART1_IMPLE_H
#define PART1_IMPLE_H

#include <vector>
#include <string>
#include <tuple>
#include <Eigen/Dense>
#include "pos_scale_rot.h"
#include "CR_spine.h"

namespace part1_imple {    


    extern int current_frame;
    extern int wrap_around;
    extern std::vector<PosScaleRot> interpolated_pos_scale_rots;


    // Parse a .script file
    // Returns: (frame_indices, pos_scale_rots, num_frames)
    std::tuple<std::vector<int>, std::vector<PosScaleRot>, int> 
    parse_pos_scale_rot_file(const std::string& file_path);
    
    // Interpolate PosScaleRot objects using Catmull-Rom spline
    std::vector<PosScaleRot> interpolate_pos_scale_rot(
        const std::vector<int>& frame_indices,
        const std::vector<PosScaleRot>& pos_scale_rots, 
        int num_frames
    );
    void display(void);

    void window_resize(int width, int height);

    void key_pressed(unsigned char key, int x, int y);

}

#endif

