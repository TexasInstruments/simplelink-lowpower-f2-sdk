###################################
Trusted Firmware-M Source Structure
###################################

:Organization: Arm Limited
:Contact: tf-m@lists.trustedfirmware.org

.. note::
  Refernce the document :doc:`Glossary </docs/reference/glossary>` for terms and
  abbreviations.

************
Introduction
************
TF-M is designed to provide a user-friendly source structure to easy
integration and service development. This document introduces the source
structure and its design intention of TF-M.

.. note::
  - This document goes ahead of the implementation so the existing source
    structure may be different from the description listed here. Staging
    changes must reference this document since it is the goal of structure
    design.
  - Related non-source content like libraries, scripts and binaries are also
    introduced but briefly since this document focuses more on the source
    code.

****************
Source Structure
****************
The description of the source structure is broken down into subsections, each
subsection introduces one detailed folder.

Root Direcotry
==============
This table describes the structure under the root directory. The item with a
`No` in the `Detailed` field is briefly introduced but not detailed into
subsections.

============= ==================================== ====================
Folder name   Purpose                              Detailed
============= ==================================== ====================
bl2           The 2nd stage bootloader.            No. [1]
cmake         Cmake files.                         No. [2]
configs       Configuration system files.          No. [2]
docs          The documentations.                  No. [2]
lib           The 3rd party library.               No. [1]
**platform**  Platform intermedia files.           See `'platform'`_.
**secure_fw** The secure firmware.                 See `'secure_fw'`_.
**test**      Tests.                               See `'test'`_.
tools         Tools in scripts for building.       No. [2]
============= ==================================== ====================

.. note::
  1. 3rd party project structure is not introduced.
  2. Non-source content maybe not listed here, or introduction is skipped even
     they are listed.

'platform'
==========
The platform sources contain necessary platform sources or intermedia files
pointing to the sources out of TF-M folder.

========================= =============================================
Folder name               Purpose
========================= =============================================
common/\*                 Common HAL implementation.
<vendor>                  Vendor specific folder.
========================= =============================================

.. note::
  The ``common`` folder contains the example implementation of the ``HAL`` API
  bases on common ``system`` (CMSIS e.g.). The platform could reference to
  sources in the ``common`` folder directly if applicable or apply a
  customized HAL implementation.
  A ``system`` can have a standalone folder under ``common``, depends on how
  'common' this system is. Each ``platform vendor`` is assigned with one
  folder for usage. As the content of the ``vendor`` folder comes by the
  contribution of vendors, the ``vendor`` manages the structure inside it.

'secure_fw'
===========
This folder contains components needed by secure firmware, and the exported
interfaces for application, service development and HAL integration:

============== =========================================== ====================
Folder name    Purpose                                     Detailed
============== =========================================== ====================
**spm**        Generic SPM sources.                        See `'spm'`_
**include**    Interfaces for PSA FF and TF-M generic.     See `'include'`_
**partitions** Default services and supportings.           See `'partitions'`_
**shared**     Sources not only for secure firmware usage. See `'shared'`_
============== =========================================== ====================

'spm'
-----
The SPM is the core component to provide a mechanism for providing secure
services.

=================================== ===========================================
Folder name                         Purpose
=================================== ===========================================
arch/\*                             Architecture sources. [1]
include/\*                          SPM public headers.
include/hal\*                       SPM public headers for HAL.
init/*                              SPM initializing entry. [2]
model_ipc/\*                        PSA-FF-M SPM implementation. [3]
model_func/\*                       The library model SPM implementation. [4]
runtime/\*                          SPM runtime utilities. [5]
services/\*                         PRoT Services not in partitions. [6]
=================================== ===========================================

.. note::
  1. The Architecture source focus on context-related operations.
  2. SPM startup code.
  3. A PSA-FF-M-complied SPM implementation.
  4. A customized library model SPM implementation.
  5. The SPM runtime utilities such as memory managing and scheduling.
  6. The services can be implemented as Secure Partitions or partition-less PSA
     RoT Services. Here puts the partition-less services.

  The private headers for each component in this folder are
  development-defined. Do not mix private headers with public headers into the
  same folder.

'include'
---------
This folder holds headers for client, services and platform integration usage.

=========================== =============================================
Folder name                 Purpose
=========================== =============================================
psa/\*.h                    PSA FF headers.
tfm/\*.h                    TF-M specific feature headers.
=========================== =============================================

.. note::
  TFM applies an explicit including policy. Each module's headers are put into
  a specific folder which follows the pattern '\*/include', this folder needs
  to be added into the project's searching path if this project needs to
  include headers in this folder. The 'include' in a path indicates the end of
  including-path. If there are subfolders under folder 'include', apply a
  relative hierarchy including.

'partitions'
------------
This folder contains default services implemented as partitions and necessary
partition runtime utilities provided by TF-M.

================================= =============================================
Folder name                       Purpose
================================= =============================================
lib/sprt/\*                       The SPRTL sources and intermedia files. [1]
<partition_x>/\*                  Files for 'partition_x'.
<partition_x>/export/include/\*.h RoT Service API headers of 'partition_x'. [2]
<partition_x>/export/src/\*.c     RoT Service API sources of 'partition_x'. [2]
<partition_y>/\*                  Files for 'partition_y'.
<partition_y>/export/include/\*.h RoT Service API headers of 'partition_y'. [2]
<partition_y>/export/src/\*.c     RoT Service API sources of 'partition_y'. [2]
================================= =============================================

.. note::
  1. The SPRTL sources and intermediate files. SPRTL contains sources from
     other folders, such as necessary RoT Service API implementation from
     'partitions' folder.
  2. Here takes 'partition_x' and 'partition_y' as an example, and showcases
     a detailed structure of them.

'shared'
--------
Here place the sources which are needed by SPE components and shared between
multiple components (includes NSPE components).

========================= =============================================
Folder name               Purpose
========================= =============================================
shared/psa/\*.c           Shared PSA FF sources.
shared/tfm/\*.c           Shared TF-M feature sources.
========================= =============================================

.. note::
  Examples for the shared sources:
    The parameter packing for ``psa_call()`` is same between NSPE clients and
    the secure clients, the only difference is the final call. In a TrustZone
    implementation it is an 'SG' for NSPE while for secure clients it can be a
    'SVC'. So this ``psa_call`` implementation under 'psa/' can be shared
    between NSPE clients and the secure clients. So does the TF-M customized
    feature sources under the 'tfm/' direcotry.

'test'
======
The NSPE examples are for test-purpose-specific.

============================== ===========================================
Folder name                    Purpose
============================== ===========================================
test/nspe/rtx/\*               RTX - the example NSPE instance. [1]
test/*                         Other test related sources. [2]
============================== ===========================================

.. note::
  1. This is the example NSPE instance.
  2. Other sources are managed by the 'test' design. The test purpose secure
     services are also put under this foler (NOT the `'partitions'`_ folder).

--------------

*Copyright (c) 2020, Arm Limited. All rights reserved.*
