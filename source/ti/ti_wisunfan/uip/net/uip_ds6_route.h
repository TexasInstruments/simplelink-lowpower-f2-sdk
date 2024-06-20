/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/******************************************************************************
 *
 * Copyright (c) 2014 Texas Instruments Inc.  All rights reserved.
 *
 * DESCRIPTION:
 *
 * HISTORY:
 *
 *
 ******************************************************************************/
#ifndef UIP_DS6_ROUTE_H
#define UIP_DS6_ROUTE_H

#include "sys/stimer.h"
#include "lib/list.h"

#ifndef UIP_CONF_DS6_SINGLE_DEFRT
#define UIP_DS6_SINGLE_DEFRT 0
#else
#define UIP_DS6_SINGLE_DEFRT UIP_CONF_DS6_SINGLE_DEFRT
#endif

#ifndef UIP_CONF_DS6_INFINITE_DEFRT
#define UIP_DS6_INFINITE_DEFRT 0
#else
#define UIP_DS6_INFINITE_DEFRT UIP_CONF_DS6_INFINITE_DEFRT
#endif

#ifndef UIP_CONF_DS6_NOTIFICATIONS
#define UIP_DS6_NOTIFICATIONS 0
#else
#define UIP_DS6_NOTIFICATIONS UIP_CONF_UIP_DS6_NOTIFICATIONS
#endif

#if UIP_DS6_NOTIFICATIONS
/* Event constants for the uip-ds6 route notification interface. The
 notification interface allows for a user program to be notified via
 a callback when a route has been added or removed and when the
 system has added or removed a default route. */
#define UIP_DS6_NOTIFICATION_DEFRT_ADD 0
#define UIP_DS6_NOTIFICATION_DEFRT_RM  1
#define UIP_DS6_NOTIFICATION_ROUTE_ADD 2
#define UIP_DS6_NOTIFICATION_ROUTE_RM  3

typedef void (* uip_ds6_notification_callback)(int event,
        uip_ipaddr_t *route,
        uip_ipaddr_t *nexthop,
        int num_routes);
struct uip_ds6_notification
{
    struct uip_ds6_notification *next;
    uip_ds6_notification_callback callback;
};

void uip_ds6_notification_add(struct uip_ds6_notification *n,
        uip_ds6_notification_callback c);

void uip_ds6_notification_rm(struct uip_ds6_notification *n);
/*--------------------------------------------------*/
#endif

/* Routing table */
#ifndef UIP_CONF_MAX_ROUTES
#ifdef UIP_CONF_DS6_ROUTE_NBU
#define UIP_DS6_ROUTE_NB UIP_CONF_DS6_ROUTE_NBU
#else /* UIP_CONF_DS6_ROUTE_NBU */
#define UIP_DS6_ROUTE_NB 4
#endif /* UIP_CONF_DS6_ROUTE_NBU */
#else /* UIP_CONF_MAX_ROUTES */
#define UIP_DS6_ROUTE_NB UIP_CONF_MAX_ROUTES
#endif /* UIP_CONF_MAX_ROUTES */

#if (UIP_DS6_ROUTING_MODE == UIP_DS6_SOURCE_ROUTING)

/** \brief An entry in the routing table */
typedef struct uip_ds6_sourceRouting_route
{
    struct uip_ds6_sourceRouting_route* next; //this is needed to make the list of routes that the routing table manages
    uip_ipaddr_t previousHop;
    uip_ipaddr_t ipaddr;
#ifdef UIP_DS6_ROUTE_STATE_TYPE
    UIP_DS6_ROUTE_STATE_TYPE state;
#endif
    uint16_t length;
}uip_ds6_route_sourceRouting_t;

/** \brief An entry in the routing table */
typedef struct uip_ds6_sourceRouting_path
{
    struct uip_ds6_sourceRouting_path* next; //this is needed to make the list of routes for a certain path that is required
    uip_ipaddr_t* ipaddr;
}uip_ds6_sourceRouting_path_t;

void uip_ds6_route_sourceRouting_init(void);

/** \name Routing Table basic routines */
/** @{ */

/*
 * Add a new entry to the source routing table. If the target exists, the previousHop is updated
 * ipaddr: target we wish to add
 * previousHop: address of the previous hop (or parent) for the target
 * length: number of bytes to consider in ipaddr (128 means full IPv6 address, 64 would be an 8 byte prefix)
 */
uip_ds6_route_sourceRouting_t *
uip_ds6_route_sourceRouting_add(uip_ipaddr_t *ipaddr, u16_t length,
        uip_ipaddr_t *previousHop);

//Remove a given route in the source routing table
void uip_ds6_route_sourceRouting_rm(uip_ds6_route_sourceRouting_t *route);

//Obtaine a route for the target in "addr". This would only be the route target/previousHop but not the path to it
uip_ds6_route_sourceRouting_t *
uip_ds6_sourceRouting_route_lookup(uip_ipaddr_t *addr);

//Get a path to "addr" or NULL if there is no full path to it
uip_ds6_sourceRouting_path_t *
uip_ds6_sourceRouting_path_lookup(uip_ipaddr_t *addr);

//Get a pointer to the beginning of the last requested path to a given target. NULL if no path was found
uip_ds6_sourceRouting_path_t *
uip_ds6_sourceRouting_path_head(void);

//Get a pointer to the next item in the path to a given target. NULL if there are no more items
uip_ds6_sourceRouting_path_t *
uip_ds6_sourceRouting_path_next(uip_ds6_sourceRouting_path_t *r);

//Get the number of nodes that conform the last requested path. 0 if no path was found
int
uip_ds6_sourceRouting_path_num_nodes(void);

//Get the number of route items (address/previousHop couples) in the source routing table. 0 if there are no route items
int uip_ds6_route_sourceRouting_num_routes(void);

//Get a pointer to the beginning of the list of route items in the source routing table. NULL if there are no route items
uip_ds6_route_sourceRouting_t *
uip_ds6_route_sourceRouting_head(void);

//Get a pointer to the next route item in the source routing table. NULL if there are no more items
uip_ds6_route_sourceRouting_t *
uip_ds6_route_sourceRouting_next(uip_ds6_route_sourceRouting_t *r);

//Returns 1 if the path obtained is valid, meaning that there are no repeated addresses in the path. 0 if it is not valid
int uip_ds6_route_sourceRouting_isValidPath(uip_ds6_sourceRouting_path_t *path);

#elif (UIP_DS6_ROUTING_MODE == UIP_DS6_STANDARD_ROUTING)

/** \brief The neighbor routes hold a list of routing table entries
 that are attached to a specific neighbor. */
struct uip_ds6_route_neighbor_routes
{
    LIST_STRUCT(route_list);
};

/** \brief An entry in the routing table */
typedef struct uip_ds6_route
{
    struct uip_ds6_route *next;
    /* Each route entry belongs to a specific neighbor. That neighbor
     holds a list of all routing entries that go through it. The
     routes field point to the uip_ds6_route_neighbor_routes that
     belong to the neighbor table entry that this routing table entry
     uses. */
    struct uip_ds6_route_neighbor_routes *neighbor_routes;
    uip_ipaddr_t ipaddr;
#ifdef UIP_DS6_ROUTE_STATE_TYPE
    UIP_DS6_ROUTE_STATE_TYPE state;
#endif
    uint16_t length;
} uip_ds6_route_t;

/** \brief A neighbor route list entry, used on the
 uip_ds6_route->neighbor_routes->route_list list. */
struct uip_ds6_route_neighbor_route
{
    struct uip_ds6_route_neighbor_route *next;
    struct uip_ds6_route *route;
};

void uip_ds6_route_init(void);

/** \name Routing Table basic routines */
/** @{ */
uip_ds6_route_t *uip_ds6_route_lookup(uip_ipaddr_t *destipaddr);
uip_ds6_route_t *uip_ds6_route_lookup_by_nexthop(uip_ipaddr_t *nexthop);
uip_ds6_route_t *uip_ds6_route_add(uip_ipaddr_t *ipaddr, uint16_t length,
                                   uip_ipaddr_t *next_hop);
void uip_ds6_route_rm(uip_ds6_route_t *route);
void uip_ds6_route_rm_by_nexthop(uip_ipaddr_t *nexthop);

uip_ipaddr_t *uip_ds6_route_nexthop(uip_ds6_route_t *);
int uip_ds6_route_num_routes(void);
uip_ds6_route_t *uip_ds6_route_head(void);
uip_ds6_route_t *uip_ds6_route_next(uip_ds6_route_t *);

#elif (UIP_DS6_ROUTING_MODE == UIP_DS6_NO_ROUTING) //Only a default router

/** \brief An entry in the routing table */
typedef struct uip_ds6_route
{
    struct uip_ds6_route *next;
    uip_ipaddr_t ipaddr;
#ifdef UIP_DS6_ROUTE_STATE_TYPE
    UIP_DS6_ROUTE_STATE_TYPE state;
#endif
    uint16_t length;
}uip_ds6_route_t;

#define uip_ds6_route_init()

/** \name Routing Table basic routines */
/** @{ */
#define uip_ds6_route_lookup(destipaddr)                NULL
#define uip_ds6_route_lookup_by_nexthop(nexthop)        NULL
#define uip_ds6_route_add(ipaddr,length,next_hop)       NULL
#define uip_ds6_route_rm(route)
#define uip_ds6_route_rm_by_nexthop(nexthop)
#define uip_ds6_route_nexthop(route)                    NULL
#define uip_ds6_route_num_routes()                      0
#define uip_ds6_route_head()                            NULL
#define uip_ds6_route_next(route)                       NULL

#endif

/** \brief An entry in the default router list */
typedef struct uip_ds6_defrt
{
#if !UIP_DS6_SINGLE_DEFRT
    struct uip_ds6_defrt *next;
#endif
    uip_ipaddr_t ipaddr;
#if !UIP_DS6_INFINITE_DEFRT
    struct stimer lifetime;
#endif
    uint16_t isinfinite;
} uip_ds6_defrt_t;
/** @} */

#if UIP_DS6_SINGLE_DEFRT

extern uip_ds6_defrt_t* uip_ds6_single_defrt_p;
#define uip_ds6_defrt_choose()    ((uip_ds6_single_defrt_p != NULL)? &(uip_ds6_single_defrt_p->ipaddr) : NULL) 

#else // !UIP_DS6_SINGLE_DEFRT

uip_ipaddr_t *uip_ds6_defrt_choose(void);

#endif

/** \name Default router list basic routines */
/** @{ */
uip_ds6_defrt_t *uip_ds6_defrt_add(uip_ipaddr_t *ipaddr,
                                   unsigned long interval);
void uip_ds6_defrt_rm(uip_ds6_defrt_t *defrt);
uip_ds6_defrt_t *uip_ds6_defrt_lookup(uip_ipaddr_t *ipaddr);
void uip_ds6_defrt_periodic(void);

/** @} */

#endif /* UIP_DS6_ROUTE_H */
