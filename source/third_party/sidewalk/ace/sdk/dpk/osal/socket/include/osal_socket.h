/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
/*
 * @file    osal_socket.h
 *
 * @brief   ACE Socket APIs
 *
 * @warning These APIs are deprecated. Please use the standard POSIX/BSD socket
 *          APIs instead.
 * @deprecated
 *
 * @addtogroup ACE_OSAL_SOCKET
 */
#ifndef ACEOSAL_SOCKET_H
#define ACEOSAL_SOCKET_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <ace/osal_common.h>

/* ACE OSAL platform specific */
#if defined(__linux__) || defined(__APPLE__)
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
/* AD: else means FREERTOS */
/* AD: TODO We might have to define an aceosal define for this */
#else
#include <lwip/sockets.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**************************** refactoring note ****************************/

/*
 * All flags/definitions based on LwIP 1.5.0 -  4/8/2018.
 * sockets.h includes "cc.h , ip_addr.h , err.h , inet.h"
 * netdb.h is merged here; contains dns functions
 */

/**************************** from cc.h partial ***************************/

/**
 * @name Platform specific settings
 * @note It's very important to set this one right per platform
 * @{
 */
#define aceSocket_LITTLE_ENDIAN 1234 /**< Little endian mode */
#define aceSocket_BIG_ENDIAN 4321    /**< Big endian mode    */

/**
 * The byte order of the system.
 * Needed for conversion of network data to host byte order.
 * Allowed values: aceSocket_LITTLE_ENDIAN and aceSocket_BIG_ENDIAN
 */
#define aceSocket_BYTE_ORDER aceSocket_LITTLE_ENDIAN

typedef unsigned char u8_t;   /**< Unsigned integer type with width of 8  */
typedef signed char s8_t;     /**< Signed integer type with width of 8    */
typedef unsigned short u16_t; /**< Unsigned integer type with width of 16 */
typedef signed short s16_t;   /**< Signed integer type with width of 16   */
typedef uint32_t u32_t;       /**< Unsigned integer type with width of 32 */
typedef int32_t s32_t;        /**< Signed integer type with width of 32   */

#define aceSocket_htons(x) htons(x) /**< Host to Network Short   */
#define aceSocket_ntohs(x) ntohs(x) /**< Network to Host Short   */
#define aceSocket_htonl(x) htonl(x) /**< Host to Network Long    */
#define aceSocket_ntohl(x) ntohl(x) /**< Network to Host Long    */

#if aceSocket_BYTE_ORDER == aceSocket_BIG_ENDIAN
#define aceSocket_PP_HTONS(x) (x) /**< htons calculated by the preprocessor */
#define aceSocket_PP_NTOHS(x) (x) /**< ntohs calculated by the preprocessor */
#define aceSocket_PP_HTONL(x) (x) /**< htonl calculated by the preprocessor */
#define aceSocket_PP_NTOHL(x) (x) /**< ntohl calculated by the preprocessor */
#else                             /* BYTE_ORDER != BIG_ENDIAN */

/*
 * These macros should be calculated by the preprocessor and are used
 * with compile-time constants only (so that there is no little-endian
 * overhead at runtime).
 */
/** htons calculated by the preprocessor */
#define aceSocket_PP_HTONS(x) ((((x)&0xff) << 8) | (((x)&0xff00) >> 8))
/** ntohs calculated by the preprocessor */
#define aceSocket_PP_NTOHS(x) PP_HTONS(x)
/** htonl calculated by the preprocessor */
#define aceSocket_PP_HTONL(x)                                             \
    ((((x)&0xff) << 24) | (((x)&0xff00) << 8) | (((x)&0xff0000UL) >> 8) | \
     (((x)&0xff000000UL) >> 24))
/** ntohl calculated by the preprocessor */
#define aceSocket_PP_NTOHL(x) PP_HTONL(x)
#endif /* BYTE_ORDER == BIG_ENDIAN */

/* @} */

/**************************** err.h ****************************/

/**
 * @name ACE OSAL NETWORK error codes
 * @{
 */
typedef s8_t aceSocket_err_t; /**< Error codes type */

#define aceSocket_ERR_OK 0          /**< No error, everything OK. */
#define aceSocket_ERR_MEM -1        /**< Out of memory error.     */
#define aceSocket_ERR_BUF -2        /**< Buffer error.            */
#define aceSocket_ERR_TIMEOUT -3    /**< Timeout.                 */
#define aceSocket_ERR_RTE -4        /**< Routing problem.         */
#define aceSocket_ERR_INPROGRESS -5 /**< Operation in progress    */
#define aceSocket_ERR_VAL -6        /**< Illegal value.           */
#define aceSocket_ERR_WOULDBLOCK -7 /**< Operation would block.   */
#define aceSocket_ERR_USE -8        /**< Address in use.          */
#define aceSocket_ERR_ALREADY -9    /**< Already connecting.      */
#define aceSocket_ERR_ISCONN -10    /**< Conn already established.*/
#define aceSocket_ERR_CONN -11      /**< Not connected.           */
/** Judge whether e belongs to FATAL error.*/
#define aceSocket_ERR_IS_FATAL(e) ((e) < ERR_CONN)
#define aceSocket_ERR_ABRT -12 /**< Connection aborted.      */
#define aceSocket_ERR_RST -13  /**< Connection reset.        */
#define aceSocket_ERR_CLSD -14 /**< Connection closed.       */
#define aceSocket_ERR_ARG -15  /**< Illegal argument.        */
#define aceSocket_ERR_IF -16   /**< Low-level netif error.   */
/* @} */

/**************************** inet.h ****************************/

typedef u32_t aceSocket_in_addr_t;

struct aceSocket_in_addr {
    aceSocket_in_addr_t s_addr;
};

struct aceSocket_in6_addr {
    union {
        u32_t u32_addr[4];
        u8_t u8_addr[16];
    } un;
#define aceSocket_s6_addr un.u8_addr
};

/**
 * @name Definitions about IP address
 * @{
 */
/** 255.255.255.255 */
#define aceSocket_INADDR_NONE INADDR_NONE
/** 127.0.0.1 */
#define aceSocket_INADDR_LOOPBACK INADDR_LOOPBACK
/** 0.0.0.0 */
#define aceSocket_INADDR_ANY INADDR_ANY
/** 255.255.255.255 */
#define aceSocket_INADDR_BROADCAST INADDR_BROADCAST
/** This macro can be used to initialize a variable of type struct in6_addr
    to the IPv6 wildcard address. */
#define aceSocket_IN6ADDR_ANY_INIT IN6ADDR_ANY_INIT
/** This macro can be used to initialize a variable of type struct in6_addr
    to the IPv6 loopback address. */
#define aceSocket_IN6ADDR_LOOPBACK_INIT IN6ADDR_LOOPBACK_INIT
/* This variable is initialized by the system to contain the wildcard IPv6
 * address. */
/* extern const struct in6_addr in6addr_any; */
/* @} */

/**
 * @name Definitions of the bits in an (IPv4) Internet address integer.
 * @note On subnets, host and network parts are found according to
 *       the subnet mask, not these masks.
 * @{
 */
#define aceSocket_IN_CLASSA(a) IN_CLASSA(a)
#define aceSocket_IN_CLASSA_NET IN_CLASSA_NET
#define aceSocket_IN_CLASSA_NSHIFT IN_CLASSA_NSHIFT
#define aceSocket_IN_CLASSA_HOST IN_CLASSA_HOST
#define aceSocket_IN_CLASSA_MAX IN_CLASSA_MAX

#define aceSocket_IN_CLASSB(b) IN_CLASSB(b)
#define aceSocket_IN_CLASSB_NET IN_CLASSB_NET
#define aceSocket_IN_CLASSB_NSHIFT IN_CLASSB_NSHIFT
#define aceSocket_IN_CLASSB_HOST IN_CLASSB_HOST
#define aceSocket_IN_CLASSB_MAX IN_CLASSB_MAX

#define aceSocket_IN_CLASSC(c) IN_CLASSC(c)
#define aceSocket_IN_CLASSC_NET IN_CLASSC_NET
#define aceSocket_IN_CLASSC_NSHIFT IN_CLASSC_NSHIFT
#define aceSocket_IN_CLASSC_HOST IN_CLASSC_HOST
#define aceSocket_IN_CLASSC_MAX IN_CLASSC_MAX

#define aceSocket_IN_CLASSD(d) IN_CLASSD(d)
#define aceSocket_IN_CLASSD_NET IN_CLASSD_NET
#define aceSocket_IN_CLASSD_NSHIFT IN_CLASSD_NSHIFT
#define aceSocket_IN_CLASSD_HOST IN_CLASSD_HOST
#define aceSocket_IN_CLASSD_MAX IN_CLASSD_MAX
#define aceSocket_IN_MULTICAST(a) IN_MULTICAST(a)
#define aceSocket_IN_EXPERIMENTAL(a) IN_EXPERIMENTAL(a)
#define aceSocket_IN_BADCLASS(a) IN_BADCLASS(a)
#define aceSocket_IN_LOOPBACKNET IN_LOOPBACKNET

#define aceSocket_INET_ADDRSTRLEN INET_ADDRSTRLEN
#define aceSocket_INET6_ADDRSTRLEN INET6_ADDRSTRLEN

#define aceSocket_inet_addr(cp) inet_addr(cp)
#define aceSocket_inet_aton(cp, addr) inet_aton(cp, (struct in_addr*)addr)
#define aceSocket_inet_ntoa(addr) inet_ntoa(*(struct in_addr*)&addr)
/* works for IP4 and IP6 */
#define aceSocket_inet_ntop(af, src, dst, size) inet_ntop(af, src, dst, size)
#define aceSocket_inet_pton(af, src, dst) inet_pton(af, src, dst)
/* @} */

/**************************** sockets.h ****************************/

typedef u8_t aceSocket_sa_family_t;
typedef u16_t aceSocket_in_port_t;
/* members are in network byte order */
struct aceSocket_sockaddr_in {
    u8_t sin_len;
    aceSocket_sa_family_t sin_family;
    aceSocket_in_port_t sin_port;
    struct aceSocket_in_addr sin_addr;
#define SIN_ZERO_LEN 8
    char sin_zero[SIN_ZERO_LEN];
};

struct aceSocket_sockaddr_in6 {
    u8_t sin6_len;                       /**< length of this structure    */
    aceSocket_sa_family_t sin6_family;   /**< AF_INET6                    */
    aceSocket_in_port_t sin6_port;       /**< Transport layer port #      */
    u32_t sin6_flowinfo;                 /**< IPv6 flow information       */
    struct aceSocket_in6_addr sin6_addr; /**< IPv6 address                */
    u32_t sin6_scope_id;                 /**< Set of interfaces for scope */
};

struct aceSocket_sockaddr {
    u8_t sa_len;
    aceSocket_sa_family_t sa_family;
    char sa_data[14];
};

struct aceSocket_sockaddr_storage {
    u8_t s2_len;
    aceSocket_sa_family_t ss_family;
    char s2_data1[2];
    u32_t s2_data2[3];
    u32_t s2_data3[3];
};

typedef u32_t aceSocket_socklen_t;

typedef struct aceSocket_connect_ctrl {
    uint32_t timeout; /**< Connect timeout */
} aceSocket_connect_ctrl_t;

#define CONNECT_PARA_DEFINE(ctrl) aceSocket_connect_ctrl_t ctrl = {0};
static inline void aceSocket_setConnectTimeout(aceSocket_connect_ctrl_t* ctrl,
                                               uint32_t timeout) {
    (void)(ctrl && (ctrl->timeout = timeout));
}

/**
 * @name Socket protocol types
 * @{
 */
#define aceSocket_SOCK_STREAM SOCK_STREAM /**< TCP */
#define aceSocket_SOCK_DGRAM SOCK_DGRAM   /**< UDP */
#define aceSocket_SOCK_RAW SOCK_RAW       /**< RAW */
                                          /* @} */

/**
 * @name Option flags per-socket
 * @note These must match the SOF_ flags in ip.h (checked in init.c)
 * @{
 */
// clang-format off
#define aceSocket_SO_REUSEADDR SO_REUSEADDR /**< Allow local address reuse */
#define aceSocket_SO_KEEPALIVE SO_KEEPALIVE /**< Keep connections alive    */
#define aceSocket_SO_BROADCAST SO_BROADCAST /**< Permit to send and to receive broadcast messages */
// clang-format on
/* @} */

/**
 * @name Additional options
 * @note Not kept in so_options
 * @{
 */
// clang-format off
#define aceSocket_SO_DEBUG SO_DEBUG                 /**< Unimplemented: turn on debugging info recording */
#define aceSocket_SO_ACCEPTCONN SO_ACCEPTCONN       /**< socket has had listen()                         */
#define aceSocket_SO_DONTROUTE SO_DONTROUTE         /**< Unimplemented: just use interface addresses     */
#define aceSocket_SO_USELOOPBACK SO_USELOOPBACK     /**< Unimplemented: bypass hardware when possible    */
#define aceSocket_SO_LINGER SO_LINGER               /**< linger on close if data present                 */
#define aceSocket_SO_DONTLINGER ((int)(~SO_LINGER)) /**< Don't linger on close if data present           */
#define aceSocket_SO_OOBINLINE SO_OOBINLINE         /**< Unimplemented: leave received OOB data in line  */
#define aceSocket_SO_REUSEPORT SO_REUSEPORT         /**< Unimplemented: allow local address & port reuse */
#define aceSocket_SO_SNDBUF SO_SNDBUF               /**< Unimplemented: send buffer size                 */
#define aceSocket_SO_RCVBUF SO_RCVBUF               /**< receive buffer size                             */
#define aceSocket_SO_SNDLOWAT SO_SNDLOWAT           /**< Unimplemented: send low-water mark              */
#define aceSocket_SO_RCVLOWAT SO_RCVLOWAT           /**< Unimplemented: receive low-water mark           */
#define aceSocket_SO_SNDTIMEO SO_SNDTIMEO           /**< send timeout                                    */
#define aceSocket_SO_RCVTIMEO SO_RCVTIMEO           /**< receive timeout                                 */
#define aceSocket_SO_ERROR SO_ERROR                 /**< get error status and clear                      */
#define aceSocket_SO_TYPE SO_TYPE                   /**< get socket type                                 */
#define aceSocket_SO_CONTIMEO SO_CONTIMEO           /**< Unimplemented: connect timeout                  */
#define aceSocket_SO_NO_CHECK SO_NO_CHECK           /**< don't create UDP checksum                       */
// clang-format on
/* @} */

/**
 * Structure used for manipulating linger option.
 */
struct aceSocket_linger {
    int l_onoff;  /**< option on/off          */
    int l_linger; /**< linger time in seconds */
};

/**
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define aceSocket_SOL_SOCKET SOL_SOCKET

#define aceSocket_AF_UNSPEC AF_UNSPEC
#define aceSocket_AF_INET AF_INET
#define aceSocket_AF_INET6 AF_INET6

/* AD map OK */
#define aceSocket_PF_INET aceSocket_AF_INET
#define aceSocket_PF_INET6 aceSocket_AF_INET6
#define aceSocket_PF_UNSPEC aceSocket_AF_UNSPEC

#define aceSocket_IPPROTO_IP IPPROTO_IP
#define aceSocket_IPPROTO_ICMP IPPROTO_ICMP
#define aceSocket_IPPROTO_TCP IPPROTO_TCP
#define aceSocket_IPPROTO_UDP IPPROTO_UDP
#define aceSocket_IPPROTO_IPV6 IPPROTO_IPV6
#define aceSocket_IPPROTO_ICMPV6 IPPROTO_ICMPV6
#define aceSocket_IPPROTO_UDPLITE IPPROTO_UDPLITE
#define aceSocket_IPPROTO_RAW IPPROTO_RAW

/**
 * @name Flags used with send and recv
 * @{
 */
// clang-format off
#define aceSocket_MSG_PEEK MSG_PEEK         /**< Peeks at an incoming message */
#define aceSocket_MSG_WAITALL MSG_WAITALL   /**< Unimplemented: Requests that the function block until the full amount of data requested can be returned */
#define aceSocket_MSG_OOB MSG_OOB           /**< Unimplemented: Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific */
#define aceSocket_MSG_DONTWAIT MSG_DONTWAIT /**< Nonblocking i/o for this operation only */
#define aceSocket_MSG_MORE MSG_MORE         /**< Sender will send more */
#define aceSocket_MSG_TRUNC MSG_TRUNC       /**< Message truncate */
// clang-format on
/* @} */

/**
 * @name Options for level IPPROTO_IP
 * @{
 */
#define aceSocket_IP_TOS IP_TOS
#define aceSocket_IP_TTL IP_TTL
/* @} */

/**
 * @name Options for level IPPROTO_TCP
 * @{
 */
// clang-format off
#define aceSocket_TCP_NODELAY TCP_NODELAY     /**< Don't delay send to coalesce packets */
#define aceSocket_TCP_KEEPALIVE TCP_KEEPALIVE /**< Send KEEPALIVE probes when idle for pcb->keep_idle milliseconds */
#define aceSocket_TCP_KEEPIDLE TCP_KEEPIDLE   /**< Set pcb->keep_idle  - Same as TCP_KEEPALIVE, but use seconds for get/setsockopt */
#define aceSocket_TCP_KEEPINTVL TCP_KEEPINTVL /**< Set pcb->keep_intvl - Use seconds for get/setsockopt */
#define aceSocket_TCP_KEEPCNT TCP_KEEPCNT     /**< Set pcb->keep_cnt   - Use number of probes sent for get/setsockopt */
// clang-format on
/* @} */

/**
 * @name Options for level IPPROTO_IPV6
 * @{
 */
#define aceSocket_IPV6_CHECKSUM                                              \
    IPV6_CHECKSUM /**< RFC3542: calculate and insert the ICMPv6 checksum for \
                     raw sockets. */
#define aceSocket_IPV6_V6ONLY                                                 \
    IPV6_V6ONLY /**< RFC3493: boolean control to restrict AF_INET6 sockets to \
                   IPv6 communications only. */
/* @} */
#define aceSocket_IPV6_MULTICAST_TTL IPV6_MULTICAST_TTL
#define aceSocket_IPV6_MULTICAST_IF IPV6_MULTICAST_IF
#define aceSocket_IPV6_MULTICAST_LOOP IPV6_MULTICAST_LOOP
#define aceSocket_IPV6_ADD_MEMBERSHIP IPV6_ADD_MEMBERSHIP
#define aceSocket_IPV6_DROP_MEMBERSHIP IPV6_DROP_MEMBERSHIP

typedef struct aceSocket_ipv6_mreq {
    struct aceSocket_in6_addr ipv6mr_multiaddr; /**< IPv6 multicast address */
    struct aceSocket_in6_addr ipv6mr_interface; /**< Interface index */
} aceSocket_ipv6_mreq;

/**
 * @name Options for level IPPROTO_UDPLITE
 * @{
 */
// clang-format off
#define aceSocket_UDPLITE_SEND_CSCOV UDPLITE_SEND_CSCOV /**< Sender checksum coverage */
#define aceSocket_UDPLITE_RECV_CSCOV UDPLITE_RECV_CSCOV /**< Minimal receiver checksum coverage */
// clang-format on
/* @} */

/**
 * @name Options and types for UDP multicast traffic handling
 * @{
 */
#define aceSocket_IP_MULTICAST_TTL IP_MULTICAST_TTL
#define aceSocket_IP_MULTICAST_IF IP_MULTICAST_IF
#define aceSocket_IP_MULTICAST_LOOP IP_MULTICAST_LOOP
/* @} */

/**
 * @name Options and types related to multicast membership
 * @{
 */
#define aceSocket_IP_ADD_MEMBERSHIP IP_ADD_MEMBERSHIP
#define aceSocket_IP_DROP_MEMBERSHIP IP_DROP_MEMBERSHIP
/* @} */

typedef struct aceSocket_ip_mreq {
    /**< IP multicast address of group */
    struct aceSocket_in_addr imr_multiaddr;
    /**< Local IP address of interface */
    struct aceSocket_in_addr imr_interface;
} aceSocket_ip_mreq;

#define aceSocket_FIONREAD FIONREAD
#define aceSocket_FIONBIO FIONBIO

/**
 * @name Commands for fcntl
 * @{
 */
#define aceSocket_F_GETFL F_GETFL
#define aceSocket_F_SETFL F_SETFL
/* @} */

/**
 * @name File status flags and file access modes for fcntl
 * @brief These are bits in an int.
 * @{
 */
// clang-format off
#define aceSocket_O_NONBLOCK O_NONBLOCK /**< Nonblocking I/O                       */
#define aceSocket_O_NDELAY O_NDELAY     /**< Same as O_NONBLOCK, for compatibility */
// clang-format on

/** Disables further receive operations. */
#define aceSocket_SHUT_RD SHUT_RD
/** Disables further send operations. */
#define aceSocket_SHUT_WR SHUT_WR
/** Disables further send and receive operations. */
#define aceSocket_SHUT_RDWR SHUT_RDWR
/* @} */

#define aceSocket_FD_SET(fd, fdsetp) FD_SET(fd, fdsetp);
#define aceSocket_FD_CLR(fd, fdsetp) FD_CLR(fd, fdsetp);
#define aceSocket_FD_ISSET(fd, fdsetp) FD_ISSET(fd, fdsetp);
#define aceSocket_FD_ZERO(fdsetp) FD_ZERO(fdsetp);
typedef fd_set aceSocket_fd_set;

/**************************** ACE Socket APIs ****************************/

/**
 * @brief Accepts a connection on a socket
 * @param[in] s Specifies the index of the socket.
 * @param[out] addr Pointer to aceSocket_sockaddr structure where the address of
 *             the connecting socket will be returned.
 * @param[in, out] addrlen Points to a socklen_t which on input specifies the
 *                 length of the supplied sockaddr structure, and on output
 *                 specifies the length of the stored address.
 * @return The index of new socket (if succeed)\n
 *         -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_accept(int s, struct aceSocket_sockaddr* addr,
                     aceSocket_socklen_t* addrlen);

/**
 * @brief Associates a local address with a socket.
 * @param[in] s Specifies the index of the socket.
 * @param[in] name Pointer to aceSocket_sockaddr structure that identify
 *            connection.
 * @param[in] namelen The length of the supplied aceSocket_sockaddr structure.
 * @retval  0 (if succeed)
 * @retval -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_bind(int s, struct aceSocket_sockaddr* name,
                   aceSocket_socklen_t namelen);

/**
 * @brief Shut down socket send and receive operations
 * @param[in] s Specifies the index of the socket.
 * @param[in] how Specifies the type of shutdown. The values are
 *            aceSocket_SHUT_RD, aceSocket_SHUT_WR and aceSocket_SHUT_RDWR.
 * @retval  0 (if succeed)
 * @retval -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_shutdown(int s, int how);

/**
 * @brief Get the name of the peer socket
 * @param[in] s Specifies the index of the socket.
 * @param[out] name Stores the peer address in the aceSocket_sockaddr structure
 *             pointed to by this argument.
 * @param[out] namelen Stores the length of the peer address in the object
 *             pointed to by this argument.
 * @retval  0 (if succeed)
 * @retval -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_getpeername(int s, struct aceSocket_sockaddr* name,
                          aceSocket_socklen_t* namelen);

/**
 * @brief Get the socket name
 * @param[in] s Specifies the index of the socket.
 * @param[out] name Stores the name address in the aceSocket_sockaddr structure
 *             pointed to by this argument.
 * @param[out] namelen Stores the length of the name address in the object
 *             pointed to by this argument.
 * @retval  0 (if succeed)
 * @retval -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_getsockname(int s, struct aceSocket_sockaddr* name,
                          aceSocket_socklen_t* namelen);

/**
 * @brief Get the socket options
 * @param[in] s Specifies the index of the socket.
 * @param[in] level Specifies the protocol level at which the option resides.
 * @param[in] optname Specifies a single option to be retrieved.
 * @param[out] optval Stores the option value in the object pointed to by this
 *             argument.
 * @param[in, out] optlen When used as an entry argument, it's the maximum
 *                 length of the option value.
 *                 When used as an exit value, it's the actual length of
 *                 the option value.
 * @retval  0 (if succeed)
 * @retval -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_getsockopt(int s, int level, int optname, void* optval,
                         aceSocket_socklen_t* optlen);

/**
 * @brief Set the socket options
 * @param[in] s Specifies the index of the socket.
 * @param[in] level Specifies the protocol level at which the option resides.
 * @param[in] optname Specifies a single option to be retrieved.
 * @param[in] optval Stores the new option value to be set.
 * @param[in] optlen The size of the option value.
 * @retval  0 (if succeed)
 * @retval -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_setsockopt(int s, int level, int optname, const void* optval,
                         aceSocket_socklen_t optlen);

/**
 * @brief Closes a socket
 * @param[in] s Specifies the index of the socket.
 * @retval  0 (if succeed)
 * @retval -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_close(int s);

/**
 * @brief Connect a socket
 * @param[in] s Specifies the index of the socket.
 * @param[in] name Points to a aceSocket_sockaddr structure cntaining the peer
 *            address.
 * @param[in] namelen Specifies the length of the aceSocket_sockaddr structure.
 * @retval  0 (if succeed)
 * @retval -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_connect(int s, struct aceSocket_sockaddr* name,
                      aceSocket_socklen_t namelen);

/**
 * @brief Connect a socket with time out
 * @param[in] s Specifies the index of the socket.
 * @param[in] name Points to a aceSocket_sockaddr structure cntaining the peer
 *            address.
 * @param[in] namelen Specifies the length of the aceSocket_sockaddr structure.
 * @param[in] ctrl Specifies the time out of the socket connection.
 * @retval  0 (if succeed)
 * @retval -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_connect2(int s, struct aceSocket_sockaddr* name,
                       aceSocket_socklen_t namelen,
                       aceSocket_connect_ctrl_t ctrl);

/**
 * @brief Sets a socket into listen mode
 * @param[in] s Specifies the index of the socket.
 * @param[in] backlog The number of outstanding connections in the socket's
 *            listen queue.
 * @retval  0 (if succeed)
 * @retval -1 (if failure)
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_listen(int s, int backlog);

/**
 * @brief Receive a message from a connected socket
 * @param[in] s Specifies the index of the socket.
 * @param[out] mem Memory buffer to store received data.
 * @param[in] len Length to data to receiving.
 * @param[in] flags Specifies the type of message reception.
 *            Values of this argument are formed by logically OR'ing zero
 *            or more of the following values:
 *            -aceSocket_MSG_PEEK - Peeks at an incoming message.
 *            -aceSocket_MSG_DONTWAIT - Nonblocking i/o for this operation.
 *            -aceSocket_MSG_MORE - Wait for more than one message.
 * @return Number of receive bytes.
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_recv(int s, void* mem, size_t len, int flags);

/**
 * @brief Reads bytes received
 * @param[in] s Specifies the index of the socket.
 * @param[out] mem Memory buffer to store received data.
 * @param[in] len Length to data to receiving.
 * @return Number of read bytes.
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_read(int s, void* mem, size_t len);

/**
 * @brief Receive a message from a socket
 * @param[in] s Specifies the index of the socket.
 * @param[out] mem Memory buffer to store received data.
 * @param[in] len Length to data to receiving.
 * @param[in] flags Specifies the type of message reception.
 *            Values of this argument are formed by logically OR'ing zero
 *            or more of the following values:
 *            -aceSocket_MSG_PEEK - Peeks at an incoming message.
 *            -aceSocket_MSG_DONTWAIT - Nonblocking i/o for this operation.
 *            -aceSocket_MSG_MORE - Wait for more than one message.
 * @param[in] from A null pointer, or points to a aceSocket_sockaddr structure
 *            in which the sending address is to be stored
 * @param[in] fromlen Specifies the length of the aceSocket_sockaddr structure
 *            pointed to by the address argument.
 * @return Number of receive bytes.
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_recvfrom(int s, void* mem, size_t len, int flags,
                       struct aceSocket_sockaddr* from,
                       aceSocket_socklen_t* fromlen);

/**
 * @brief Send a message on a socket
 * @param[in] s Specifies the index of the socket.
 * @param[in] dataptr Points to the buffer containing the message to send.
 * @param[in] size Length to data to sending.
 * @param[in] flags Specifies the type of message reception.
 *            Values of this argument are formed by logically OR'ing zero
 *            or more of the following values:
 *            -aceSocket_MSG_PEEK - Peeks at an incoming message.
 *            -aceSocket_MSG_DONTWAIT - Nonblocking i/o for this operation.
 *            -aceSocket_MSG_MORE - Wait for more than one message..
 * @return Number of bytes sent or -1 (Failure).
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_send(int s, const void* dataptr, size_t size, int flags);

/**
 * @brief Send a message on a socket
 * @param[in] s Specifies the index of the socket.
 * @param[in] dataptr Points to the buffer containing the message to send.
 * @param[in] size Length to data to sending.
 * @param[in] flags Specifies the type of message reception.
 *            Values of this argument are formed by logically OR'ing zero
 *            or more of the following values:
 *            -aceSocket_MSG_PEEK - Peeks at an incoming message.
 *            -aceSocket_MSG_DONTWAIT - Nonblocking i/o for this operation.
 *            -aceSocket_MSG_MORE - Wait for more than one message.
 * @param[in] to Points to a aceSocket_sockaddr structure containing the
 *            destination address.
 * @param[in] tolen Specifies the length of the aceSocket_sockaddr structure.
 * @return Number of bytes sent or -1 (Failure).
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_sendto(int s, const void* dataptr, size_t size, int flags,
                     struct aceSocket_sockaddr* to, aceSocket_socklen_t tolen);

/**
 * @brief Create an endpoint for communication
 * @param[in] domain Specifies the protocol family of the created socket.
 * @param[in] type Specifies the type of socket to be created.
 * @param[in] protocol Specifies a particular protocol to be used with the
 *            socket.
 * @return The index of socket or -1 (Failure).
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_socket(int domain, int type, int protocol);

/**
 * @brief Send a message on a socket
 * @param[in] s Specifies the index of the socket.
 * @param[in] dataptr Points to the buffer containing the message to write.
 * @param[in] size Length to data to sending.
 * @return Number of bytes sent or -1 (Failure).
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_write(int s, const void* dataptr, size_t size);

/**
 * @brief Indicates which of the specified sockets is ready for reading,
 *        ready for writing, or has an error condition pending
 * @param[in] maxfdp1 Specifies the range of socket indices to be tested.
 * @param[in] readset  Specifies the socket indices to be checked for being
 *            ready to read.
 * @param[in] writeset Specifies the socket indices to be checked for being
 *            ready to write.
 * @param[in] exceptset Specifies the socket indices to be checked for error
 *            conditions pending.
 * @param[in] timeout Specifies a maximum interval to wait for the selection to
 *            complete.
 * @note If it is a null pointer, this function blocks until an event occur.
 * @return The total number of bits set in the bit masks or -1 (Failure).
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_select(int maxfdp1, aceSocket_fd_set* readset,
                     aceSocket_fd_set* writeset, aceSocket_fd_set* exceptset,
                     struct timeval* timeout);

/**
 * @brief Control a socket I/O
 * @param[in] s Specifies the index of the socket.
 * @param[in] cmd Selects the control function to be performed.
 * @param[in] argp Represents additional information that is needed by control
 *            function.
 * @return A value other than -1 that depends upon the control function or -1
 *         (Failure).
 *
 * @ingroup ACE_OSAL_SOCKET
 * @deprecated
 */
int aceSocket_ioctl(int s, long cmd, void* argp);

/**
 * @brief Control a socket index
 * @param[in] s Specifies the index of the socket.
 * @param[in] cmd Selects the control function (Currently only the commands
 *            F_GETFL and F_SETFL are implemented on RTOS).
 * @param[in] val Represents additional information that is needed by control
 *            function.
 * @return A value other than -1 that depends upon the control function or -1
 *         (Failure).
 *
 * @ingroup ACE_OSAL_SOCKET
 */
int aceSocket_fcntl(int s, int cmd, int val);

/**************************** ACE Netdb APIs ****************************/

/* TBD if we'd like to support DNS functions

struct aceSocket_hostent *aceSocket_gethostbyname(const char *name);
int aceSocket_gethostbyname_r(const char *name, struct aceSocket_hostent *ret,
char *buf, size_t buflen, struct aceSocket_hostent **result, int *h_errnop);
void aceSocket_freeaddrinfo(struct aceSocket_addrinfo *ai);
int aceSocket_getaddrinfo(const char *nodename,
                          const char *servname,
                          const struct aceSocket_addrinfo *hints,
                          struct aceSocket_addrinfo **res);
*/

#ifdef __cplusplus
}
#endif
#endif /* ACEOSAL_SOCKET_H */
