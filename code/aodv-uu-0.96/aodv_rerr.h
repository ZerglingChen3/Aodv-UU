/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University & Ericsson AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Erik Nordstr�m, <erik.nordstrom@it.uu.se>
 *          
 *
 *****************************************************************************/
#ifndef _AODV_RERR_H
#define _AODV_RERR_H

#ifndef NS_NO_GLOBALS
#include <endian.h>

#include "defs.h"
#include "routing_table.h"

/* RERR Flags: */
#define RERR_NODELETE 0x1
//modified by mjw
#define RERR_LOCAL_REPAIR 0x2

typedef struct {
    u_int8_t channel;
    u_int8_t type;
#if defined(__LITTLE_ENDIAN)
    //modified by
    u_int8_t res1:6;
    u_int8_t lr:1;
    //mjw
    u_int8_t n:1;
#elif defined(__BIG_ENDIAN)
    u_int8_t n:1;
    //modified by
    uint8_t lr:1;
    u_int8_t res1:6;
    //mjw
#error "Adjust your <bits/endian.h> defines"
#endif
    u_int8_t res2;
    u_int8_t dest_count;
    u_int32_t dest_addr;
    u_int32_t dest_seqno;
    u_int32_t rerr_id;
    u_int32_t rerr_origin_addr;
}  RERR;

#define RERR_SIZE sizeof(RERR)

/* Extra unreachable destinations... */
typedef struct {
    u_int32_t dest_addr;
    u_int32_t dest_seqno;
} RERR_udest;

struct rerr_record {
    list_t l;
    struct in_addr orig_addr;	/* Source of the RRER */
    u_int32_t rerr_id;		/* RERR's broadcast ID */
    u_int32_t last_addr;
    struct timer rec_timer;
};

#define RERR_UDEST_SIZE sizeof(RERR_udest)

/* Given the total number of unreachable destination this macro
   returns the RERR size */
#define RERR_CALC_SIZE(rerr) (RERR_SIZE + (rerr->dest_count-1)*RERR_UDEST_SIZE)
#define RERR_UDEST_FIRST(rerr) ((RERR_udest *)&rerr->dest_addr)
#define RERR_UDEST_NEXT(udest) ((RERR_udest *)((char *)udest + RERR_UDEST_SIZE))
#endif				/* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS
RERR *rerr_create(u_int8_t flags, struct in_addr dest_addr,
		  u_int32_t dest_seqno);
void rerr_add_udest(RERR * rerr, struct in_addr udest, u_int32_t udest_seqno);
void rerr_process(RERR * rerr, int rerrlen, struct in_addr ip_src,
		  struct in_addr ip_dst);
void rerr_record_timeout(void *arg);

//mjw
#ifdef NS_PORT
struct rerr_record *rerr_record_insert(struct in_addr orig_addr,
				       u_int32_t rerr_id, u_int32_t last_addr);
struct rerr_record *rerr_record_find(struct in_addr orig_addr,
				     u_int32_t rerr_id, u_int32_t last_addr);

#endif


#endif				/* NS_NO_DECLARATIONS */

#endif				/* AODV_RERR_H */
