# Release change log

This page summarizes the major functional changes in each release.

<!-- ---------------------------------------------------------------------- -->
## 1.2.0

**Released:** November 2024

This is small feature releases, adding support for reporting GPU architecture
version numbers as an alternative to parsing product names.

* **General:**
  * **Feature:** C++ namespace changed to `libarmgpuinfo`.
  * **Feature:** Supports reporting architecture major/minor versions.

<!-- ---------------------------------------------------------------------- -->
## 1.1.0

**Released:** June 2024

This is small feature releases, adding support for new Arm GPUs and a few new
GPU configuration values.

* **General:**
  * **Feature:** Supports Immortalis-G925 series hardware.
  * **Feature:** Supports new Mali-G310 and Mali-G510 IP configurations.
  * **Feature:** Supports reporting shader core topology mask.


<!-- ---------------------------------------------------------------------- -->
## 1.0.0

**Released:** June 2023

The first release of libGPUInfo.

* **General:**
  * **Feature:** Support IP from Mali-T720 (Midgard architecture) through to
    Immortalis-G720 (5th Generation architecture).
  * **Feature:** Supports querying GPU model number and name.
  * **Feature:** Supports querying GPU shader core and cache configuration.
  * **Feature:** Supports querying GPU speed-of-light performance metrics.
  * **Feature:** Command line utility provided for easy device testing.

- - -

_Copyright © 2023-2024, Arm Limited and contributors._
