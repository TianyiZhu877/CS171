#include "ppm_image.h"
#include <iostream>
#include <cstdint>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./ppm_test xres yres" << std::endl;
        return 1;
    }
    
    int xres = std::stoi(argv[1]);  // width
    int yres = std::stoi(argv[2]);  // height
    
    ppm_image::PPMImage<uint8_t> image(yres, xres, ppm_image::MAGENTA);
    
    // Calculate the radius and center of the circle
    int radius = (xres <= yres) ? xres / 4 : yres / 4;
    int center_x = xres / 2;
    int center_y = yres / 2;
    
    // Iterate through the each pixel to fill the circle
    for (int y = 0; y < yres; ++y) 
        for (int x = 0; x < xres; ++x) {
            int dx = x - center_x;
            int dy = y - center_y;
            if (dx * dx + dy * dy <= radius * radius) 
                image[y][x] = ppm_image::YELLOW;
        }
    
    // Print the ppm image to cout
    image.serialize();
    
    return 0;
}
