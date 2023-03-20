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

/**
 * @brief The core libGPUInfo library interface.
 *
 * This library provides developers with an easy way to query the Arm
 * Immortalis or Arm Mali GPU configuration in their system.  This information
 * can be used to adjust rendering workload to match the capabilities of the
 * device.
 *
 * The library is simple to use:
 *
 *     // Create a connection with the kernel driver ...
 *     std::unique_ptr<instance> conn = libgpuinfo::instance::create();
 *     if (!conn)
 *     {
 *         std::cout << "ERROR: Failed to create Mali instance\n";
 *         return;
 *     }
 *
 *     // Fetch the information result and do something with it ...
 *     const gpuinfo& info = conn->get_info();
 *     std::cout << "GPU: " << info.gpu_name << " MP" << info.num_shader_cores << "\n";
 *
 * Note that the returned information object is returned by reference, and has
 * the same lifetime as the instance object.
 */

#pragma once

#include <array>
#include <cerrno>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace libgpuinfo {

/** Mali GPU information. */
struct gpuinfo
{
    /** GPU name */
    const char* gpu_name;

    /** GPU architecture name */
    const char* architecture_name;

    /** GPU ID */
    uint32_t gpu_id;

    /** Number of shader cores */
    uint32_t num_shader_cores;

    /** Number of L2 cache slices */
    uint32_t num_l2_slices;

    /** L2 cache size, summed for all slices, in bytes */
    uint32_t num_l2_bytes;

    /** GPU external bus width per cache slice, in bits */
    uint32_t num_bus_bits;

    /** Number of execution engines per core */
    uint32_t num_exec_engines;

    /** Maximum number of 32-bit floating-point FMAs per clock per core */
    uint32_t num_fp32_fmas_per_cy;

    /** Maximum number of 16-bit floating-point FMAs per clock per core */
    uint32_t num_fp16_fmas_per_cy;

    /** Maximum number of bilinear filtered texels per clock per core */
    uint32_t num_texels_per_cy;

    /** Maximum number of output pixels per clock per core */
    uint32_t num_pixels_per_cy;
};


/** Kbase ioctl interface type. */
enum class iface_type {
    /** Pre R21 kernel */
    pre_r21,
    /** Post R21 kernel (inclusive) */
    post_r21
};

/**
 * Mali device driver instance.
 */
class instance
{
public:
    /**
     * Factory function to create a device instance.
     *
     * @param id   The driver instance, e.g. 0 for /dev/mali0.
     *
     * @return The created instance, or @c nullptr on failure.
     */
    static std::unique_ptr<instance> create(const uint32_t id=0);

    /**
     * Get the GPU device property information.
     *
     * The returned reference has the same lifetime as the instance.
     *
     * @return The device property information.
     */
    const gpuinfo& get_info() const;

    /**
     * Destroy an instance.
     *
     * Any returned information references become invalid.
     */
    ~instance();

private:
    /**
     * Create a new instance.
     *
     * @param fd   The opened driver file descriptor.
     *
     */
    instance(int fd);

    /** Check the Mali kernel driver interface version. */
    bool check_version();

    /** Configure Mali kernel driver connection flags. */
    bool set_flags();

    /** Query properties and store them locally. */
    bool init_props();

    /** Get device constants from the old format ioctl. */
    bool init_props_pre_r21();

    /** Get device constants from the new format ioctl. */
    bool init_props_post_r21();

    /** The queries device properties. */
    gpuinfo info_ {};

    /** The driver interface type. */
    iface_type iface_ {};

    /** The validity state of the object if initialization fails. */
    bool valid_ { true };

    /** The kernel driver file descriptor. */
    int fd_ {};
};

/** Kbase Pre R21 ioctl interface. */
namespace kbase_pre_r21 {

/** Related to mali0 ioctl interface */
enum class header_id : uint32_t {
    /** Version check. */
    version_check = 0,
    /** Base Context Create Kernel Flags. */
    create_kernel_flags = 2,
    /** Kbase Func Get Props. */
    get_props = 526,
    /** Kbase Func Set Flags. */
    set_flags = 530,
};

/** Message header. */
union uk_header {
    /** Number identifying the called UK function. */
    header_id id;
    /** The return code of the called UK function. */
    uint32_t ret;
    /** Dummy to ensure type has 64-bit alignment */
    uint64_t sizer;
};

/** Check version compatibility between kernel and userspace. */
struct version_check_t {
    /** UK header */
    uk_header header;
    /** Major version number */
    uint16_t major;
    /** Minor version number */
    uint16_t minor;

    bool is_set() const
    {
        return major || minor;
    }
};

/** IOCTL parameters to set flags */
struct set_flags_t {
    /** UK header */
    uk_header header;
    /** Create flags */
    uint32_t create_flags;
    /** Padding */
    uint32_t padding;
};

/** Base GPU Num Texture Features Registers. */
static constexpr const uint32_t base_gpu_num_texture_features_registers = 3;

/** Base Max Coherent Groups. */
static constexpr const uint32_t base_max_coherent_groups = 16;

/** GPU Max Job Slots. */
static constexpr const uint32_t gpu_max_job_slots = 16;

/** Kbase UK GPU props. */
struct uk_gpuprops_t {
    /**
     * IOCTL parameters to probe GPU properties
     *
     * NOTE: the raw_props member in this data structure contains the register
     * values from which the value of the other members are derived. The derived
     * members exist to allow for efficient access and/or shielding the details
     * of the layout of the registers.
     *
     */
    struct gpu_props {
        /** Core. */
        struct core {
            /** Product specific value. */
            uint32_t product_id;
            /**
             * Status of the GPU release.
             * No defined values, but starts at 0 and increases by one for each
             * release status (alpha, beta, EAC, etc.).
             * 4 bit values (0-15).
             */
            uint16_t version_status;
            /**
             * Minor release number of the GPU. "P" part of an "RnPn" release number.
             * 8 bit values (0-255).
             */
            uint16_t minor_revision;
            /**
             * Major release number of the GPU. "R" part of an "RnPn" release number.
             * 4 bit values (0-15).
             */
            uint16_t major_revision;
            /** Padding. */
            uint16_t padding;
            /**
             * This property is deprecated since it has not contained the real current
             * value of GPU clock speed. It is kept here only for backwards compatibility.
             * For the new ioctl interface, it is ignored and is treated as a padding
             * to keep the structure of the same size and retain the placement of its
             * members.
             */
            uint32_t gpu_speed_mhz;
            /**
             * @usecase GPU clock max speed is required for computing best case
             * in tasks as job scheduling ant irq_throttling. (It is not specified in the
             * Midgard Architecture).
             * Also, GPU clock max speed is used for OpenCL's clGetDeviceInfo() function.
             */
            uint32_t gpu_freq_khz_max;
            /**
             * @usecase GPU clock min speed is required for computing worst case
             * in tasks as job scheduling ant irq_throttling. (It is not specified in the
             * Midgard Architecture).
             */
            uint32_t gpu_freq_khz_min;
            /** Size of the shader program counter, in bits. */
            uint32_t log2_program_counter_size;
            /**
             * TEXTURE_FEATURES_x registers, as exposed by the GPU. This is a
             * bitpattern where a set bit indicates that the format is supported.
             *
             * Before using a texture format, it is recommended that the corresponding
             * bit be checked.
             */
            uint32_t texture_features[base_gpu_num_texture_features_registers];
            /**
             * Theoretical maximum memory available to the GPU. It is unlikely that a
             * client will be able to allocate all of this memory for their own
             * purposes, but this at least provides an upper bound on the memory
             * available to the GPU.
             *
             * This is required for OpenCL's clGetDeviceInfo() call when
             * CL_DEVICE_GLOBAL_MEM_SIZE is requested, for OpenCL GPU devices. The
             * client will not be expecting to allocate anywhere near this value.
             */
            uint64_t gpu_available_memory_size;
        };

        /**
         * More information is possible - but associativity and bus width are not
         * required by upper-level apis.
         */
        struct l2_cache {
            /** Log2 Line Size. */
            uint8_t log2_line_size;
            /** Log2 Cache Size. */
            uint8_t log2_cache_size;
            /** Num L2 Slices. */
            uint8_t num_l2_slices;
            /** Padding bytes. */
            uint8_t padding[5];
        };

        /** Tiler. */
        struct tiler {
            /** Max is 4*2^15 */
            uint32_t bin_size_bytes;
            /** Max is 2^15 */
            uint32_t max_active_levels;
        };

        /** GPU threading system details. */
        struct thread {
            /** Max. number of threads per core */
            uint32_t max_threads;
            /** Max. number of threads per workgroup */
            uint32_t max_workgroup_size;
            /** Max. number of threads that can synchronize on a simple barrier */
            uint32_t max_barrier_size;
            /** Total size [1..65535] of the register file available per core. */
            uint16_t max_registers;
            /** Max. tasks [1..255] which may be sent to a core before it becomes blocked. */
            uint8_t max_task_queue;
            /** Max. allowed value [1..15] of the Thread Group Split field. */
            uint8_t max_thread_group_split;
            /** 0 = Not specified, 1 = Silicon, 2 = FPGA, 3 = SW Model/Emulation */
            uint8_t impl_tech;
            /** Padding bytes. */
            uint8_t padding[7];
        };

        /**
         * A complete description of the GPU's Hardware Configuration Discovery
         * registers.
         *
         * The information is presented inefficiently for access. For frequent access,
         * the values should be better expressed in an unpacked form in the
         * base_gpu_props structure.
         *
         * @usecase The raw properties in @ref gpu_raw_gpu_props are necessary to
         * allow a user of the Mali Tools (e.g. PAT) to determine "Why is this device
         * behaving differently?". In this case, all information about the
         * by the driver</b>. Instead, the raw registers can be processed by the Mali
         * Tools software on the host PC.
         */
        struct raw {
            /** Shader Present. */
            uint64_t shader_present;
            /** Tiler Present. */
            uint64_t tiler_present;
            /** L2 Present. */
            uint64_t l2_present;
            /** Unused 1. */
            uint64_t unused_1;
            /** L2 Features. */
            uint32_t l2_features;
            /** Suspend Size. */
            uint32_t suspend_size;
            /** Mem Features. */
            uint32_t mem_features;
            /** Mmu Features. */
            uint32_t mmu_features;
            /** As Present. */
            uint32_t as_present;
            /** Js Present. */
            uint32_t js_present;
            /** Js Features. */
            uint32_t js_features[gpu_max_job_slots];
            /** Tiler Features. */
            uint32_t tiler_features;
            /** Texture Features. */
            uint32_t texture_features[3];
            /** GPU ID. */
            uint32_t gpu_id;
            /** Thread Max Threads. */
            uint32_t thread_max_threads;
            /** Thread Max Workgroup Size. */
            uint32_t thread_max_workgroup_size;
            /** Thread Max Barrier Size. */
            uint32_t thread_max_barrier_size;
            /** Thread Features. */
            uint32_t thread_features;
            /**
             * Coherency Mode.
             * Note: This is the _selected_ coherency mode rather than the
             * available modes as exposed in the coherency_features register.
             */
            uint32_t coherency_mode;
        };

        /**
         * Coherency group information
         *
         * Note that the sizes of the members could be reduced. However, the \c group
         * member might be 8-byte aligned to ensure the u64 core_mask is 8-byte
         * aligned, thus leading to wastage if the other members sizes were reduced.
         *
         * The groups are sorted by core mask. The core masks are non-repeating and do
         * not intersect.
         */
        struct coherent_group_info {
            /**
             * descriptor for a coherent group
             *
             * \c core_mask exposes all cores in that coherent group, and \c num_cores
             * provides a cached population-count for that mask.
             *
             * @note Whilst all cores are exposed in the mask, not all may be available to
             * the application, depending on the Kernel Power policy.
             *
             * @note if u64s must be 8-byte aligned, then this structure has 32-bits of
             * wastage.
             */
            struct coherent_group {
                /** Core restriction mask required for the group */
                uint64_t core_mask;
                /** Number of cores in the group */
                uint16_t num_cores;
                /** Padding bytes. */
                uint16_t padding[3];
            };

            /** Num Groups. */
            uint32_t num_groups;
            /**
             * Number of core groups (coherent or not) in the GPU. Equivalent to the number of
             * L2 Caches.
             * The GPU Counter dumping writes 2048 bytes per core group, regardless of whether
             * the core groups are coherent or not. Hence this member is needed to calculate
             * how much memory is required for dumping.
             * @note Do not use it to work out how many valid elements are in the group[]
             * member. Use num_groups instead.
             */
            uint32_t num_core_groups;
            /** Coherency features of the memory, accessed by @ref gpu_mem_features methods. */
            uint32_t coherency;
            /** Padding. */
            uint32_t padding;
            /** Descriptors of coherent groups */
            coherent_group group[base_max_coherent_groups];
        };

        /** Core Props. */
        core core_props;
        /** L2 Props. */
        l2_cache l2_props;
        /** Unused to keep for backwards compatibility. */
        uint64_t unused;
        /** Tiler Props. */
        tiler tiler_props;
        /** Thread Props. */
        thread thread_props;
        /** This member is large, likely to be 128 bytes. */
        raw raw_props;
        /** This must be last member of the structure. */
        coherent_group_info coherency_info;
    };

    /** Header. */
    uk_header header;
    /** Props. */
    gpu_props props;
};

constexpr auto iface_number = 0x80;

/** Commands describing kbase_pre_r21 ioctl interface. */
enum command_type {
    /** Check version compatibility between JM kernel and userspace. */
    version_check = _IOWR(iface_number, 0x0, version_check_t),
    /** Set kernel context creation flags. */
    set_flags = _IOWR(iface_number, 0x212, set_flags_t),
    /** Get GPU properties. */
    get_gpuprops = _IOWR(iface_number, 0x20e, uk_gpuprops_t),
};

}

/** Kbase Post R21 ioctl interface. */
namespace kbase_post_r21 {

template <typename value_t>
class pointer64 {
  public:
    /** @return Pointer to the object. */
    value_t* get() const {
        return reinterpret_cast<value_t*>(static_cast<uintptr_t>(value));
    }

    /**
     * Set pointer value.
     *
     * @param ptr   The new pointer value.
     */
    void reset(value_t* ptr) {
        value = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(ptr));
    }

  private:
    /** Pointer value as uint64_t. */
    uint64_t value { 0 };
};

/** Check version compatibility between kernel and userspace. */
struct version_check_t {
    /** Major version number. */
    uint16_t major;
    /** Minor version number */
    uint16_t minor;

    bool is_set() const
    {
        return major || minor;
    }
};

/** Set kernel context creation flags. */
struct set_flags_t {
    /** kernel context creation flags. */
    uint32_t create_flags;
};

/**
 * The ioctl will return the number of bytes stored into buffer or an error
 * on failure (e.g. size is too small). If size is specified as 0 then no
 * data will be written but the return value will be the number of bytes needed
 * for all the properties.
 *
 * flags may be used in the future to request a different format for the
 * buffer. With flags == 0 the following format is used.
 *
 * The buffer will be filled with pairs of values, a __u32 key identifying the
 * property followed by the value. The size of the value is identified using
 * the bottom bits of the key. The value then immediately followed the key and
 * is tightly packed (there is no padding). All keys and values are
 * little-endian.
 *
 * 00 = __u8
 * 01 = __u16
 * 10 = __u32
 * 11 = __u64
 */
struct get_gpuprops_t {
    /** GPU property size. */
    enum class gpuprop_size : uint8_t {
        /** Property type is uint8_t. */
        uint8 = 0x0,
        /** Property type is uint16_t. */
        uint16 = 0x1,
        /** Property type is uint32_t. */
        uint32 = 0x2,
        /** Property type is uint64_t. */
        uint64 = 0x3
    };

    /** GPU properties codes. */
    enum class gpuprop_code : uint8_t {
        /** Product id. */
        product_id = 1,
        /** L2 log2 line size. */
        l2_log2_line_size = 13,
        /** L2 log2 cache size. */
        l2_log2_cache_size = 14,
        /** L2 num l2 slices. */
        l2_num_l2_slices = 15,
        /** Max threads. */
        max_threads = 18,
        /** Max registers. */
        max_registers = 21,
        /** Raw l2 features. */
        raw_l2_features = 29,
        /** Raw core features. */
        raw_core_features = 30,
        /** Raw thread max threads. */
        raw_thread_max_threads = 56,
        /** Raw thread max workgroup size. */
        raw_thread_max_workgroup_size = 57,
        /** Raw thread max barrier size. */
        raw_thread_max_barrier_size = 58,
        /** Raw thread features. */
        raw_thread_features = 59,
        /** Raw coherency mode. */
        raw_coherency_mode = 60,
        /** Coherency num groups. */
        coherency_num_groups = 61,
        /** Coherency num core groups. */
        coherency_num_core_groups = 62,
        /** Coherency coherency. */
        coherency_coherency = 63,
        /** Coherency group 0. */
        coherency_group_0 = 64,
        /** Coherency group 1. */
        coherency_group_1 = 65,
        /** Coherency group 2. */
        coherency_group_2 = 66,
        /** Coherency group 3. */
        coherency_group_3 = 67,
        /** Num exec engines. */
        num_exec_engines = 82
    };

    /** Pointer to the buffer to store properties into. */
    pointer64<uint8_t> buffer;

    /** Size of the buffer. */
    uint32_t size;

    /** Flags - must be zero for now. */
    uint32_t flags;
};

constexpr auto iface_number = 0x80;

/** Commands describing kbase ioctl interface. */
enum command_type {
    /** Check version compatibility between JM kernel and userspace. */
    version_check_jm = _IOWR(iface_number, 0x0, version_check_t),
    /** Check version compatibility between CSF kernel and userspace. */
    version_check_csf = _IOWR(iface_number, 0x34, version_check_t),
    /** Set kernel context creation flags. */
    set_flags = _IOW(iface_number, 0x1, set_flags_t),
    /** Get GPU properties. */
    get_gpuprops = _IOW(iface_number, 0x3, get_gpuprops_t),
};

}
}
