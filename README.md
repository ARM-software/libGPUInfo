# About

libGPUInfo is a small utility library that allows applications to query the
configuration of the Arm® Immortalis™ or Arm Mali™ GPU present in the system.
This information allows developers to adjust application workload complexity to
match the performance capability of the current device.

This library is able to provide the Arm GPU hardware configuration, as well as
performance metrics for the shader cores inside the GPU. The library is unable
to provide system infomation, such as the available GPU clock frequencies,
because this is provided by the device manufacturer and is not part of the Arm
GPU itself.

For offline documentation about the capabilities of the various Arm GPUs on the
market today please refer to the [Arm GPU Datasheet][2].

## Supported devices

This library aims to support all Arm GPU products from the Mali-T700 series
onwards, ensuring developers have coverage of the vast majority of smartphones
with Arm GPUs that are in use today. If you find a device with an Arm GPU which
does not work, or gives inaccurate results, please open an Issue on the GitHub
issue tracker.

This library only supports devices using the Arm commercial driver.

## Related API extensions

This library is intended to support any Arm device, but some developers prefer
to use functionality within the graphics API when it is available. New devices
can report a similar set of information to this library using in-API queries.
For more information please refer to the extension specifications:

* [VK_ARM_shader_core_properties][3]
* [VK_ARM_shader_core_builtins][4]


## License

This project is licensed under the MIT license. By downloading any component
from this repository you acknowledge that you accept terms specified in the
[LICENSE.txt](LICENSE.txt) file.

# Available information

The query mechanism can report the following information about the GPU:

* **Name:** The product name string, e.g. "Mali-G710".
* **Architecture:** The product architecture name string, e.g. "Valhall".
* **Model number:** The product ID number, e.g. 0xa002.
* **Shader core count:** The number of shader cores in the design.
* **L2 cache count:** The number of L2 cache slices in the design.
* **L2 cache size:** The total L2 cache size, summed over all slices, in bytes.
* **Bus size:** The width of the external data bus, per cache slice, in bits.

The query mechanism can report the following per-core shader core information:

* **Execution engine count:** The number of arithmetic macroblocks.
* **FP32 FMA count:** The peak fp32 FMAs per clock, summed over all engines.
* **FP16 FMA count:** The peak fp16 FMAs per clock, summed over all engines.
* **Texel count:** The peak bilinear filtered texture samples per clock.
* **Pixel count:** The peak pixels per clock.

# Using the library

The library is very simple to use:

```C++
// Create a connection with the kernel driver ...
std::unique_ptr<instance> conn = libgpuinfo::instance::create();
if (!conn)
{
    std::cout << "ERROR: Failed to create Mali instance\n";
    return;
}

// Fetch the information result and do something with it ...
const gpuinfo& info = conn->get_info();
std::cout << "GPU: " << info.gpu_name << " MP" << info.num_shader_cores << "\n";
```

Note that the returned instance uses a unique pointer for lifetime management,
and both the instance and the query result will be freed when the instance 
drops out of scope.

# Building

The library is provided as a single C++ source file and a single C++ header
file. It is expected that developers will copy the libgpuinfo files directly
into their existing application build system, so no off-the-shelf build system
is provided for the library integration.

You can also build a simple command line tool that can be used for adhoc
testing of devices. To build the Android command line tool:

* Set `ANDROID_NDK_HOME` to the path of your Android NDK install.
* Run `./android_build.sh [Release|Debug]`.

The output binary will be `./bin/arm_gpuinfo`. You can run this on the device
and print the results for your device to the terminal using the following
commands:

```
adb push ./bin/arm_gpuinfo /data/local/tmp
adb shell chmod u+x /data/local/tmp/arm_gpuinfo
adb shell /data/local/tmp/arm_gpuinfo
adb shell rm /data/local/tmp/arm_gpuinfo
```

# Support

If you have issues with the library itself, please raise them in the project's
GitHub issue tracker.

If you have any questions about Arm GPUs, application development for Arm GPUs,
or general mobile graphics development or technology please submit them on the
[Arm Community graphics forums][1].

- - -

_Copyright © 2023, Arm Limited and contributors. All rights reserved._

[1]: https://community.arm.com/support-forums/f/graphics-gaming-and-vr-forum/
[2]: https://developer.arm.com/documentation/102849/latest/
[3]: https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_ARM_shader_core_properties.html
[4]: https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_ARM_shader_core_builtins.html
