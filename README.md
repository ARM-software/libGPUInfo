# About

libGPUInfo is a small utility library that allows applications to query the
configuration of the Arm® Immortalis™ or Arm Mali™ GPU present in the system.
This information allows developers to adjust application workload complexity to
match the performance capability of the current device.

> NOTE: This library is still undergoing development, but we expect the first
> version to be released soon ...

## What information

This library is only able to provide information about the GPU hardware
configuration itself. It is not possible to determine the available range of
GPU clock frequencies, as this is handled outside of the GPU itself.

For documentation about the capabilities of the various Arm GPUs you can
refer to the [Arm GPU Datasheet][2].

## Supported devices

This library supports all Arm GPU products from the Mali-T700 series onwards,
ensuring developers have coverage of the vast majority of smartphones with
Arm GPUs that are in use today.

This library supports devices using the Arm commercial driver.

## License

This project is licensed under the MIT license.  By downloading any component
from this repository you acknowledge that you accept terms specified in the
[LICENSE.txt](LICENSE.txt) file.

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
