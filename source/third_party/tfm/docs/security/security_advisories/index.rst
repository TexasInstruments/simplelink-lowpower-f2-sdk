###################
Security Advisories
###################

.. toctree::
    :maxdepth: 1
    :hidden:

    stack_seal_vulnerability
    svc_caller_sp_fetching_vulnerability
    crypto_multi_part_ops_abort_fail
    profile_small_key_id_encoding_vulnerability
    fwu_write_vulnerability

+------------+-----------------------------------------------------------------+
| ID         | Title                                                           |
+============+=================================================================+
|  |TFMV-1|  | NS world may cause the CPU to perform an unexpected return      |
|            | operation due to unsealed stacks.                               |
+------------+-----------------------------------------------------------------+
|  |TFMV-2|  | Invoking Secure functions from handler mode may cause TF-M IPC  |
|            | model to behave unexpectedly.                                   |
+------------+-----------------------------------------------------------------+
|  |TFMV-3|  | ``abort()`` function may not take effect in TF-M Crypto         |
|            | multi-part MAC/hashing/cipher operations.                       |
+------------+-----------------------------------------------------------------+
|  |TFMV-4|  | NSPE may access secure keys stored in TF-M Crypto service       |
|            | in Profile Small with Crypto key ID encoding disabled.          |
+------------+-----------------------------------------------------------------+
|  |TFMV-5|  | ``psa_fwu_write()`` may cause buffer overflow in SPE.           |
+------------+-----------------------------------------------------------------+

.. |TFMV-1| replace:: :doc:`TFMV-1 <stack_seal_vulnerability>`
.. |TFMV-2| replace:: :doc:`TFMV-2 <svc_caller_sp_fetching_vulnerability>`
.. |TFMV-3| replace:: :doc:`TFMV-3 <crypto_multi_part_ops_abort_fail>`
.. |TFMV-4| replace:: :doc:`TFMV-4 <profile_small_key_id_encoding_vulnerability>`
.. |TFMV-5| replace:: :doc:`TFMV-5 <fwu_write_vulnerability>`

--------------

*Copyright (c) 2020-2023, Arm Limited. All rights reserved.*
