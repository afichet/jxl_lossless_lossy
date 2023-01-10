#include "jxl_helpers.h"

#include <cmath>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <cstring>

#include <jxl/encode.h>
#include <jxl/encode_cxx.h>

#include <jxl/thread_parallel_runner.h>
#include <jxl/thread_parallel_runner_cxx.h>


// ----------------------------------------------------------------------------

#define CHECK_JXL_ENC_STATUS(status)                                          \
    if (JXL_ENC_SUCCESS != (status)) {                                        \
        std::stringstream err_msg;                                            \
        err_msg << "[ERR] JXL_ENCODER: "                                      \
                << __FILE__ << ":" << __LINE__                                \
                << "> " << status;                                            \
        throw std::runtime_error(err_msg.str());                              \
    }                                                                         \

// ----------------------------------------------------------------------------

void compress_framebuffer(
    const std::vector<float>& framebuffer_in,
    const char* filename,
    uint32_t width, uint32_t height,
    uint32_t bits_per_sample,
    uint32_t exponent_bits_per_sample,
    float frame_distance,
    uint32_t downsampling_ratio)
{
    assert(framebuffer_in.size() == width * height);

    // 1. Compress the framebuffer using JXL
    JxlEncoderStatus           status;
    JxlEncoderPtr              enc;
    JxlThreadParallelRunnerPtr runner;
    JxlBasicInfo               basic_info;
    JxlColorEncoding           color_encoding;

    runner = JxlThreadParallelRunnerMake(
            nullptr,
            JxlThreadParallelRunnerDefaultNumWorkerThreads()
        );

    enc = JxlEncoderMake(nullptr);

    status = JxlEncoderSetParallelRunner(
        enc.get(),
        JxlThreadParallelRunner,
        runner.get()
    );

    CHECK_JXL_ENC_STATUS(status);

    // ------------------------------------------------------------------------

    JxlEncoderInitBasicInfo(&basic_info);

    basic_info.xsize                    = width;
    basic_info.ysize                    = height;
    basic_info.num_extra_channels       = 0;
    basic_info.num_color_channels       = 1;
    basic_info.bits_per_sample          = bits_per_sample;
    basic_info.exponent_bits_per_sample = exponent_bits_per_sample;
    basic_info.uses_original_profile    = JXL_TRUE;

    status = JxlEncoderSetBasicInfo(enc.get(), &basic_info);

    CHECK_JXL_ENC_STATUS(status);

    // ------------------------------------------------------------------------

    JxlEncoderFrameSettings* frame_settings = JxlEncoderFrameSettingsCreate(enc.get(), nullptr);

    // Set compression quality
    JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_EFFORT, 7);
    CHECK_JXL_ENC_STATUS(status);

    if (frame_distance > 0) {
        status = JxlEncoderSetFrameLossless(frame_settings, JXL_FALSE);
        status = JxlEncoderSetFrameDistance(frame_settings, frame_distance);
    } else {
        status = JxlEncoderSetFrameLossless(frame_settings, JXL_TRUE);
    }
    CHECK_JXL_ENC_STATUS(status);

    status = JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_RESAMPLING, downsampling_ratio);
    CHECK_JXL_ENC_STATUS(status);

    // ------------------------------------------------------------------------

    color_encoding.color_space       = JXL_COLOR_SPACE_GRAY;
    color_encoding.white_point       = JXL_WHITE_POINT_D65;
    color_encoding.primaries         = JXL_PRIMARIES_SRGB;
    color_encoding.transfer_function = JXL_TRANSFER_FUNCTION_LINEAR;
    color_encoding.rendering_intent  = JXL_RENDERING_INTENT_PERCEPTUAL;

    status = JxlEncoderSetColorEncoding(enc.get(), &color_encoding);

    CHECK_JXL_ENC_STATUS(status);

    // ------------------------------------------------------------------------

    JxlPixelFormat format;
    format.num_channels = 1;
    format.data_type    = JXL_TYPE_FLOAT,
    format.endianness   = JXL_NATIVE_ENDIAN;
    format.align        = 0;

    status = JxlEncoderAddImageFrame(
        frame_settings,
        &format,
        framebuffer_in.data(),
        width * height * sizeof(float)
    );

    CHECK_JXL_ENC_STATUS(status);

    JxlEncoderCloseInput(enc.get());

    // ------------------------------------------------------------------------

    std::vector<uint8_t> compressed(64);

    uint8_t* next_out = compressed.data();
    size_t avail_out = compressed.size() - (next_out - compressed.data());

    status = JXL_ENC_NEED_MORE_OUTPUT;

    while (status == JXL_ENC_NEED_MORE_OUTPUT) {
        status = JxlEncoderProcessOutput(enc.get(), &next_out, &avail_out);

        if (status == JXL_ENC_NEED_MORE_OUTPUT) {
            size_t offset = next_out - compressed.data();
            compressed.resize(compressed.size() * 2);

            next_out  = compressed.data() + offset;
            avail_out = compressed.size() - offset;
        }
    }

    CHECK_JXL_ENC_STATUS(status);

    // ------------------------------------------------------------------------

    compressed.resize(next_out - compressed.data());

    std::FILE* file = std::fopen(filename, "wb");
    std::fwrite(compressed.data(), sizeof(uint8_t), compressed.size(), file);
    std::fclose(file);
}
