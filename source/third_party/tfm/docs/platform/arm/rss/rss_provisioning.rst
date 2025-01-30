RSS provisioning
================

Provisioning theory
-------------------

The LifeCycle Manager (LCM) controls access to the RSS OTP, and includes a
state-machine that controls Lifecycle-state (LCS) transitions. The LCM is
derived from the OTP management and state machine subsystems of the
CryptoCell-3XX series accelerators, and will be familiar to those who have
worked with them.

When the chip hasn't been provisioned, the OTP is blank, which means the LCM is
in "Virgin" Test/Production mode "TP mode" state. The first step for
provisioning must be to set the LCM to either test-chip mode "TCI" or
production-chip mode "PCI". In TCI mode the RTL key is masked to avoid
disclosure, several OTP fields are changed from write-only to read-write, to aid
in debugging, and debugging is not limited in secure provisioning mode (though
the RTL key reads as zero). If ``TFM_DUMMY_PROVISIONING`` is enabled in cmake
(which it is by default) then the chip will be set to TCI mode. If this option
is not enabled, execution will pause to allow the setting to be set by a
debugger.

Once the TP mode has been set, the chip is then is Chip Manufacturer
provisioning state "CM". This mode is intended for the provisioning of the HUK,
GUK, CM provisioning key, CM code-encryption key, the root-of-trust public key
and the CM config. To provision these fields, The firmware must first receive a
provisioning bundle via a debugger. This bundle must be placed at the start of
VM0. This bundle contains the keys and also code to perform the provisioning
such as a driver for the LCM, and a function to randomly generate the HUK via
the CryptoCell TRNG. The chip must then enter secure provisioning mode by
setting the SP_ENABLE register. This causes a reset (but does not clear the RSS
SRAMs), and allows access to the RTL key by exporting it to the KMU, though in
secure provisioning mode the ability to debug the RSS is disabled, to prevent
disclosure of the decrypted provisioning bundle values. The RSS will then
decrypt and authenticate the bundle using the RTL key. Under TCI mode the RTL
key is zeroed, the bundle generation tool must use a zeroed key to encrypt and
sign the bundle. Once the CM provisioning bundle has been unpacked, the RSS will
execute the code which will provision the CM provisioning data into OTP. The RSS
must be cold-reset, which will disable secure provisioning mode. If
``TFM_DUMMY_PROVISIONING`` is enabled the reset will happen automatically, else
the external provisioning device should read the provisioning state from the
GPIO/PSI (which is set via the ``rss_sysctrl`` register) and perform the reset.

After the cold reset, the RSS will automatically transition to Device
Manufacturer provisioning state "DM" as the LCM hardware state-machine reads the
values of the cm_config_1 and cm_config_2 fields as non-zero. This state is
designed to provision the DM provisioning key, the DM code-encryption key and
the DM config. The procedure follows the same steps as the CM provisioning flow,
with the exception that the bundle will now be encrypted and signed using the CM
provisioning key and must be placed at the base of VM1. As before, once the
provisioning bundle has been unpacked/run, the RSS must either be cold-reset or
will perform this automatically.

After the cold reset, the device will now be in Secure Enable "SE" mode, due to
the dm_config_1 field being non-zero. Debug may be limited based on the hardware
DCU mask for SE state. Provisioning will not be run on boot.

Practical RSS provisioning
--------------------------

The RSS buildsystem produces two provisioning bundles (containing both code and
data), and then encrypts and signs them with the RTL key to produce
``encrypted_cm_provisioning_bundle.bin`` and
``encrypted_dm_provisioning_bundle.bin``.

.. Note::
   Currently encrypted provisioning bundles are not supported due to a lack of
   AEAD encryption support in the cc312-rom-lib driver. The
   encrypted_*_provisioning_bundle.bin files should still be used, but note that
   their contents are not encrypted.

On first boot, the RSS is in Virgin state. If the RSS firmware was built with
``TFM_DUMMY_PROVISIONING`` enabled then it will automatically set the chip to
TCI mode and cold-reset. Production ROM implementations must disable
``TFM_DUMMY_PROVISIONING``, which will cause RSS to loop in the ROM until either
TCI or PCI mode is set with a debugger. It is possible to set the TP mode in the
LCS registers directly, however it may be easier to set the ``tp_mode`` variable
in the frame where RSS is looping, at which point the loop will exit and the TP
mode will be set by the ROM code.

On non-virgin boot in CM lifecycle state, RSS checks the start of VM0 for the
magic constant ``0xC0DEFEED``, which is required to be the first word in the CM
provisioning bundle. There is also a second check for a constant at the end of
the bundle to ensure the bundle has finished writing. The RSS will perform this
check in a loop until a bundle is found.

This procedure is repeated for DM LCS, except that the magic constant is
``0xBEEFFEED`` and the bundle must be loaded to the base of VM1. Note that the
size of RSS memory may vary depending on implementation, so the load address of
the DM bundle may change.

In production systems it is intended that these bundles are loaded by a
debugger, but for development systems it may be too onerous to perform this
procedure, particularly if the system is one that has ephemeral OTP such as an
FVP. The preferred solution is to preload to the provisioning bundles into VM0
and VM1 as part of the image loading, which is supported on FVPs but may not be
on other systems. An alternative solution is to perform provisioning manually
once, and then to save the state of the OTP in SE LCS and then preload that on
subsequent boots.

RSS provisioning GPIO signalling
--------------------------------
The state of the RSS ROM boot/provisioning flow is signalled outside of the RSS
subsystem via the GPIOs as part of the Persistent State Interface (PSI). The PSI
signals the lifecycle state as a hardware signal, but additionally the software
can signal over the PSI by setting the ``rss_sysctrl`` register.

The boot state is encoded in the lowest 4 bits of the ``rss_sysctrl`` register,
and has meaning as follows:

+--------+------------------------------------------------------------------+
| Signal | State                                                            |
+========+==================================================================+
| 0x0    | RSS cold boot default                                            |
+--------+------------------------------------------------------------------+
| 0x1    | Virgin chip idle, ready to set PCI/TCI mode                      |
+--------+------------------------------------------------------------------+
| 0x2    | CM LCS idle, waiting for CM provisioning bundle                  |
+--------+------------------------------------------------------------------+
| 0x3    | RMA LCS idle                                                     |
+--------+------------------------------------------------------------------+
| 0x4    | CM secure provisioning started, secure provisioning mode enabled |
+--------+------------------------------------------------------------------+
| 0x5    | CM secure provisioning failed due to bundle authentication error |
+--------+------------------------------------------------------------------+
| 0x6    | CM secure provisioning failed due to other error                 |
+--------+------------------------------------------------------------------+
| 0x7    | CM secure provisioning succeeded                                 |
+--------+------------------------------------------------------------------+
| 0x8    | DM LCS idle, waiting for CM provisioning bundle                  |
+--------+------------------------------------------------------------------+
| 0x9    | DM secure provisioning started, secure provisioning mode enabled |
+--------+------------------------------------------------------------------+
| 0xa    | DM secure provisioning failed due to bundle authentication error |
+--------+------------------------------------------------------------------+
| 0xb    | DM secure provisioning failed due to other error                 |
+--------+------------------------------------------------------------------+
| 0xc    | DM secure provisioning succeeded                                 |
+--------+------------------------------------------------------------------+
| 0xd    | SE LCS standard boot                                             |
+--------+------------------------------------------------------------------+

--------------

*Copyright (c) 2022-2023, Arm Limited. All rights reserved.*
