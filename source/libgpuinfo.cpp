/*
 * Copyright (c) 2021-2023 ARM Limited.
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

#include <array>
#include <cerrno>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "libgpuinfo.hpp"

namespace libgpuinfo {

struct product_entry {
    uint32_t id;
    uint32_t mask;
    uint32_t min_cores;
    const char* name;
    const char* architecture;
    uint32_t fp32_fmas_per_engine;
    std::function<uint32_t(int, uint32_t, uint32_t)> get_num_texels;
    std::function<uint32_t(int, uint32_t, uint32_t)> get_num_pixels;
    std::function<uint32_t(int, uint32_t, uint32_t)> get_num_exec_engines;
};

static const uint32_t MASK_OLD { 0xFFFF };
static const uint32_t MASK_NEW { 0xF00F };

static uint32_t get_num_1(
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    return 1;
}

static uint32_t get_num_2(
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    return 2;
}

static uint32_t get_num_3(
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    return 3;
}

static uint32_t get_num_4(
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    return 4;
}

static uint32_t get_num_8(
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    return 8;
}

static uint32_t get_num_ee_g31(
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    if ((core_count == 1) && ((thread_features & 0xFFFF) == 0x2000))
    {
        return 1;
    }
    return 2;
}

static uint32_t get_num_ee_g51(
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    if ((core_count == 1) && ((thread_features & 0xFFFF) == 0x2000))
    {
        return 1;
    }
    return 3;
}

static uint32_t get_num_ee_g52(
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    return core_features & 0xF;
}

static uint32_t get_num_ee_g310_g510(
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    if ((core_features & 0xF) <= 1)
    {
        return 1;
    }

    return 2;
}

const std::array<product_entry, 30> PRODUCT_VERSIONS {{
    //                  ID,  ID Mask, Min cores,              Name,      Args, FMA,    Texels,    Pixels,   Engines
    product_entry { 0x6956, MASK_OLD,         1,       "Mali-T600", "Midgard",  4, get_num_1, get_num_1, get_num_2 },
    product_entry { 0x0620, MASK_OLD,         1,       "Mali-T620", "Midgard",  4, get_num_1, get_num_1, get_num_2 },
    product_entry { 0x0720, MASK_OLD,         1,       "Mali-T720", "Midgard",  4, get_num_1, get_num_1, get_num_1 },
    product_entry { 0x0750, MASK_OLD,         1,       "Mali-T760", "Midgard",  4, get_num_1, get_num_1, get_num_2 },
    product_entry { 0x0820, MASK_OLD,         1,       "Mali-T820", "Midgard",  4, get_num_1, get_num_1, get_num_1 },
    product_entry { 0x0830, MASK_OLD,         1,       "Mali-T830", "Midgard",  4, get_num_1, get_num_1, get_num_2 },
    product_entry { 0x0860, MASK_OLD,         1,       "Mali-T860", "Midgard",  4, get_num_1, get_num_1, get_num_2 },
    product_entry { 0x0880, MASK_OLD,         1,       "Mali-T880", "Midgard",  4, get_num_1, get_num_1, get_num_3 },
    product_entry { 0x6000, MASK_NEW,         1,        "Mali-G71", "Bifrost",  4, get_num_1, get_num_1, get_num_3 },
    product_entry { 0x6001, MASK_NEW,         1,        "Mali-G72", "Bifrost",  4, get_num_1, get_num_1, get_num_3 },
    product_entry { 0x7000, MASK_NEW,         1,        "Mali-G51", "Bifrost",  4, get_num_2, get_num_2, get_num_ee_g51 },
    product_entry { 0x7001, MASK_NEW,         1,        "Mali-G76", "Bifrost",  8, get_num_2, get_num_2, get_num_3 },
    product_entry { 0x7002, MASK_NEW,         1,        "Mali-G52", "Bifrost",  8, get_num_2, get_num_2, get_num_ee_g52 },
    product_entry { 0x7003, MASK_NEW,         1,        "Mali-G31", "Bifrost",  4, get_num_2, get_num_2, get_num_ee_g31 },
    product_entry { 0x9000, MASK_NEW,         1,        "Mali-G77", "Valhall", 16, get_num_4, get_num_2, get_num_2 },
    product_entry { 0x9001, MASK_NEW,         1,        "Mali-G57", "Valhall", 16, get_num_4, get_num_2, get_num_2 },
    product_entry { 0x9003, MASK_NEW,         1,        "Mali-G57", "Valhall", 16, get_num_4, get_num_2, get_num_2 },
    product_entry { 0x9004, MASK_NEW,         1,        "Mali-G68", "Valhall", 16, get_num_4, get_num_2, get_num_2 },
    product_entry { 0x9002, MASK_NEW,         1,        "Mali-G78", "Valhall", 16, get_num_4, get_num_2, get_num_2 },
    product_entry { 0x9005, MASK_NEW,         1,      "Mali-G78AE", "Valhall", 16, get_num_4, get_num_2, get_num_2 },
    product_entry { 0xa002, MASK_NEW,         1,       "Mali-G710", "Valhall", 32, get_num_8, get_num_4, get_num_2 },
    product_entry { 0xa007, MASK_NEW,         1,       "Mali-G610", "Valhall", 32, get_num_8, get_num_4, get_num_2 },
    // TODO: Extract FMA, pixel, and texel settings
    product_entry { 0xa003, MASK_NEW,         1,       "Mali-G510", "Valhall", 32, get_num_8, get_num_4, get_num_ee_g310_g510 },
    // TODO: Extract FMA, pixel, and texel settings
    product_entry { 0xa004, MASK_NEW,         1,       "Mali-G310", "Valhall", 32, get_num_8, get_num_4, get_num_ee_g310_g510 },
    product_entry { 0xb002, MASK_NEW,        10, "Immortalis-G715", "Valhall", 64, get_num_8, get_num_4, get_num_2 },
    product_entry { 0xb003, MASK_NEW,        10, "Immortalis-G715", "Valhall", 64, get_num_8, get_num_4, get_num_2 },
    product_entry { 0xb002, MASK_NEW,         7,       "Mali-G715", "Valhall", 64, get_num_8, get_num_4, get_num_2 },
    product_entry { 0xb003, MASK_NEW,         7,       "Mali-G715", "Valhall", 64, get_num_8, get_num_4, get_num_2 },
    product_entry { 0xb002, MASK_NEW,         1,       "Mali-G615", "Valhall", 64, get_num_8, get_num_4, get_num_2 },
    product_entry { 0xb003, MASK_NEW,         1,       "Mali-G615", "Valhall", 64, get_num_8, get_num_4, get_num_2 },
}};

uint32_t get_gpu_id(
    uint32_t gpu_id
) {
    for (const auto& entry : PRODUCT_VERSIONS)
    {
        if (((gpu_id & entry.mask) == entry.id))
        {
            return gpu_id & entry.mask;
        }
    }

    return gpu_id;
}

const char* get_gpu_name(
    uint32_t gpu_id,
    int core_count
) {
    for (const auto& entry : PRODUCT_VERSIONS)
    {
        if(((gpu_id & entry.mask) == entry.id) &&
           (core_count >= entry.min_cores))
        {
            return entry.name;
        }
    }

    return "Unknown gpu_id";
}

const char* get_architecture_name(
    uint32_t gpu_id
) {
    for (const auto& entry : PRODUCT_VERSIONS)
    {
        if((gpu_id & entry.mask) == entry.id)
        {
            return entry.architecture;
        }
    }

    return "Unknown gpu_id";
}

int get_num_exec_engines(
    uint32_t gpu_id,
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    for (const auto& entry : PRODUCT_VERSIONS)
    {
        if(((gpu_id & entry.mask) == entry.id) &&
           (core_count >= entry.min_cores))
        {
            return entry.get_num_exec_engines(core_count, core_features, thread_features);
        }
    }

    return 0;
}

const uint32_t get_num_fp32_fmas(
    uint32_t gpu_id,
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    for (const auto& entry : PRODUCT_VERSIONS)
    {
        if(((gpu_id & entry.mask) == entry.id) &&
           (core_count >= entry.min_cores))
        {
            return entry.fp32_fmas_per_engine * entry.get_num_exec_engines(core_count, core_features, thread_features);
        }
    }

    return 0;
}

const uint32_t get_num_texels(
    uint32_t gpu_id,
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    for (const auto& entry : PRODUCT_VERSIONS)
    {
        if(((gpu_id & entry.mask) == entry.id) &&
           (core_count >= entry.min_cores))
        {
            return entry.get_num_texels(core_count, core_features, thread_features);
        }
    }

    return 0;
}

const uint32_t get_num_pixels(
    uint32_t gpu_id,
    int core_count,
    uint32_t core_features,
    uint32_t thread_features
) {
    for (const auto& entry : PRODUCT_VERSIONS)
    {
        if(((gpu_id & entry.mask) == entry.id) &&
           (core_count >= entry.min_cores))
        {
            return entry.get_num_pixels(core_count, core_features, thread_features);
        }
    }

    return 0;
}


class prop_decoder {
  public:
    explicit prop_decoder(std::vector<unsigned char> buffer)
        : reader_{std::move(buffer)} {}

    gpuinfo decode(bool &success)  {
        gpuinfo dev_consts {};
        uint64_t num_core_groups {};
        std::array<uint64_t, 16> core_mask {};
        uint64_t raw_core_features {};
        uint64_t raw_thread_features {};

        while (reader_.size() > 0) {
            auto p = next(success);
            if (!success) {
                return {};
            }

            prop_id_type id = p.first;
            uint64_t value = p.second;

            switch (id) {
            case prop_id_type::product_id:
                dev_consts.gpu_id = value;
                break;
            case prop_id_type::l2_log2_cache_size:
                dev_consts.num_l2_bytes = (1UL << value);
                break;
            case prop_id_type::l2_num_l2_slices:
                dev_consts.num_l2_slices = value;
                break;
            case prop_id_type::raw_l2_features:
                /* log2(bus width in bits) stored in top 8 bits of register. */
                dev_consts.num_bus_bits = 1UL << ((value & 0xFF000000) >> 24);
                break;
            case prop_id_type::raw_core_features:
                raw_core_features = value;
                break;
            case prop_id_type::coherency_num_core_groups:
                num_core_groups = value;
                break;
            case prop_id_type::raw_thread_features:
                raw_thread_features = value;
                break;
            case prop_id_type::coherency_group_0:
                core_mask[0] = value;
                break;
            case prop_id_type::coherency_group_1:
                core_mask[1] = value;
                break;
            case prop_id_type::coherency_group_2:
                core_mask[2] = value;
                break;
            case prop_id_type::coherency_group_3:
                core_mask[3] = value;
                break;
            default:
                break;
            }
        }

        // TODO: Build this as we go?
        dev_consts.num_shader_cores = 0;
        for (uint64_t i = 0; i < num_core_groups; ++i)
        {
            dev_consts.num_shader_cores += __builtin_popcount(core_mask[i]);
        }

        dev_consts.num_exec_engines = get_num_exec_engines(
            dev_consts.gpu_id,
            dev_consts.num_shader_cores,
            raw_core_features,
            raw_thread_features);

        dev_consts.num_fp32_fmas_per_cy = get_num_fp32_fmas(
            dev_consts.gpu_id,
            dev_consts.num_shader_cores,
            raw_core_features,
            raw_thread_features);

        dev_consts.num_fp16_fmas_per_cy = dev_consts.num_fp32_fmas_per_cy * 2;

        dev_consts.num_texels_per_cy = get_num_texels(
            dev_consts.gpu_id,
            dev_consts.num_shader_cores,
            raw_core_features,
            raw_thread_features);

        dev_consts.num_pixels_per_cy = get_num_pixels(
            dev_consts.gpu_id,
            dev_consts.num_shader_cores,
            raw_core_features,
            raw_thread_features);

        if (!dev_consts.num_exec_engines) {
            return {};
        }

        return dev_consts;
    }

  private:
    /** Property id type. */
    using prop_id_type = kbase_post_r21::get_gpuprops_t::gpuprop_code;
    /** Property size type. */
    using prop_size_type = kbase_post_r21::get_gpuprops_t::gpuprop_size;

    static std::pair<prop_id_type, prop_size_type> to_prop_metadata(uint32_t v)  {
        /* Property id/size encoding is:
         * +--------+----------+
         * | 31   2 | 1      0 |
         * +--------+----------+
         * | PropId | PropSize |
         * +--------+----------+
         */
        static constexpr unsigned prop_id_shift = 2;
        static constexpr unsigned prop_size_mask = 0x3;

        return {static_cast<prop_id_type>(v >> prop_id_shift), static_cast<prop_size_type>(v & prop_size_mask)};
    }

    std::pair<prop_id_type, uint64_t> next(bool& success)  {
        success = true;
        auto p = to_prop_metadata(reader_.read_bytes<uint32_t>(success));
        if (!success)
        {
            return {};
        }

        prop_id_type id = p.first;
        prop_size_type size = p.second;

        switch (size) {
        case prop_size_type::uint8:
            return {id, reader_.read_bytes<uint8_t>(success)};
        case prop_size_type::uint16:
            return {id, reader_.read_bytes<uint16_t>(success)};
        case prop_size_type::uint32:
            return {id, reader_.read_bytes<uint32_t>(success)};
        case prop_size_type::uint64:
            return {id, reader_.read_bytes<uint64_t>(success)};
        }

        return {};
    }

    /* Reads values out of the KBASE_IOCTL_GET_GPUPROPS data buffer */
    class prop_reader {
      public:
        explicit prop_reader(std::vector<unsigned char> buffer)
            : buffer_{std::move(buffer)}
            , data_{buffer_.data()}
            , size_{buffer_.size()} {}

        size_t size() const noexcept { return size_; }

        template <typename T>
        T read_bytes(bool& success)  {
            // Check we have enough bytes in the buffer
            if (size_ < sizeof(T)) {
                success = false;
                return 0;
            }

            T ret {};
            for (size_t b = 0; b < sizeof(T); b++)
            {
                ret |= static_cast<T>(static_cast<uint64_t>(data_[b]) << (8 * b));
            }
            data_ += sizeof(T);
            size_ -= sizeof(T);
            return ret;
        }

      private:
        std::vector<unsigned char> const buffer_;
        unsigned char const *data_;
        std::size_t size_;
    } reader_;
};

/**
 * Mali device driver instance.
 */
std::unique_ptr<instance> instance::create(
    uint32_t id
) {
    std::string device_path("/dev/mali" + std::to_string(id));

    // Open the kernel driver device node
    const int fd = ::open(device_path.c_str(), O_RDONLY);
    if (fd < 0) {
        return nullptr;
    }

    // Check that it is a character device
    struct stat s {};
    const int fs_result = fstat(fd, &s);
    if ((fs_result < 0) || (S_ISCHR(s.st_mode) == 0)) {
        ::close(fd);
        return nullptr;
    }

    // Create the instance
    auto result = std::unique_ptr<instance>(new instance(fd));
    if (!result || !result->valid_) {
        return nullptr;
    }

    return result;
}

/**
 * Get the GPU device constants.
 *
 * @return The device constants.
 */
const gpuinfo& instance::get_info() const
{
    return constants_;
};

/**
  * Destroy an instance, closing the device fd.
  */
instance::~instance()
{
    ::close(fd_);
}

instance::instance(int fd):
    fd_(fd)
{
    if (!version_check()) {
        valid_ = false;
        return;
    }

    if (!set_flags()) {
        valid_ = false;
        return;
    }

    if (!init_constants()) {
        valid_ = false;
        return;
    }
}

static bool is_supported(unsigned int major, unsigned int minor)
{
    return (major > 10) || ((major == 10) && (minor >= 2));
}

/** Detect kbase version. */
bool instance::version_check() {
    // Probe pre-r21 JM kernel
    // Must be first in the list because CSF reuses an old IOCTL ID
    iface_ = iface_type::pre_r21;
    kbase_pre_r21::version_check_t pre_r21 {};
    pre_r21.header.id = kbase_pre_r21::header_id::version_check;
    ::ioctl(fd_, kbase_pre_r21::version_check, &pre_r21);
    // If this is non-zero this must be pre-r21 driver, so check version
    if (pre_r21.is_set()) {
        return is_supported(pre_r21.major, pre_r21.minor);
    }

    // Probe r21+ JM kernel
    iface_ = iface_type::post_r21;
    kbase_post_r21::version_check_t post_r21 {};
    ::ioctl(fd_, kbase_post_r21::version_check_jm, &post_r21);
    // If this is non-zero this must be post-r21 JM driver, so check version
    if (post_r21.is_set()) {
        return is_supported(post_r21.major, post_r21.minor);
    }

    // Probe r21+ CSF kernel
    ::ioctl(fd_, kbase_post_r21::version_check_csf, &post_r21);
    // If this is any non-zero value this is a valid CSF GPU
    return post_r21.is_set();
}

/** Call set flags ioctl. */
bool instance::set_flags() {
    static constexpr auto system_monitor_flag_submit_disabled_bit = 1;
    static constexpr auto system_monitor_flag = 1U << system_monitor_flag_submit_disabled_bit;

    // Clear errno
    errno = 0;

    if (iface_ == iface_type::pre_r21) {
        kbase_pre_r21::set_flags_t flags {};
        flags.header.id = kbase_pre_r21::header_id::set_flags;
        flags.create_flags = system_monitor_flag;
        ::ioctl(fd_, kbase_pre_r21::set_flags, &flags);
    } else {
        kbase_post_r21::set_flags_t flags { system_monitor_flag };
        ::ioctl(fd_, kbase_post_r21::set_flags, &flags);
    }

    // Mali driver will fail if reinitialized, but it's benign
    // TODO: Does this ever happen with this usage pattern
    return errno == 0 || errno == EINVAL || errno == EPERM;
}

/** Initialize constants_ field. */
bool instance::init_constants() {
    bool success;
    if (iface_ == iface_type::pre_r21) {
        success = props_pre_r21(fd_);
    } else {
        success = props_post_r21(fd_);
    }

    // Perform some common cleanup on the data
    if (!success)
    {
        return false;
    }

    constants_.num_l2_bytes *= constants_.num_l2_slices;
    constants_.gpu_name = get_gpu_name(constants_.gpu_id, constants_.num_shader_cores);
    constants_.architecture_name = get_architecture_name(constants_.gpu_id);
    constants_.gpu_id = get_gpu_id(constants_.gpu_id);
    return true;
}

/** Get device constants from old ioctl. */
bool instance::props_pre_r21(int fd) {
    int error = 0;

    kbase_pre_r21::uk_gpuprops_t props {};
    props.header.id = kbase_pre_r21::header_id::get_props;
    errno = 0;
    ::ioctl(fd, kbase_pre_r21::get_gpuprops, &props);
    if (errno) {
        return false;
    }

    constants_.gpu_id = props.props.core_props.product_id;
    constants_.num_l2_bytes = 1UL << props.props.l2_props.log2_cache_size;
    constants_.num_l2_slices = props.props.l2_props.num_l2_slices;
    constants_.num_bus_bits = 1UL << (props.props.raw_props.l2_features >> 24);

    constants_.num_shader_cores = 0;
    for (uint32_t i = 0; i < props.props.coherency_info.num_core_groups; i++)
    {
        constants_.num_shader_cores += __builtin_popcount(props.props.coherency_info.group[i].core_mask);
    }

    constants_.num_exec_engines = get_num_exec_engines(
        constants_.gpu_id,
        constants_.num_shader_cores,
        0, 0);

    constants_.num_fp32_fmas_per_cy = get_num_fp32_fmas(
        constants_.gpu_id,
        constants_.num_shader_cores,
        0, 0);

    constants_.num_fp16_fmas_per_cy = constants_.num_fp32_fmas_per_cy * 2;

    constants_.num_texels_per_cy = get_num_texels(
        constants_.gpu_id,
        constants_.num_shader_cores,
        0, 0);

    constants_.num_pixels_per_cy = get_num_pixels(
        constants_.gpu_id,
        constants_.num_shader_cores,
        0, 0);

    return true;
}

/** Get the raw properties buffer as it's returned from the kernel. */
bool instance::props_post_r21(int fd) {
    errno = 0;

    kbase_post_r21::get_gpuprops_t get_props = {};
    int size = ::ioctl(fd, kbase_post_r21::get_gpuprops, &get_props);
    if (errno) {
        return false;
    }

    std::vector<unsigned char> buffer(static_cast<std::size_t>(size));
    get_props.size = static_cast<uint32_t>(size);
    get_props.buffer.reset(buffer.data());
    ::ioctl(fd, kbase_post_r21::get_gpuprops, &get_props);
    if (errno) {
        return false;
    }

    prop_decoder decoder { buffer };
    bool success = true;
    constants_ = decoder.decode(success);
    return success;
}

}
