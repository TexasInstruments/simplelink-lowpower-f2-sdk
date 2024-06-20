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
#include "net/uip-ds6.h"
#include "net/uip.h"

#include "lib/list.h"
#include "lib/memb.h"
#include "net/nbr-table.h"

#include "uip_rpl_process.h"

#if UIP_CONF_IPV6

#include <string.h>

#undef DEBUG
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#if UIP_DS6_NOTIFICATIONS
LIST(notificationlist);

static void
call_route_callback(int event, uip_ipaddr_t *route,
        uip_ipaddr_t *nexthop)
{
    int num;
    struct uip_ds6_notification *n;
    for(n = list_head(notificationlist);
            n != NULL;
            n = list_item_next(n))
    {
        if(event == UIP_DS6_NOTIFICATION_DEFRT_ADD ||
                event == UIP_DS6_NOTIFICATION_DEFRT_RM)
        {
            num = list_length(defaultrouterlist);
        }
        else
        {
            num = num_routes;
        }
        n->callback(event, route, nexthop, num);
    }
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_notification_add(struct uip_ds6_notification *n,
        uip_ds6_notification_callback c)
{
    if(n != NULL && c != NULL)
    {
        n->callback = c;
        list_add(notificationlist, n);
    }
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_notification_rm(struct uip_ds6_notification *n)
{
    list_remove(notificationlist, n);
}
#endif //NOTIFICATIONS

/*---------------------------------------------------------------------------*/
#if UIP_DS6_SINGLE_DEFRT

uip_ds6_defrt_t uip_ds6_single_defrt;
uip_ds6_defrt_t* uip_ds6_single_defrt_p = NULL;

uip_ds6_defrt_t *uip_ds6_defrt_add(uip_ipaddr_t *ipaddr, unsigned long interval)
{
    uip_ipaddr_copy(&uip_ds6_single_defrt.ipaddr, ipaddr);
#if !UIP_DS6_INFINITE_DEFRT
    if(interval != 0)
    {
        stimer_set(&uip_ds6_single_defrt.lifetime, interval);
        uip_ds6_single_defrt.isinfinite = 0;
    }
    else
    {
        uip_ds6_single_defrt.isinfinite = 1;
    }
#else
    uip_ds6_single_defrt.isinfinite = 1;
#endif
    uip_ds6_single_defrt_p = &uip_ds6_single_defrt;
    UIP_DS6_NOTIFICATION(UIP_DS6_ROUTE_DEFRT_ADD, uip_ds6_single_defrt_p);
    return uip_ds6_single_defrt_p;
}

void uip_ds6_defrt_rm(uip_ds6_defrt_t *defrt)
{
    if (defrt == uip_ds6_single_defrt_p)
    {
        UIP_DS6_NOTIFICATION(UIP_DS6_ROUTE_DEFRT_RM, defrt);
        uip_ds6_single_defrt_p = NULL;
    }
}
/*---------------------------------------------------------------------------*/
uip_ds6_defrt_t *
uip_ds6_defrt_lookup(uip_ipaddr_t *ipaddr)
{
    if (uip_ds6_single_defrt_p != NULL)
    {
        if (uip_ipaddr_cmp(&uip_ds6_single_defrt_p->ipaddr, ipaddr))
        {
            return uip_ds6_single_defrt_p;
        }
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/
#if !UIP_DS6_INFINITE_DEFRT
void
uip_ds6_defrt_periodic(void)
{
    if (uip_ds6_single_defrt_p != NULL)
    {
        if(!uip_ds6_single_defrt_p->isinfinite &&
                stimer_expired(&uip_ds6_single_defrt_p->lifetime))
        {
            PRINTF("uip_ds6_defrt_periodic: defrt lifetime expired\n");
            uip_ds6_defrt_rm(uip_ds6_single_defrt_p);
        }
    }
}
#endif

#else //!UIP_CONF_DS6_SINGLE_DEFRT

/* Default routes are held on the defaultrouterlist and their
 structures are allocated from the defaultroutermemb memory block.*/
LIST(defaultrouterlist);
MEMB(defaultroutermemb, uip_ds6_defrt_t, UIP_DS6_DEFRT_NB);

uip_ds6_defrt_t *
uip_ds6_defrt_add(uip_ipaddr_t *ipaddr, unsigned long interval)
{
    uip_ds6_defrt_t *d;

#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */

    PRINTF("uip_ds6_defrt_add\n");
    d = uip_ds6_defrt_lookup(ipaddr);
    if(d == NULL)
    {
        d = memb_alloc(&defaultroutermemb);
        if(d == NULL)
        {
            PRINTF("uip_ds6_defrt_add: could not add default route to ");
            PRINT6ADDR(ipaddr);
            PRINTF(", out of memory\n");
            return NULL;
        }
        else
        {
            PRINTF("uip_ds6_defrt_add: adding default route to ");
            PRINT6ADDR(ipaddr);
            PRINTF("\n");
        }

        list_push(defaultrouterlist, d);
    }

    uip_ipaddr_copy(&d->ipaddr, ipaddr);
    if(interval != 0)
    {
        stimer_set(&d->lifetime, interval);
        d->isinfinite = 0;
    }
    else
    {
        d->isinfinite = 1;
    }

    UIP_DS6_NOTIFICATION(UIP_DS6_ROUTE_DEFRT_ADD, d);

#if UIP_DS6_NOTIFICATIONS
    call_route_callback(UIP_DS6_NOTIFICATION_DEFRT_ADD, ipaddr, ipaddr);
#endif

#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */

    return d;
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_defrt_rm(uip_ds6_defrt_t *defrt)
{
    uip_ds6_defrt_t *d;

#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */

    /* Make sure that the defrt is in the list before we remove it. */
    for(d = list_head(defaultrouterlist);
            d != NULL;
            d = list_item_next(d))
    {
        if(d == defrt)
        {
            PRINTF("Removing default route\n");
            //Before we free the memory call the callback
            UIP_DS6_NOTIFICATION(UIP_DS6_ROUTE_DEFRT_RM, defrt);
            list_remove(defaultrouterlist, defrt);
            memb_free(&defaultroutermemb, defrt);
            //ANNOTATE("#L %u 0\n", defrt->ipaddr.u8[sizeof(uip_ipaddr_t) - 1]);
#if UIP_DS6_NOTIFICATIONS
            call_route_callback(UIP_DS6_NOTIFICATION_DEFRT_RM,
                    &defrt->ipaddr, &defrt->ipaddr);
#endif
            return;
        }
    }
#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */

}
/*---------------------------------------------------------------------------*/
uip_ds6_defrt_t *
uip_ds6_defrt_lookup(uip_ipaddr_t *ipaddr)
{
    uip_ds6_defrt_t *d;
    for(d = list_head(defaultrouterlist);
            d != NULL;
            d = list_item_next(d))
    {
        if(uip_ipaddr_cmp(&d->ipaddr, ipaddr))
        {
            return d;
        }
    }
    return NULL;
}
/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
uip_ds6_defrt_choose(void)
{
    uip_ds6_defrt_t *d;
    uip_ds6_nbr_t *bestnbr;
    uip_ipaddr_t *addr;

    addr = NULL;
    for(d = list_head(defaultrouterlist);
            d != NULL;
            d = list_item_next(d))
    {
        PRINTF("Defrt, IP address ");
        PRINT6ADDR(&d->ipaddr);
        PRINTF("\n");
        bestnbr = uip_ds6_nbr_lookup(&d->ipaddr);
        if(bestnbr != NULL && bestnbr->state != NBR_INCOMPLETE)
        {
            PRINTF("Defrt found, IP address ");
            PRINT6ADDR(&d->ipaddr);
            PRINTF("\n");
            return &d->ipaddr;
        }
        else
        {
            addr = &d->ipaddr;
            PRINTF("Defrt INCOMPLETE found, IP address ");
            PRINT6ADDR(&d->ipaddr);
            PRINTF("\n");
        }
    }
    return addr;
}
/*---------------------------------------------------------------------------*/
#if !UIP_DS6_INFINITE_DEFRT
void
uip_ds6_defrt_periodic(void)
{
    uip_ds6_defrt_t *d;
    d = list_head(defaultrouterlist);
    while(d != NULL)
    {
        if(!d->isinfinite &&
                stimer_expired(&d->lifetime))
        {
            PRINTF("uip_ds6_defrt_periodic: defrt lifetime expired\n");
            uip_ds6_defrt_rm(d);
            d = list_head(defaultrouterlist);
        }
        else
        {
            d = list_item_next(d);
        }
    }
}
#endif

#endif //SINGLE DEFRT

/*---------------------------------------------------------------------------*/
#if (UIP_DS6_ROUTING_MODE == UIP_DS6_SOURCE_ROUTING)

static int num_routes = 0;

/* Each route is repressented by a uip_ds6_route_t structure and
 memory for each route is allocated from the routememb memory
 block. These routes are maintained on the routelist. */
LIST(routelist);
MEMB(routememb, uip_ds6_route_sourceRouting_t, UIP_DS6_ROUTE_NB);

LIST(pathlist);
MEMB(pathmemb, uip_ds6_sourceRouting_path_t, UIP_DS6_ROUTE_NB);

static int num_nodes_path = 0;

void
uip_ds6_route_sourceRouting_init(void)
{
    memb_init(&routememb);
    list_init(routelist);

    memb_init(&pathmemb);
    list_init(pathlist);

#if UIP_DS6_SINGLE_DEFRT
    uip_ds6_single_defrt_p = NULL;
#else
    memb_init(&defaultroutermemb);
    list_init(defaultrouterlist);
#endif
}

/*---------------------------------------------------------------------------*/
uip_ds6_route_sourceRouting_t *
uip_ds6_route_sourceRouting_head(void)
{
    return list_head(routelist);
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_sourceRouting_t *
uip_ds6_route_sourceRouting_next(uip_ds6_route_sourceRouting_t *r)
{
    if(r != NULL)
    {
        uip_ds6_route_sourceRouting_t *n = list_item_next(r);
        return n;
    }
    return NULL;
}
/*---------------------------------------------------------------------------*/
int
uip_ds6_route_sourceRouting_num_routes(void)
{
    return num_routes;
}
/*---------------------------------------------------------------------------*/
uip_ds6_sourceRouting_path_t *
uip_ds6_sourceRouting_path_head(void)
{
    return list_head(pathlist);
}
/*---------------------------------------------------------------------------*/
uip_ds6_sourceRouting_path_t *
uip_ds6_sourceRouting_path_next(uip_ds6_sourceRouting_path_t *r)
{
    if(r != NULL)
    {
        uip_ds6_sourceRouting_path_t *n = list_item_next(r);
        return n;
    }
    return NULL;
}
/*---------------------------------------------------------------------------*/
int
uip_ds6_sourceRouting_path_num_nodes(void)
{
    return num_nodes_path;
}
/*-------------------------------------------------*/
void cleanPath()
{
    uip_ds6_sourceRouting_path_t* path_item;
    num_nodes_path = 0;
    while (path_item = uip_ds6_sourceRouting_path_head())
    {
        list_remove(pathlist, path_item);
        memb_free(&pathmemb, path_item);
    }
}
/*-------------------------------------------------*/
int uip_ds6_route_sourceRouting_isValidPath(uip_ds6_sourceRouting_path_t *path)
{
    uip_ds6_sourceRouting_path_t* element;
    while (path)
    {
        element = uip_ds6_sourceRouting_path_next(path);
        while (element)
        {
            if (uip_ipaddr_cmp(path->ipaddr, element->ipaddr))
            {
                //If an address is repeated in the path, the path is not valid! (loop)
                return 0;
            }
            element = uip_ds6_sourceRouting_path_next(element);
        }
        path = uip_ds6_sourceRouting_path_next(path);
    }
    return 1;
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_sourceRouting_t *
uip_ds6_sourceRouting_route_lookup(uip_ipaddr_t *addr)
{
    uip_ds6_route_sourceRouting_t * found_route = NULL;
    uip_ds6_route_sourceRouting_t *r;
    u16_t longestmatch = 0;

    for(r = uip_ds6_route_sourceRouting_head();
            r != NULL;
            r = uip_ds6_route_sourceRouting_next(r))
    {
        if(r->length >= longestmatch &&
                uip_ipaddr_prefixcmp(addr, &r->ipaddr, r->length))
        {
            longestmatch = r->length;
            found_route = r;
        }
    }
    return found_route;
}
/*---------------------------------------------------------------------------*/
uip_ds6_sourceRouting_path_t *
uip_ds6_sourceRouting_path_lookup(uip_ipaddr_t *addr)
{
    uip_ds6_sourceRouting_path_t* path_item;
    uip_ds6_route_sourceRouting_t *found_route;

    PRINTF("uip-ds6-route: Looking up route for ");
    PRINT6ADDR(addr);
    PRINTF("\n");

    cleanPath();

    while (1)
    {
        //loop until finding a path or until finding there is no path (and return NULL)
        found_route = uip_ds6_sourceRouting_route_lookup(addr);

        if(found_route != NULL)
        {
            //the route (entry) was found in the table
            //add to the path (list of hops)
            path_item = memb_alloc(&pathmemb);
            if (!path_item)
            {
                PRINTF("uip-ds6-route: ERROR. No more memory for path.\n");
                cleanPath();
                return NULL;
            }
            path_item->ipaddr = &found_route->ipaddr;
            list_push(pathlist,path_item); //put the item in the beginning of the list (we are searching backwards)
            num_nodes_path++;
            //if the previous hop (parent) of this entry is this node, we completed the path so return that
            if (uip_ds6_is_my_addr(&found_route->previousHop))
            {
                return uip_ds6_sourceRouting_path_head();
            }
            else
            {
                //change the ip address that we are looking for. Now we will search the previousHop in the routing table
                addr = &found_route->previousHop;
            }
        }
        else
        {
            cleanPath();
            //the target (ip address required) or any of its ancestors, has no entry in the routing table
            return NULL;
        }

    }
}

/*---------------------------------------------------------------------------*/
uip_ds6_route_sourceRouting_t *
uip_ds6_route_sourceRouting_add(uip_ipaddr_t *ipaddr, u16_t length,
        uip_ipaddr_t *previousHop)
{
    uip_ds6_route_sourceRouting_t *r;
    /* First make sure that we don't add a route twice. If we find an
     existing route for our destination, we'll just update the old
     one. */
    r = uip_ds6_sourceRouting_route_lookup(ipaddr);
    if(r == NULL)
    {		  //Create new route if we don't have it already

        if(uip_ds6_route_sourceRouting_num_routes() == UIP_DS6_ROUTE_NB)
        {
            //If there is no more room for routes, return NULL
            UIP_STAT_ROUTE_NOT_ADDED_NUM_INC;
            return NULL;
        }

        /* Allocate a routing entry and populate it. */
        r = memb_alloc(&routememb);

        if(r == NULL)
        {
            /* This should not happen, as we explicitly deallocated one
             route table entry above. */
            PRINTF("uip_ds6_route_add: could not allocate route\n");
            UIP_STAT_ROUTE_NOT_ADDED_NUM_INC;
            return NULL;
        }

        list_add(routelist, r);

        num_routes++;

        PRINTF("uip_ds6_route_add num %d\n", num_routes);
        UIP_STAT_ROUTE_ADDED_NUM_INC;
    }

    uip_ipaddr_copy(&(r->ipaddr), ipaddr);
    uip_ipaddr_copy(&(r->previousHop), previousHop);
    r->length = length;

#ifdef UIP_DS6_ROUTE_STATE_TYPE
    memset(&r->state, 0, sizeof(UIP_DS6_ROUTE_STATE_TYPE));
#endif

    PRINTF("uip_ds6_route_add: adding route: ");
    PRINT6ADDR(ipaddr);
    PRINTF(" via ");
    PRINT6ADDR(nexthop);
    PRINTF("\n");
    //ANNOTATE("#L %u 1;blue\n", nexthop->u8[sizeof(uip_ipaddr_t) - 1]);

    UIP_DS6_NOTIFICATION(UIP_DS6_ROUTE_ADD, r);

#if UIP_DS6_NOTIFICATIONS
    call_route_callback(UIP_DS6_NOTIFICATION_ROUTE_ADD, ipaddr, previousHop);
#endif

#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */
    return r;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_route_sourceRouting_rm(uip_ds6_route_sourceRouting_t *route)
{
#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */
    if(route != NULL)
    {
        //Removing is simply removing the entry from the table. Routes depending on this route are not changed. When a Path to a certain node that depends on this route will be searched, no path will be found
        PRINTF("uip_ds6_route_rm: removing route: ");
        PRINT6ADDR(&route->ipaddr);
        PRINTF("\n");

        //Before we remove the route, call the callback
        UIP_DS6_NOTIFICATION(UIP_DS6_ROUTE_RM, route);
        /* Remove the neighbor from the route list */
        list_remove(routelist, route);
        memb_free(&routememb, route);
        num_routes--;

        PRINTF("uip_ds6_route_rm num %d\n", num_routes);

        UIP_STAT_ROUTE_DELETED_NUM_INC;

#if UIP_DS6_NOTIFICATIONS
        call_route_callback(UIP_DS6_NOTIFICATION_ROUTE_RM,
                &route->ipaddr, &route->previousHop);
#endif
    }

#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */
    return;
}

#elif (UIP_DS6_ROUTING_MODE == UIP_DS6_STANDARD_ROUTING)

static int num_routes = 0;

/* The nbr_routes holds a neighbor table to be able to maintain
 information about what routes go through what neighbor. This
 neighbor table is registered with the central nbr-table repository
 so that it will be maintained along with the rest of the neighbor
 tables in the system. */
NBR_TABLE(struct uip_ds6_route_neighbor_routes, nbr_routes,
          UIP_ROUTE_NEIGHBOR_NUM);
MEMB(neighborroutememb, struct uip_ds6_route_neighbor_route, UIP_DS6_ROUTE_NB);

/* Each route is repressented by a uip_ds6_route_t structure and
 memory for each route is allocated from the routememb memory
 block. These routes are maintained on the routelist. */
LIST(routelist);
MEMB(routememb, uip_ds6_route_t, UIP_DS6_ROUTE_NB);

static void rm_routelist_callback(nbr_table_item_t *ptr);
/*---------------------------------------------------------------------------*/
#if DEBUG != DEBUG_NONE
static void
assert_nbr_routes_list_sane(void)
{
    uip_ds6_route_t *r;
    int count;

    /* Check if the route list has an infinite loop. */
    for(r = uip_ds6_route_head(),
            count = 0;
            r != NULL &&
            count < UIP_DS6_ROUTE_NB * 2;
            r = uip_ds6_route_next(r),
            count++);

    if(count > UIP_DS6_ROUTE_NB)
    {
        printf("uip-ds6-route.c: assert_nbr_routes_list_sane route list is in infinite loop\n");
    }

    /* Make sure that the route list has as many entries as the
     num_routes vairable. */
    if(count < num_routes)
    {
        printf("uip-ds6-route.c: assert_nbr_routes_list_sane too few entries on route list: should be %d, is %d, max %d\n",
                num_routes, count, UIP_CONF_MAX_ROUTES);
    }
}
#endif /* DEBUG != DEBUG_NONE */

/*---------------------------------------------------------------------------*/
void uip_ds6_route_init(void)
{
    memb_init(&routememb);
    list_init(routelist);
    nbr_table_register(nbr_routes,
                       (nbr_table_callback *) rm_routelist_callback);

#if UIP_DS6_SINGLE_DEFRT
    uip_ds6_single_defrt_p = NULL;
#else
    memb_init(&defaultroutermemb);
    list_init(defaultrouterlist);
#endif

#if UIP_DS6_NOTIFICATIONS
    list_init(notificationlist);
#endif
}
/*---------------------------------------------------------------------------*/
static uip_lladdr_t *
uip_ds6_route_nexthop_lladdr(uip_ds6_route_t *route)
{
    if (route != NULL)
    {
        return (uip_lladdr_t *) nbr_table_get_lladdr(nbr_routes,
                                                     route->neighbor_routes);
    }
    else
    {
        return NULL;
    }
}
/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
uip_ds6_route_nexthop(uip_ds6_route_t *route)
{
    if (route != NULL)
    {
        return uip_ds6_nbr_ipaddr_from_lladdr(
                uip_ds6_route_nexthop_lladdr(route));
    }
    else
    {
        return NULL;
    }
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
uip_ds6_route_head(void)
{
    return list_head(routelist);
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
uip_ds6_route_next(uip_ds6_route_t *r)
{
    if (r != NULL)
    {
        uip_ds6_route_t *n = list_item_next(r);
        return n;
    }
    return NULL;
}
/*---------------------------------------------------------------------------*/
int uip_ds6_route_num_routes(void)
{
    return num_routes;
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
uip_ds6_route_lookup(uip_ipaddr_t *addr)
{
    uip_ds6_route_t *r;
    uip_ds6_route_t *found_route;
    u16_t longestmatch;

    PRINTF("uip-ds6-route: Looking up route for ");PRINT6ADDR(addr);PRINTF("\n");

    found_route = NULL;
    longestmatch = 0;
    for (r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r))
    {
        if (r->length >= longestmatch&&
        uip_ipaddr_prefixcmp(addr, &r->ipaddr, r->length))
        {
            longestmatch = r->length;
            found_route = r;
        }
    }

    if (found_route != NULL)
    {
        PRINTF("uip-ds6-route: Found route: ");PRINT6ADDR(addr);PRINTF(" via ");PRINT6ADDR(uip_ds6_route_nexthop(found_route));PRINTF("\n");
    }
    else
    {
        PRINTF("uip-ds6-route: No route found\n");
    }

    if (found_route != NULL)
    {
        /* If we found a route, we put it at the end of the routeslist
         list. The list is ordered by how recently we looked them up:
         the least recently used route will be at the start of the
         list. */
        list_remove(routelist, found_route);
        list_add(routelist, found_route);
    }
    return found_route;
}

/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
uip_ds6_route_lookup_by_nexthop(uip_ipaddr_t *nexthop)
{
    uip_ds6_route_t *r;
    uip_ds6_route_t *found_route;

    found_route = NULL;
    for (r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r))
    {
        if (uip_ds6_route_nexthop(r))
        {		  //if there is next hop
            if (uip_ipaddr_cmp(uip_ds6_route_nexthop(r), nexthop))
            {
                found_route = r;
                break; // if we found a route and nexthop is the next hop, return this one
            }
        }
    }

    return found_route;
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
uip_ds6_route_add(uip_ipaddr_t *ipaddr, uint16_t length, uip_ipaddr_t *nexthop)
{
    uip_ds6_route_t *r;
    struct uip_ds6_route_neighbor_route *nbrr;

#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */

    /* Get link-layer address of next hop, make sure it is in neighbor table */
    const uip_lladdr_t *nexthop_lladdr = uip_ds6_nbr_lladdr_from_ipaddr(
            nexthop);
    if (nexthop_lladdr == NULL)
    {
        PRINTF("uip_ds6_route_add: neighbor link-local address unknown for ");PRINT6ADDR(nexthop);PRINTF("\n");
        return NULL;
    }

    new_route_new_nexthop:

    /* First make sure that we don't add a route twice. If we find an
     existing route for our destination, we'll just update the old
     one. */
    r = uip_ds6_route_lookup(ipaddr);
    if (r != NULL)
    {
        PRINTF("uip_ds6_route_add: old route already found, updating this one instead: ");PRINT6ADDR(ipaddr);PRINTF("\n");
        //If next hop is not the same as is was in the route we have for that destination, delete and add the new route
        if (uip_ds6_route_nexthop(
                r)&&!uip_ipaddr_cmp(uip_ds6_route_nexthop(r), nexthop))
        {
            //incase nexthop is alreay invalid
            uip_ds6_route_rm(r);
            goto new_route_new_nexthop;
        }

    }
    else
    {
        struct uip_ds6_route_neighbor_routes *routes;
        /* If there is no routing entry, create one. We first need to
         check if we have room for this route. If not, we remove the
         least recently used one we have. */

        if (uip_ds6_route_num_routes() == UIP_DS6_ROUTE_NB)
        {
            /* Removing the oldest route entry from the route table. The
             least recently used route is the first route on the list. */
            uip_ds6_route_t *oldest;

            oldest = uip_ds6_route_head();
            PRINTF("uip_ds6_route_add: dropping route to ");PRINT6ADDR(&oldest->ipaddr);PRINTF("\n");
            uip_ds6_route_rm(oldest);
        }

        /* Every neighbor on our neighbor table holds a struct
         uip_ds6_route_neighbor_routes which holds a list of routes that
         go through the neighbor. We add our route entry to this list.

         We first check to see if we already have this neighbor in our
         nbr_route table. If so, the neighbor already has a route entry
         list.
         */
        routes = nbr_table_get_from_lladdr(nbr_routes,
                                           (rimeaddr_t *) nexthop_lladdr);

        if (routes == NULL)
        {
            /* If the neighbor did not have an entry in our neighbor table,
             we create one. The nbr_table_add_lladdr() function returns a
             pointer to a pointer that we may use for our own purposes. We
             initialize this pointer with the list of routing entries that
             are attached to this neighbor. */
            routes = nbr_table_add_lladdr(nbr_routes,
                                          (rimeaddr_t *) nexthop_lladdr);
            if (routes == NULL)
            {
                /* This should not happen, as we explicitly deallocated one
                 route table entry above. */
                PRINTF("uip_ds6_route_add: could not allocate neighbor table entry\n");

                UIP_STAT_ROUTE_NOT_ADDED_NUM_INC;
                return NULL;
            }
            LIST_STRUCT_INIT(routes, route_list);
        }

        /* Allocate a routing entry and populate it. */
        r = memb_alloc(&routememb);

        if (r == NULL)
        {
            /* This should not happen, as we explicitly deallocated one
             route table entry above. */
            PRINTF("uip_ds6_route_add: could not allocate route\n");

            UIP_STAT_ROUTE_NOT_ADDED_NUM_INC;
            return NULL;
        }

        list_add(routelist, r);

        nbrr = memb_alloc(&neighborroutememb);

        if (nbrr == NULL)
        {
            /* This should not happen, as we explicitly deallocated one
             route table entry above. */
            PRINTF("uip_ds6_route_add: could not allocate neighbor route list entry\n");
            memb_free(&routememb, r);

            UIP_STAT_ROUTE_NOT_ADDED_NUM_INC;
            return NULL;
        }

        nbrr->route = r;
        /* Add the route to this neighbor */
        list_add(routes->route_list, nbrr);
        r->neighbor_routes = routes;
        num_routes++;

        PRINTF("uip_ds6_route_add num %d\n", num_routes);

        UIP_STAT_ROUTE_ADDED_NUM_INC;
    }

    uip_ipaddr_copy(&(r->ipaddr), ipaddr);
    r->length = length;

#ifdef UIP_DS6_ROUTE_STATE_TYPE
    memset(&r->state, 0, sizeof(UIP_DS6_ROUTE_STATE_TYPE));
#endif

    PRINTF("uip_ds6_route_add: adding route: ");PRINT6ADDR(ipaddr);PRINTF(" via ");PRINT6ADDR(nexthop);PRINTF("\n");
    //ANNOTATE("#L %u 1;blue\n", nexthop->u8[sizeof(uip_ipaddr_t) - 1]);
    UIP_DS6_NOTIFICATION(UIP_DS6_ROUTE_ADD, r);

#if UIP_DS6_NOTIFICATIONS
    call_route_callback(UIP_DS6_NOTIFICATION_ROUTE_ADD, ipaddr, nexthop);
#endif

#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */
    return r;

}

/*---------------------------------------------------------------------------*/
void uip_ds6_route_rm(uip_ds6_route_t *route)
{
    struct uip_ds6_route_neighbor_route *neighbor_route;
#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */
    if (route != NULL && route->neighbor_routes != NULL)
    {

        PRINTF("uip_ds6_route_rm: removing route: ");PRINT6ADDR(&route->ipaddr);PRINTF("\n");

        /* Remove the neighbor from the route list */
        list_remove(routelist, route);

        /* Find the corresponding neighbor_route and remove it. */
        for (neighbor_route = list_head(route->neighbor_routes->route_list);
                neighbor_route != NULL && neighbor_route->route != route;
                neighbor_route = list_item_next(neighbor_route))
            ;

        if (neighbor_route == NULL)
        {
            PRINTF("uip_ds6_route_rm: neighbor_route was NULL for ");
            //uip_debug_ipaddr_print(&route->ipaddr);
            PRINT6ADDR(&route->ipaddr);PRINTF("\n");
        }
        list_remove(route->neighbor_routes->route_list, neighbor_route);
        if (list_head(route->neighbor_routes->route_list) == NULL)
        {
            /* If this was the only route using this neighbor, remove the
             neibhor from the table */
            PRINTF("uip_ds6_route_rm: removing neighbor too\n");
            nbr_table_remove(nbr_routes, route->neighbor_routes->route_list);
        }
        //Before we free the memory, call the route callback
        UIP_DS6_NOTIFICATION(UIP_DS6_ROUTE_RM, route);

        memb_free(&routememb, route);
        memb_free(&neighborroutememb, neighbor_route);

        num_routes--;

        PRINTF("uip_ds6_route_rm num %d\n", num_routes);

        UIP_STAT_ROUTE_DELETED_NUM_INC;

#if UIP_DS6_NOTIFICATIONS
        call_route_callback(UIP_DS6_NOTIFICATION_ROUTE_RM,
                &route->ipaddr, uip_ds6_route_nexthop(route));
#endif
    }

#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */
    return;

}
/*---------------------------------------------------------------------------*/
static void rm_routelist(struct uip_ds6_route_neighbor_routes *routes)
{
#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */
    PRINTF("uip_ds6_route_rm_routelist\n");
    if (routes != NULL && routes->route_list != NULL)
    {
        struct uip_ds6_route_neighbor_route *r;
        r = list_head(routes->route_list);
        while (r != NULL)
        {
            uip_ds6_route_rm(r->route);
            r = list_head(routes->route_list);
        }
        nbr_table_remove(nbr_routes, routes);
    }
#if DEBUG != DEBUG_NONE
    assert_nbr_routes_list_sane();
#endif /* DEBUG != DEBUG_NONE */

}
/*---------------------------------------------------------------------------*/
static void rm_routelist_callback(nbr_table_item_t *ptr)
{
    rm_routelist((struct uip_ds6_route_neighbor_routes *) ptr);
}
/*---------------------------------------------------------------------------*/
void uip_ds6_route_rm_by_nexthop(uip_ipaddr_t *nexthop)
{
    /* Get routing entry list of this neighbor */
    const uip_lladdr_t *nexthop_lladdr;
    struct uip_ds6_route_neighbor_routes *routes;

    nexthop_lladdr = uip_ds6_nbr_lladdr_from_ipaddr(nexthop);
    routes = nbr_table_get_from_lladdr(nbr_routes,
                                       (rimeaddr_t *) nexthop_lladdr);
    rm_routelist(routes);

}

#endif //UIP_DS6_STANDARD_ROUTING

#endif /* UIP_CONF_IPV6 */
