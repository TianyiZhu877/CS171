#ifndef PART2_SPECIFIC_H
#define PART2_SPECIFIC_H

namespace part2 {
    // loaded in the beginning when opengl initializes
    bool load_textures(const char* texture_png_path, const char* normal_png_path);
    
    // hardcoded surface, called for every draw object cycle
    void draw_surface();
}

#endif // PART2_SPECIFIC_H
