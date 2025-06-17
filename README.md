# About

libGPUInfo is a small utility library that allows applications to query the
configuration of the Arm® Immortalis™ or Arm Mali™ GPU present in the system.
This information allows developers to adjust application workload complexity to
match the performance capability of the current device.

This library is able to provide the Arm GPU hardware configuration, as well as
performance metrics for the shader cores inside the GPU. The library is unable
to provide system information, such as the available GPU clock frequencies,
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

## Recent changes

* Change log: [1.x series](./docs/changelog.md)

## Related API extensions

This library is intended to support any Arm device, but some developers prefer
to use functionality within the graphics API when it is available. New devices
can report a similar set of information to this library using in-API queries.

We recommend using the extensions on devices where it is available. Doing so
means the application automatically gets up-to-date information for all
devices, even those released after the application binary was built.

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
* **Architecture version:** The product architecture version, e.g. 9.2.
* **Model number:** The product ID number, e.g. 0xa002.
* **Shader core count:** The number of shader cores in the design.
* **Shader core mask:** The shader core topology mask.
* **L2 cache count:** The number of L2 cache slices in the design.
* **L2 cache size:** The total L2 cache size, summed over all slices, in bytes.
* **Bus size:** The width of the external data bus, per cache slice, in bits.

The query mechanism can report the following per-core shader core performance
information:

* **Execution engine count:** The number of arithmetic macroblocks.
* **FP32 FMA count:** The peak fp32 FMAs per clock, summed over all engines.
* **FP16 FMA count:** The peak fp16 FMAs per clock, summed over all engines.
* **Texel count:** The peak bilinear filtered texture samples per clock.
* **Pixel count:** The peak pixels per clock.

# Using the library

The library is very simple to use:

```C++
// Create a connection with the kernel driver ...
std::unique_ptr<instance> conn = libarmgpuinfo::instance::create();
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

## Handling unknown devices

The library will be regularly updated to support new Arm GPU products, but it
is inevitable that applications will run on new devices with GPU models that
did not exist at the time they were released. For this there are two failure
modes that applications must consider.

The most likely error is the case where a connection can be established with
the Arm kernel driver, but the product code is unknown. In this case the call
to `libarmgpuinfo::instance::create()` will succeed but return a partially
populated result. It will include any information that can be determined
programmatically, but will report the GPU name and architecture as "Unknown",
and the per-core shader core performance metrics as zero.

For example, we can currently show the following information when the product
model is not explicitly supported:

```yaml
GPU configuration:
  Model number: 0xa862
  Core count: 7
  L2 cache count: 4
  Total L2 cache size: 2097152 bytes
  Bus width: 256 bits
```

If the kernel driver interface has changed and the library cannot establish a
connection then we can return no useful information. In this case the
`libarmgpuinfo::instance::create()` function will fail and will return a
`nullptr`.

# Building

The library is provided as a single C++ source file and a single C++ header
file. It is expected that developers will copy the files directly into their
existing application build system, so no off-the-shelf build system is provided
for the library integration.

# Sample application

The repository also contains a simple command line tool that demonstrates use of
the API, and which can be used for adhoc testing of devices. To build the
Android command line tool:

* Set `ANDROID_NDK_HOME` to the path of your Android NDK install.
* Run `./android_build.sh [Release|Debug]`.

The output binary will be `./bin/arm_gpuinfo`. You can run this on the device
and print the results for your device to the terminal using the following
commands:

```sh
adb push ./bin/arm_gpuinfo /data/local/tmp
adb shell chmod u+x /data/local/tmp/arm_gpuinfo
adb shell /data/local/tmp/arm_gpuinfo
adb shell rm /data/local/tmp/arm_gpuinfo
```

The generated output is formatted using a YAML-like syntax, but is designed for
human consumption with additional line breaks. To generate strictly compliant
YAML output for use in scripts pass the `--yaml` or `-y` argument on the
`arm_gpuinfo` command line.

# Support

If you have issues with the library itself, please raise them in the project's
GitHub issue tracker.

If you have any questions about Arm GPUs, application development for Arm GPUs,
or general mobile graphics development or technology please submit them on the
[Arm Community graphics forums][1].

- - -

_Copyright © 2023-2025, Arm Limited and contributors._

[1]: https://community.arm.com/support-forums/f/graphics-gaming-and-vr-forum/
[2]: https://developer.arm.com/documentation/102849/latest/
[3]: https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_ARM_shader_core_properties.html
[4]: https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_ARM_shader_core_builtins.html
