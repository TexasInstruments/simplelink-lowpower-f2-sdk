##############################
Mailbox NS Agent Design Update
##############################

:Organization: Arm Limited
:Contact: tf-m@lists.trustedfirmware.org

**********
Background
**********
The SPE component that maintains the non-secure clients' request is called
'NS Agent' in TF-M. Besides the Trustzone-based isolation mechanism, there is
one other isolation mechanism that implements individual PEs in physically
isolated cores respectively. NSPE and SPE transfer non-secure client requests
via inter-processor communication based on mailboxes. The component that
handles inter-processor communication messages is called ``Mailbox NS Agent``.

.. note::
  There may be hardware components and software solutions containing 'mailbox'
  in their names. The concept ``mailbox`` in this document represent the
  mechanism described above, which is not referring to the external concepts.

When the first version ``Mailbox NS Agent`` was introduced, the generic FFM
interrupt handling was not ready. Hence a customized solution
``Multiple Core`` is implemented. This customized implementation:

- Perform customized operations on SPM internal data in a deferred interrupt
  handler.
- Process mailbox operations as special cases in SPM common logic.

These behaviours couple SPM tightly with mailbox logic, which bring issues for
maintenance. To address the issue, an updated design shall:

- Make SPM manage other components in a unified way (For example, it is
  simpler for SPM if all non-SPM components under the IPC model act as
  ``processes``.)
- Can use FF-M compliant interrupt mechanism and APIs.

Following the above guidelines makes the ``Mailbox NS Agent`` work like a
``partition``. The agent has an endless loop and waits for signals, calls FFM
API based on the parsing result on the communication messages. But there are
still issues after looking closer to the requirements of the agent:

- SPM treats FFM Client API caller's ID as the client ID. While the mailbox NS
  agent may represent multiple non-secure clients. Hence it needs to tell
  SPM which non-secure client it is representing, and the default FFM Client
  API does not have such capability.
- FFM Client API blocks caller before the call is replied; while the
  mailbox NS Agent needs to respond to the non-secure interrupts in time.
  Blocking while waiting for a reply may cause the non-secure communication
  message not to be handled in time.

Extra design items need to be added to address the issues.

*************
Design Update
*************
The below figure shows the overall design to cover various component types.
NS Agents are the implementation-defined components that provide FF-M compliant
Client API to the non-secure clients. Hence from the view of the non-secure
clients, the FF-M client API behaviour follows the FF-M definition. And NS
Agent needs customization in SPM since it has extra requirements compared to
a generic secure partition.

.. figure:: /design_docs/media/mailbox_ns_agent1.*
    :align: center
    :name: fig-mailbox1
    :width: 70%

    Component types and the callable API set

.. note::
  3 non-SPM component types here: FFM-compliant Secure Partition
  (aka ``partition``), Trustzone-based NS Agent (aka ``Trustzone NS Agent``)
  and mailbox-based NS Agent (aka ``Mailbox NS Agent``).
  ``Trustzone NS Agent`` is mentioned here for the comparison purpose. The
  implementation details for this NS agent type is not introduced here.

To make the programming model close to the FFM compliance, the
``Mailbox NS Agent`` is designed as:

- Working like a standard Secure Partition under the IPC model, has one
  single thread, can call FFM standard API.
- Having a manifest file to describe the attributes and resources and a
  positive valued ``Partition ID`` in the manifest.

Services rely on the ``client_id`` to apply policy-checking, hence SPM
needs to know which ``client_id`` the mailbox NS Agent is representing when
mailbox API is calling Client API. The standard API treats the caller as the
client of the service, which means that a specific API is required to
support identifying the represented non-secure client. SPM sets the non-secure
``client_id`` into the message right at the moment the message is
going to be sent. Before this point, SPM performs the call based on the
agent's ID.

This specifc ``Agent API`` is non-blocking, unlike the standard FF-M Client
APIs. This can improve the communication efficiency between NS clients and
mailbox NS agents. With this mechanism, extra signals and APIs for message
acknowledges are also required.

.. note::
  A standard Secure Partition gets errors when calling the ``Agent API``.

Updated programming interfaces
==============================
These Client APIs are expanded from the standard Client APIs:

- ``agent_psa_connect()`` is extended from ``psa_connect()``.
- ``agent_psa_call()`` is extended from ``psa_call()``.

And to cooperate with the changed behaviour of these APIs, extra defined
signals and types are also involved.

.. note::
  Namespace ``agent`` is involved for NS Agent callable API; namespace ``tfm``
  is involved for TF-M specific concepts. Even though ``agent`` is TF-M
  specific at the current stage, it is proposed to be a common concept for
  general FF-M compliant implementations, hence assigning ``agent`` for
  proposed API and data structures.

Input and output vectors
========================
When non-secure clients call ``psa_call()``, a mailbox message containing
``psa_call()`` parameters is delivered into ``Mailbox NS Agent`` and the agent
needs to extract parameters from the message and then call ``agent_psa_call()``.
Revisit the ``psa_call()`` prototype to see the parameters:

.. code-block:: c

  psa_status_t psa_call(psa_handle_t handle,
                        int32_t type,
                        const psa_invec *in_vec,
                        size_t in_len,
                        psa_outvec *out_vec,
                        size_t out_len);

Interface ``agent_psa_call()`` has 4 arguments only, to avoid ABI complexity
when more than 4 arguments get involved. Then input and output vectors must
be squashed into a new type to achieve this squashing. There are several
scenarios to be considered:

- In the shared-memory-based mailbox scheme, the non-secure interface layer
  puts ``psa_invec`` and ``psa_outvec`` instances and their members in the
  shared memory. The pointers of these items are delivered to
  ``Mailbox NS Agent`` for the agent's referencing. All vectors are
  non-secure in this case.
- Still, in the shared-memory-based mailbox scheme, but this time not all
  parameters are prepared by non-secure clients. Some of the items are
  allocated by the agent. Secure and non-secure vectors get mixed in this case.
- In the scheme that a memory address can not be delivered (A serial
  interface e.g.), ``Mailbox NS Agent`` allocates ``psa_invec`` and
  ``psa_outvec`` in local memory to collect pointers and sizes of received
  buffers. All the vectors are secure in this case.

Based on these scenarios, and the case that an agent might access depending
services with the agent itself's identifier and secure vectors, a straight
conclusion is that the memory source information - secure or non-secure - for
input and output vectors and their pointing memories need to be indicated by
the agent so that SPM can perform a memory check towards given vectors.

Most of the SPE platforms have the capability to identify if a given memory
pointer is secure or not, which makes this indication look duplicated. But the
indication is necessary for these scenarios:

- The SPE platform identifies the memory source by the address mapped into
  SPE's address space, but this mapping happens inside the agent instead of
  SPM. It is the agent who receives the mailbox data and maps it, so it knows
  which addresses need mapping and the others do not.
- The SPE platform just does not have such capability. The addresses from the
  mailbox can be treated as non-secure always but there are cases that the
  agent itself needs to access services with its own memory instead of
  representing the non-secure clients.

To cover the above mentioned scenarios, guidelines are listed for input and
output vector processing:

- The agent needs to tell SPM where vectors and descriptors come from, to
  assist SPM performs a proper memory checking. The source information is
  encoded in the parameter ``control``.
- When SPE platforms have the capability to identify the memory sources,
  platforms can decide whether to skip the indication or not, in the HAL.

A composition type is created for packing the vectors:

.. code-block:: c

  struct client_param_t {
    int32_t         ns_client_id_stateless;
    const psa_invec *p_invecs;
    psa_outvec      *p_outvecs;
  };

``ns_client_id_stateless`` indicates the non-secure client id when the client
is accessing a stateless service. This member is ignored if the target service
is a connection-based one.

.. note::
  The vectors and non-secure client ID are recorded in the internal handle.
  Hence it is safe to claim ``client_param_t`` instance as local variable.

Agent-specific Client API
=========================
``agent_psa_connect()`` is the API added to support agent forwarding NS
requests.

.. code-block:: c

  psa_handle_t agent_psa_connect(uint32_t sid, uint32_t version,
                                 int32_t ns_client_id, const void *client_data);

One extra parameter ``ns_client_id`` added to tell SPM which NS client the
agent is representing when API gets called. It is recorded in the handle
association data in SPM and requires to be a negative value; ZERO or positive
values are invalid non-secure client IDs, SPM does not use these invalid IDs
in the message. Instead, it puts the agent's ID into the messaging in this
case. This mechanism can provide chances for the agents calling APIs for their
own service accessing and API works asynchronously.

As mentioned, the standard FFM Client service accessing API are blocked until
the IPC message gets replied to. While this API returns immediately without
waiting for acknowledgement. Unless an error occurred, these agent-specific
API returns PSA_SUCCESS always. The replies for these access requests are
always fetched initiative by the agent with a ``psa_get()``.

.. code-block:: c

  psa_status_t agent_psa_call(psa_handle_t handle, uint32_t control,
                              const struct client_param_t   *params,
                              const void   *client_data_stateless);

Compared to the standard ``psa_call()``, this API:

- Squashes the ``psa_invec``, ``psa_outvec``, and ``ns_client_id_stateless``
  into parameter ``params``.
- One extra parameter ``client_data_stateless`` for ``agent_psa_call()`` stands
  for the auxiliary data added. This member is ignored for connection-based
  services because ``agent_psa_connect()`` already assigned one in the
  connected handle.
- Has a composited agrument ``control``.

The encoding scheme for ``control``:

====== ====== ================================================================
Bit(s) Name   Description
====== ====== ================================================================
27     NSIV   `1`: Input vectors in non-secure memory. `0`: Secure memory.
24-26  IVNUM  Number of input vectors.
19     NSOV   `1`: Output vectors in non-secure memory. `0`: Secure memory.
16-18  OVNUM  Number of output vectors.
0-15   type   signed 16-bit service `type`.
====== ====== ================================================================

.. note::
  ``control`` is a 32-bit unsigned integer and bits not mentioned in the
  table are reserved for future usage.

Agent-specific signal
=====================
To cooperate with the agent-specific API, one extra acknowledgement signal is
defined:

.. code-block:: c

  #define ASYNC_MSG_REPLY            (0x00000004u)

This signal can be sent to agent type component only. An agent can call
``psa_get()`` with this signal to get one acknowledged message. This signal is
cleared when all queued messages for the agent have been retrieved using
``psa_get()``. SPM assembles the information into agent provided message object.
For the stateless handle, the internal handle object is freed after this
``psa_get()`` call. The agent can know what kind of message is acknowledged by
the ``type`` member in the ``psa_msg_t``, and the ``client_data`` passed in is
put in member ``rhandle``. If no 'ASYNC_MSG_REPLY' signals pending, calling
``psa_get()`` gets ``panic``.

Code Example
============

.. code-block:: c

  /*
   * The actual implementation can change this __customized_t freely, or
   * discard this type and apply some in-house mechanism - the example
   * here is to introduce how an agent works only.
   */
  struct __customized_t {
      int32_t      type;
      int32_t      client_id;
      psa_handle_t handle;
      psa_handle_t status;
  };

  void mailbox_main(void)
  {
      psa_signal_t   signals;
      psa_status_t   status;
      psa_msg_t      msg;
      struct client_param_t client_param;
      struct __customized_t ns_msg;

      while (1) {
          signals = psa_wait(ALL, BLOCK);

          if (signals & MAILBOX_INTERRUPT_SIGNAL) {
              /* NS memory check needs to be performed. */
              __customized_platform_get_mail(&ns_msg);

              /*
               * MACRO 'SID', 'VER', 'NSID', 'INVEC_LEN', 'OUTVEC_LEN', and
               * 'VECTORS' represent necessary information extraction from
               * 'ns_msg', put MACRO names here and leave the details to the
               * implementation.
               */
              if (ns_msg.type == PSA_IPC_CONNECT) {
                  status = agent_psa_connect(SID(ns_msg), VER(ns_msg),
                                             NSID(ns_msg), &ns_msg);
              } else if (ns_msg.type == PSA_IPC_CLOSE) {
                  psa_close(ns_msg.handle);
              } else {
                  /* Other types as call type and let API check errors. */
                  client_param.ns_client_id_stateless = NSID(ns_msg);

                  /*
                   * Use MACRO to demonstrate two cases: local vector
                   * descriptor and direct descriptor forwarding.
                   */

                  /* Point to vector pointers in ns_msg. */
                  PACK_VECTOR_POINTERS(client_param, ns_msg);
                  status = agent_psa_call(ns_msg.handle,
                                          PARAM_PACK(ns_msg.type,
                                                     INVEC_LEN(ns_msg),
                                                     OUTVEC_LEN(ns_msg))|
                                                     NSIV | NSOV,
                                          &client_param,
                                          &ns_msg);
              }
              /*
               * The service access reply is always fetched by a later
               * `psa_get` hence here only errors need to be dispatched.
               */
              error_dispatch(status);

          } else if (signals & ASYNC_MSG_REPLY) {
              /* The handle is freed for stateless service after 'psa_get'. */
              status        = psa_get(ASYNC_MSG_REPLY, &msg);
              ms_msg        = msg.rhandle;
              ns_msg.status = status;
              __customized_platform__send_mail(&ns_msg);
          }
      }
  }

.. note::
  ``__customized*`` API are implementation-specific APIs to be implemented by
  the mailbox Agent developer.

Customized manifest attribute
=============================
Two extra customized manifest attributes are added:

============= ====================================================
Name          Description
============= ====================================================
ns_agent      Indicate if manifest owner is an Agent.
------------- ----------------------------------------------------
ns_client_ids Possible non-secure Client ID values (<0).
============= ====================================================

Attribute 'ns_client_ids' can be a set of numbers, or it can use a range
expression such as [min, max]. The tooling can detect ID overlap between
multiple non-secure agents.

.. note::
  Per-non-secure-client dependencies scheme is now assumed to be implemented
  by agent customization. Feedback if there are requirements for a unified
  scheme.

***********************
Manifest tooling update
***********************
The manifest for agents involves specific keys ('ns_agent' e.g.), these keys
give hints about how to achieve out-of-FFM partitions which might be abused
easily by developers, for example, claim partitions as agents. Some
restrictions need to be applied in the manifest tool to limit the general
secure service development referencing these keys.

.. note::
  The limitations can mitigate the abuse but can't prevent it, as developers
  own all the source code they are working with.

One mechanism: adding a confirmation in the partition list file.

.. parsed-literal::

  "description": "Non-Secure Mailbox Agent",
  "manifest": "${CMAKE_SOURCE_DIR}/secure_fw/partitions/ns_agent_mailbox/ns_agent_mailbox.yaml",
  "non_ffm_attributes": "ns_agent", "other_option",

``non_ffm_attributes`` tells the manifest tool that ``ns_agent`` is valid
in ns_agent_mailbox.ymal. Otherwise, the manifest tool reports an error when a
non-agent service abuses ns_agent in its manifest.

***********************************
Runtime programming characteristics
***********************************

Mailbox agent shall not be blocked by Agent-specific APIs. It can be blocked when:

- It is calling standard PSA Client APIs.
- It is calling ``psa_wait()``.

IDLE processing
===============
Only ONE place is recommended to enter IDLE. The place is decided based on the
system topologies:

- If there is one Trustzone-based NSPE, this NSPE is the recommended place no
  matter how many mailbox agents are in the system.
- If there are only mailbox-based NSPEs, entering IDLE can happen in
  one of the mailbox agents.

The solution is:

- An IDLE entering API is provided in SPRTL.
- A partition without specific flag can't call this API.
- The manifest tooling counts the partitions with this specific flag, and
  assert errors when multiple instances are found.

--------------

*Copyright (c) 2022-2023, Arm Limited. All rights reserved.*
*Copyright (c) 2023 Cypress Semiconductor Corporation (an Infineon company)
or an affiliate of Cypress Semiconductor Corporation. All rights reserved.*
