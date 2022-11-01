## FreeRTOS Pre-generated Kernel Configuration

This file is a pre-generated default kernel configuration for a specific device
family. If your application is not using SysConfig, or you need a FreeRTOS
configuration for another use case (library compilation, for example), you can
use this file instead of maintaining one locally. This file will remain up to
date with the default kernel configuration, as defined by the SysConfig file in
the parent folder.

## Warning

It is possible to change the sizes of kernel objects using the SysConfig
configuration, in particular when enabling debugging features. It is critical
that all of your software has identical object sizes.

An identical FreeRTOSConfig file should be used for compilation of any software
that includes kernel objects - including object accesses or pointer dereferences
as these may be impacted by the object memory layout.
