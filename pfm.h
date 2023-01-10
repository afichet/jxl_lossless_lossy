#pragma once

#include <cstdio>
#include <cstdint>
#include <vector>

bool read_pfm(
    const char* filename,
    std::vector<float>& framebuffer,
    uint32_t& width, uint32_t& height,
    int& n_color_channels);
