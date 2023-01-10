#include "pfm.h"

bool is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}


float swap_endianness(float in) {
    union val {
        uint32_t bytes;
        float value;
    };

    val u_in, u_out;
    u_in.value = in;

    u_out.bytes  = (u_in.bytes & 0x00FF) << 8;
    u_out.bytes |= (u_in.bytes & 0xFF00) >> 8;

    return u_out.value;
}


bool read_pfm_type(std::FILE *file, int& n_color_channels)
{
    int read_c = 0;

    read_c = std::fgetc(file);

    if (std::feof(file) || std::ferror(file) || read_c != 'P') {
        n_color_channels = -1;
        return false;
    }

    read_c = std::fgetc(file);

    if (read_c == 'f') {
        n_color_channels = 1;
    } else if (read_c == 'F') {
        n_color_channels = 3;
    } else if (std::feof(file) || std::ferror(file)) {
        return false;
    } else {
        return false;
    }

    // New line

    read_c = std::fgetc(file);

    if (read_c != 0x0a || std::feof(file) || std::ferror(file)) {
        return false;
    }

    return true;
}


bool read_pfm_dimensions(std::FILE *file, uint32_t& width, uint32_t& height)
{
    int read_c;

    width = 0;

    do {
        read_c = std::fgetc(file);

        if (std::feof(file) || std::ferror(file)) {
            return false;
        }

        // Either we have a number or the termination character.
        // Otherwise, this is an invalid value
        if (read_c >= 48 && read_c <= 57) {
            width = width * 10 + (read_c - 48);
        } else if (read_c != ' ') {
            return false;
        }
    } while (read_c != ' ');


    height = 0;

    do {
        read_c = std::fgetc(file);

        if (std::feof(file) || std::ferror(file)) {
            return false;
        }

        // Either we have a number or the termination character.
        // Otherwise, this is an invalid value
        if (read_c >= 48 && read_c <= 57) {
            height = height * 10 + (read_c - 48);
        } else if (read_c != 0x0a) {
            return false;
        }
    } while (read_c != 0x0a);

    return true;
}


bool read_pfm_endiannes(std::FILE* file, bool& is_little_endian) {
    const char little_endian_marker[5] = {'-', '1', '.', '0', 0x0a};
    const char big_endian_marker[4]    =      {'1', '.', '0', 0x0a};

    int read_c;

    read_c = std::fgetc(file);

    if (read_c == '-') {
        is_little_endian = true;
    } else {
        is_little_endian = false;
    }

    // Consume until new line
    do {
        if (std::feof(file) || std::ferror(file)) {
            return false;
        }

        read_c = std::fgetc(file);
    } while(read_c != 0x0a);

    if (std::feof(file) || std::ferror(file)) {
        return false;
    }

    return true;
}


bool read_pfm(
    const char* filename,
    std::vector<float>& framebuffer,
    uint32_t& width, uint32_t& height,
    int& n_color_channels)
{
    bool is_pfm_little_endian;
    bool is_host_little_endian = !is_big_endian();

    int read_c;
    size_t size_read;

    bool status_ok = true;

    std::FILE* image_file = std::fopen(filename, "rb");

    if (image_file == NULL) {
        return false;
    }

    status_ok = read_pfm_type(image_file, n_color_channels);

    if (!status_ok) {
        goto end;
    }

    status_ok = read_pfm_dimensions(image_file, width, height);

    if (!status_ok) {
        goto end;
    }

    status_ok = read_pfm_endiannes(image_file, is_pfm_little_endian);

    if (!status_ok) {
        goto end;
    }

    framebuffer.resize(width * height * n_color_channels);

    size_read = std::fread(framebuffer.data(), sizeof(float), framebuffer.size(), image_file);

    if (size_read != framebuffer.size()) {
        status_ok = false;
        goto end;
    }

    if (is_host_little_endian != is_pfm_little_endian) {
        for (size_t i = 0; i < framebuffer.size(); i++) {
            framebuffer[i] = swap_endianness(framebuffer[i]);
        }
    }

end:
    std::fclose(image_file);

    return status_ok;
}
