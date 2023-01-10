#include <iostream>
#include <cstdint>
#include <vector>
#include <cstring>
#include <cmath>

#include "pfm.h"
#include "jxl_helpers.h"


int main(int argc, char* argv[])
{
    std::vector<float> framebuffer;
    uint32_t width, height;
    int n_color_channels;

    if (argc < 2) {
        std::cout << "Usage:" << std::endl
                  << "------" << std::endl
                  << argv[0] << " <pfm file>" << std::endl;
        return 0;
    }

    if (!read_pfm(argv[1], framebuffer, width, height, n_color_channels)) {
        std::cerr << "Error while reading input image file" << std::endl;
        return 1;
    }

    if (n_color_channels != 1) {
        std::cerr << "This program expects B&W images" << std::endl;
        return 1;
    }

    // Check for NaNs and Infs
    for (size_t px = 0; px < framebuffer.size(); px++) {
        if (std::isnan(framebuffer[px])) {
            std::cerr << "Found a NaN" << std::endl;
        }

        if (std::isinf(framebuffer[px])) {
            std::cerr << "Found an Inf" << std::endl;
        }
    }

    // Save the framebuffer with lossy and lossless compression
    std::cout << "Lossy" << std::endl;

    compress_framebuffer(
        framebuffer,
        "lossy.jxl",
        width, height,
        32, 8, // Quantization
        1,     // Compression
        1      // Downsampling
    );

    std::cout << "Lossless" << std::endl;

    compress_framebuffer(
        framebuffer,
        "lossless.jxl",
        width, height,
        32, 8, // Quantization
        0,     // Compression
        1      // Downsampling
    );

    return 0;
}
