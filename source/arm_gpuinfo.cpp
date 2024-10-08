
/*
 * Copyright (c) 2023-2024 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @brief An example command line application using libGPUInfo.
 *
 * This file contains a command line application that will query and print
 * key properties about your device, and the Arm GPU that it contains.
 *
 * It is primarily intended as an example of using the libGPUInfo library, but
 * the command line application itself is a useful diagnostic tool for support
 * investigations.
 *
 * On Android devices you can install and run the application from the shell:
 *
 *     adb push arm_gpuinfo /data/local/tmp
 *     adb shell chmod u+x /data/local/tmp/arm_gpuinfo
 *     adb shell /data/local/tmp/arm_gpuinfo
 *
 * The generated output is formatted using a YAML-like syntax, but by default is
 * designed for human consumption with additional line breaks. To generate
 * strictly compliant YAML output for use in scripts pass the --yaml or -y
 * argument on the arm_gpuinfo command line.
 */

#include <iostream>
#include <sys/utsname.h>
#include <cstring>

#if defined(__ANDROID__)
    #include <sys/system_properties.h>
#endif

#include "libgpuinfo.hpp"

#if defined(__ANDROID__)
std::string get_android_property(
    const char* propertyA,
    const char* propertyB=nullptr
) {
    char buf[PROP_VALUE_MAX];
    int size = __system_property_get(propertyA, buf);

    if (!size && propertyB) {
        size = __system_property_get(propertyB, buf);
    }

    std::string result { buf };
    result[0] = toupper(result[0]);
    return result;
}
#endif

std::string get_kernel_version() {
    struct utsname unamedata;
    uname(&unamedata);
    return { unamedata.release };
}

int main(int argc, char *argv[])
{
    bool emit_yaml = false;
    for (int i = 1; i < argc; i++)
    {
        if ((!strcmp(argv[i], "-y")) || (!strcmp(argv[i], "--yaml")))
        {
            emit_yaml = true;
        }
    }

    auto instance = libarmgpuinfo::instance::create();
    if (!instance)
    {
        std::cout << "ERROR: Failed to create instance\n";
        return 1;
    }

    const auto info = instance->get_info();

    if (emit_yaml)
    {
        std::cout << "---\n";
    }

    std::cout << "Device configuration:\n";
#if defined(__ANDROID__)
    std::cout << "  Manufacturer: " << get_android_property("ro.product.vendor.manufacturer", "ro.product.brand") << "\n";
    std::cout << "  Model: " << get_android_property("ro.product.vendor.model", "ro.product.model") << "\n";
    std::cout << "  Android version: " << get_android_property("ro.build.version.release") << "\n";
#endif
    std::cout << "  Kernel version: " << get_kernel_version() << "\n";
    if (!emit_yaml)
    {
        std::cout << "\n";
    }

    std::cout << "GPU configuration:\n";
    std::cout << "  Name: " << info.gpu_name << "\n";
    std::cout << "  Architecture: " << info.architecture_name << "\n";
    std::cout << "  Architecture version: " << info.architecture_major
              << "." << info.architecture_minor <<"\n";
    std::cout << "  Model number: 0x" << std::hex << info.gpu_id << std::dec << "\n";
    std::cout << "  Core count: " << info.num_shader_cores << "\n";
    std::cout << "  Core mask: 0x" << std::hex << info.shader_core_mask << std::dec << "\n";
    std::cout << "  L2 cache count: " << info.num_l2_slices << "\n";
    std::cout << "  Total L2 cache size: " << info.num_l2_bytes << " bytes\n";
    std::cout << "  Bus width: " << info.num_bus_bits << " bits\n";
    if (!emit_yaml)
    {
        std::cout << "\n";
    }

    if (!info.num_exec_engines)
    {
        std::cout << "ERROR: Detected an unknown model "
                  << std::hex << info.gpu_id << std::dec << "\n";
        return 1;
    }

    std::cout << "Per-core statistics:\n";
    std::cout << "  Engine count: " << info.num_exec_engines << "\n";
    std::cout << "  FP32 FMAs: " << info.num_fp32_fmas_per_cy << "/cy\n";
    std::cout << "  FP16 FMAs: " << info.num_fp16_fmas_per_cy << "/cy\n";
    std::cout << "  Texels: " << info.num_texels_per_cy << "/cy\n";
    std::cout << "  Pixels: " << info.num_pixels_per_cy << "/cy\n";
    if (!emit_yaml)
    {
        std::cout << "\n";
    }

    std::cout << "Per-GPU statistics:\n";
    std::cout << "  FP32 FMAs: " << info.num_fp32_fmas_per_cy * info.num_shader_cores << "/cy\n";
    std::cout << "  FP16 FMAs: " << info.num_fp16_fmas_per_cy * info.num_shader_cores << "/cy\n";
    std::cout << "  Texels: " << info.num_texels_per_cy * info.num_shader_cores << "/cy\n";
    std::cout << "  Pixels: " << info.num_pixels_per_cy * info.num_shader_cores << "/cy\n";

    return 0;
}
