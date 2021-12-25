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

#ifdef NS_PORT
#include "ns-2/aodv-uu.h"
#include <math.h>
#else
#include <netinet/in.h>
#include "aodv_rrep.h"
#include "aodv_neighbor.h"
#include "aodv_hello.h"
#include "routing_table.h"
#include "aodv_timeout.h"
#include "timer_queue.h"
#include "aodv_socket.h"
#include "defs.h"
#include "debug.h"
#include "params.h"
#define MJW_DEBUG

extern int unidir_hack, optimized_hellos, llfeedback;

#endif

RREP *NS_CLASS rrep_create(u_int8_t flags,
			   u_int8_t prefix,
			   u_int8_t hcnt,
			   struct in_addr dest_addr,
			   u_int32_t dest_seqno,
			   struct in_addr orig_addr, u_int32_t life)
{
    RREP *rrep;

    rrep = (RREP *) aodv_socket_new_msg();
    rrep->type = AODV_RREP;
    rrep->res1 = 0;
    rrep->res2 = 0;
    rrep->prefix = prefix;
    rrep->hcnt = hcnt;
    rrep->dest_addr = dest_addr.s_addr;
    rrep->dest_seqno = htonl(dest_seqno);
    rrep->orig_addr = orig_addr.s_addr;
    rrep->lifetime = htonl(life);
	rrep->dest_count = 1;

    if (flags & RREP_REPAIR)
	rrep->r = 1;
    if (flags & RREP_ACK)
	rrep->a = 1;
	//modified by
	if (flags & RREP_LOCAL_REPAIR){
		rrep->lr = 1;
		rrep->type = AODV_RREP_LR;
		/*#ifdef MJW_DEBUG
		printf("创建路由修复rrep dest:%d origin:%d \n",dest_addr.s_addr, orig_addr.s_addr);
		#endif */
	}

    /* Don't print information about hello messages... */
#ifdef DEBUG_OUTPUT
    if (rrep->dest_addr != rrep->orig_addr) {
	DEBUG(LOG_DEBUG, 0, "Assembled RREP:");
	log_pkt_fields((AODV_msg *) rrep);
    }
#endif

    return rrep;
}

/* modifed by chenjiyuan 11.26 */
RREP *NS_CLASS rrep_create_with_cost(u_int8_t flags,
                           u_int8_t prefix,
                           u_int8_t hcnt,
                           struct in_addr dest_addr,
                           u_int32_t dest_seqno,
                           struct in_addr orig_addr,
                           u_int32_t life,
                           double cost)
{
    RREP *rrep;

    rrep = (RREP *) aodv_socket_new_msg();
    rrep->type = AODV_RREP;
    rrep->res1 = 0;
    rrep->res2 = 0;
    rrep->prefix = prefix;
    rrep->hcnt = hcnt;
    rrep->dest_addr = dest_addr.s_addr;
    rrep->dest_seqno = htonl(dest_seqno);
    rrep->orig_addr = orig_addr.s_addr;
    rrep->lifetime = htonl(life);

    if (flags & RREP_REPAIR)
        rrep->r = 1;
    if (flags & RREP_ACK)
        rrep->a = 1;

    AODV_ext* ext = rrep_add_ext(rrep, RREP_COST_EXT, RREP_SIZE, sizeof(cost), (char*)&cost);

    /* Don't print information about hello messages... */
#ifdef DEBUG_OUTPUT
    if (rrep->dest_addr != rrep->orig_addr) {
	DEBUG(LOG_DEBUG, 0, "Assembled RREP:");
	log_pkt_fields((AODV_msg *) rrep);
    }
#endif

    return rrep;
}
/* end modifed*/

RREP_ack *NS_CLASS rrep_ack_create()
{
    RREP_ack *rrep_ack;

    rrep_ack = (RREP_ack *) aodv_socket_new_msg();
    rrep_ack->type = AODV_RREP_ACK;
//    if(0) // switch to use this feature or not
    {//modified by XY
        rrep_ack->is_hello_ack = 0;
		rrep_ack->hello_index = 0;
        rrep_ack->host_stability = 0;
    }    
	DEBUG(LOG_DEBUG, 0, "Assembled RREP_ack");

    return rrep_ack;
}

void NS_CLASS rrep_ack_process(RREP_ack * rrep_ack, int rrep_acklen,
			       struct in_addr ip_src, struct in_addr ip_dst)
{
	printf("---------------------------------------------------------------------------------\n");
    /* modified by chenjiyuan at 11.29 */
    rt_table_t *rt = rt_table_find(ip_src);
    /* end modified at 11.29 */

//if (0) // switch to use this feature or not
    { //modified by XY
        if (rrep_ack->is_hello_ack)
        {
            printf("[%.9f] %d received Hello_ack from %d\n", Scheduler::instance().clock(),this_host.devs[0].ipaddr, ip_src);
            int neib_index=0;
            for(neib_index=0;neib_index<this_host.neighbor_num;++neib_index){
                if(this_host.neighbors[neib_index].ipaddr.s_addr==ip_src.s_addr){
                    break;
                }
            }
            if(rrep_ack->hello_index >= this_host.hello_head)
            {
                this_host.neighbors[neib_index].host_stability_sequence[rrep_ack->hello_index % MAX_SEQUENCE_LEN] = rrep_ack->host_stability;
				this_host.neighbors[neib_index].channel_hello_sequence[rrep_ack->channel][rrep_ack->hello_index % MAX_SEQUENCE_LEN] = 1;
            }
            this_host.neighbors[neib_index].channel_hello_remote_received[rrep_ack->channel] = rrep_ack->channel_hello_received;
			// printf("[%.9f] remote %d received %d hellos from %d, now hello index is %d\n",Scheduler::instance().clock(), this_host.neighbors[neib_index].ipaddr.s_addr, this_host.neighbors[neib_index].channel_hello_remote_received[rrep_ack->channel], this_host.devs[0].ipaddr.s_addr,);
            return;
		}
		printf("---------------------------------------------------------------------------\n");
    }
    if (rt == NULL) {
	DEBUG(LOG_WARNING, 0, "No RREP_ACK expected for %s", ip_to_str(ip_src));

	return;
    }
	
    DEBUG(LOG_DEBUG, 0, "Received RREP_ACK from %s", ip_to_str(ip_src));

    /* Remove unexpired timer for this RREP_ACK */
    timer_remove(&rt->ack_timer);
}

AODV_ext *NS_CLASS rrep_add_ext(RREP * rrep, int type, unsigned int offset,
				int len, char *data)
{
    AODV_ext *ext = NULL;

    if (offset < RREP_SIZE)
	return NULL;

    ext = (AODV_ext *) ((char *) rrep + offset);

    ext->type = type;
    ext->length = len;

    memcpy(AODV_EXT_DATA(ext), data, len);

    return ext;
}

//modified by mjw
void NS_CLASS rrep_add_udest(RREP * rrep, struct in_addr udest,
			     u_int32_t udest_seqno)
{
    RREP_udest ud;

    ud.dest_addr = udest.s_addr;
    ud.dest_seqno = htonl(udest_seqno);

	#ifdef MJW_DEBUG
		printf("rrep_add_udest: rrep_origin:%d, udest:%d\n",rrep->orig_addr, udest.s_addr);
	#endif
	rrep_add_ext(rrep, RREP_UDEST_EXT, RREP_EXT_OFFSET(rrep), RREP_UDEST_SIZE, (char*)&ud);

    rrep->dest_count++;
}
//end modified


/* modifed by chenjiyuan 11.29 */
void NS_CLASS rrep_send_with_channel(RREP * rrep, rt_table_t * rev_rt,
                        rt_table_t * fwd_rt, int size, int channel)
{
    u_int8_t rrep_flags = 0;
    struct in_addr dest;

    if (!rev_rt) {
        DEBUG(LOG_WARNING, 0, "Can't send RREP, rev_rt = NULL!");
        return;
    }

    dest.s_addr = rrep->dest_addr;

    /* Check if we should request a RREP-ACK */
    if ((rev_rt->state == VALID && rev_rt->flags & RT_UNIDIR) ||
        (rev_rt->hcnt == 1 && unidir_hack)) {
        rt_table_t *neighbor = rt_table_find(rev_rt->next_hop);

        if (neighbor && neighbor->state == VALID && !neighbor->ack_timer.used) {
            /* If the node we received a RREQ for is a neighbor we are
               probably facing a unidirectional link... Better request a
               RREP-ack */
            rrep_flags |= RREP_ACK;
            neighbor->flags |= RT_UNIDIR;

            /* Must remove any pending hello timeouts when we set the
               RT_UNIDIR flag, else the route may expire after we begin to
               ignore hellos... */
            timer_remove(&neighbor->hello_timer);
            neighbor_link_break(neighbor);

            DEBUG(LOG_DEBUG, 0, "Link to %s is unidirectional!",
                  ip_to_str(neighbor->dest_addr));

            timer_set_timeout(&neighbor->ack_timer, NEXT_HOP_WAIT);
        }
    }

    DEBUG(LOG_DEBUG, 0, "Sending RREP to next hop %s about %s->%s",
          ip_to_str(rev_rt->next_hop), ip_to_str(rev_rt->dest_addr),
          ip_to_str(dest));

    //todo: set channel
    channelNum = channel;
    //printf("set channel in rrep send : %d\n", channel);

    aodv_socket_send((AODV_msg *) rrep, rev_rt->next_hop, size, MAXTTL,
                     &DEV_IFINDEX(rev_rt->ifindex));

    /* Update precursor lists */
    if (fwd_rt) {
        precursor_add(fwd_rt, rev_rt->next_hop);
        precursor_add(rev_rt, fwd_rt->next_hop);
    }

    if (!llfeedback && optimized_hellos)
        hello_start();
}
/* end modified at 11.29 */

void NS_CLASS rrep_send(RREP * rrep, rt_table_t * rev_rt,
			rt_table_t * fwd_rt, int size)
{
    u_int8_t rrep_flags = 0;
    struct in_addr dest;

    if (!rev_rt) {
	DEBUG(LOG_WARNING, 0, "Can't send RREP, rev_rt = NULL!");
	return;
    }

    dest.s_addr = rrep->dest_addr;

    /* Check if we should request a RREP-ACK */
    if ((rev_rt->state == VALID && rev_rt->flags & RT_UNIDIR) ||
	(rev_rt->hcnt == 1 && unidir_hack)) {
	rt_table_t *neighbor = rt_table_find(rev_rt->next_hop);

	if (neighbor && neighbor->state == VALID && !neighbor->ack_timer.used) {
	    /* If the node we received a RREQ for is a neighbor we are
	       probably facing a unidirectional link... Better request a
	       RREP-ack */
	    rrep_flags |= RREP_ACK;
	    neighbor->flags |= RT_UNIDIR;

	    /* Must remove any pending hello timeouts when we set the
	       RT_UNIDIR flag, else the route may expire after we begin to
	       ignore hellos... */
	    timer_remove(&neighbor->hello_timer);
	    neighbor_link_break(neighbor);

	    DEBUG(LOG_DEBUG, 0, "Link to %s is unidirectional!",
		  ip_to_str(neighbor->dest_addr));

	    timer_set_timeout(&neighbor->ack_timer, NEXT_HOP_WAIT);
	}
    }

    DEBUG(LOG_DEBUG, 0, "Sending RREP to next hop %s about %s->%s",
	  ip_to_str(rev_rt->next_hop), ip_to_str(rev_rt->dest_addr),
	  ip_to_str(dest));

    aodv_socket_send((AODV_msg *) rrep, rev_rt->next_hop, size, MAXTTL,
		     &DEV_IFINDEX(rev_rt->ifindex));

    /* Update precursor lists */
    if (fwd_rt) {
	precursor_add(fwd_rt, rev_rt->next_hop);
	precursor_add(rev_rt, fwd_rt->next_hop);
    }

    if (!llfeedback && optimized_hellos)
	hello_start();
}

void NS_CLASS rrep_forward(RREP * rrep, int size, rt_table_t * rev_rt,
			   rt_table_t * fwd_rt, int ttl)
{
    /* Sanity checks... */
    if (!fwd_rt || !rev_rt) {
	DEBUG(LOG_WARNING, 0, "Could not forward RREP because of NULL route!");
	return;
    }

    if (!rrep) {
	DEBUG(LOG_WARNING, 0, "No RREP to forward!");
	return;
    }

    DEBUG(LOG_DEBUG, 0, "Forwarding RREP to %s", ip_to_str(rev_rt->next_hop));

    /* Here we should do a check if we should request a RREP_ACK,
       i.e we suspect a unidirectional link.. But how? */
    if (0) {
	rt_table_t *neighbor;

	/* If the source of the RREP is not a neighbor we must find the
	   neighbor (link) entry which is the next hop towards the RREP
	   source... */
	if (rev_rt->dest_addr.s_addr != rev_rt->next_hop.s_addr)
	    neighbor = rt_table_find(rev_rt->next_hop);
	else
	    neighbor = rev_rt;

	if (neighbor && !neighbor->ack_timer.used) {
	    /* If the node we received a RREQ for is a neighbor we are
	       probably facing a unidirectional link... Better request a
	       RREP-ack */
	    rrep->a = 1;
	    neighbor->flags |= RT_UNIDIR;

	    timer_set_timeout(&neighbor->ack_timer, NEXT_HOP_WAIT);
	}
    }

    rrep = (RREP *) aodv_socket_queue_msg((AODV_msg *) rrep, size);
    rrep->hcnt = fwd_rt->hcnt;	/* Update the hopcount */

    /* modified by chenjiyuan at 11.30 */
    /*aodv_socket_send((AODV_msg *) rrep, rev_rt->next_hop, size, ttl,
		     &DEV_IFINDEX(rev_rt->ifindex));*/
    aodv_socket_send((AODV_msg *) rrep, rev_rt->next_hop, size, ttl,
                     &DEV_IFINDEX(rev_rt->ifindex));
    /* end modified at 11.30 */

    precursor_add(fwd_rt, rev_rt->next_hop);
    precursor_add(rev_rt, fwd_rt->next_hop);

    rt_table_update_timeout(rev_rt, ACTIVE_ROUTE_TIMEOUT);
}

//modified by mjw
void NS_CLASS rrep_process_lr(RREP * rrep, int rreplen, struct in_addr ip_src,
			   struct in_addr ip_dst, int ip_ttl,
			   unsigned int ifindex)
{
	#ifdef MJW_DEBUG
	printf("开始处理修复rrep 来自：%d, dst:%d origin:%d, ttl:%d\n"
		, ip_src.s_addr, rrep->dest_addr, rrep->orig_addr, ip_ttl);
	#endif
	u_int32_t rrep_lifetime, rrep_seqno, rrep_new_hcnt;
    u_int8_t pre_repair_hcnt = 0, pre_repair_flags = 0;
    rt_table_t *fwd_rt, *rev_rt;
    AODV_ext *ext;
    unsigned int extlen = 0;
    int rt_flags = 0;
    struct in_addr rrep_dest, rrep_orig;
#ifdef CONFIG_GATEWAY
    struct in_addr inet_dest_addr;
    int inet_rrep = 0;
#endif

    /* Convert to correct byte order on affeected fields: */
    rrep_dest.s_addr = rrep->dest_addr;
    rrep_orig.s_addr = rrep->orig_addr;
    rrep_seqno = ntohl(rrep->dest_seqno);
    rrep_lifetime = ntohl(rrep->lifetime);
    /* Increment RREP hop count to account for intermediate node... */
    rrep_new_hcnt = rrep->hcnt + 1;

    if (rreplen < (int) RREP_SIZE) {
	alog(LOG_WARNING, 0, __FUNCTION__,
	     "IP data field too short (%u bytes)"
	     " from %s to %s", rreplen, ip_to_str(ip_src), ip_to_str(ip_dst));
	return;
    }

    /* Ignore messages which aim to a create a route to one self */
    if (rrep_dest.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr)
	return;

    DEBUG(LOG_DEBUG, 0, "from %s about %s->%s",
	  ip_to_str(ip_src), ip_to_str(rrep_orig), ip_to_str(rrep_dest));
#ifdef DEBUG_OUTPUT
    log_pkt_fields((AODV_msg *) rrep);
#endif

    /* Determine whether there are any extensions */
    ext = (AODV_ext *) ((char *) rrep + RREP_SIZE);
    while ((rreplen - extlen) > RREP_SIZE) {
	//modified by mjw 
	printf("need to deal rrep_ext\n");
	RREP_udest *ud = (RREP_udest*) (((char*)ext) + AODV_EXT_HDR_SIZE);
	switch (ext->type) {
	case RREP_UDEST_EXT:
	    DEBUG(LOG_INFO, 0, "RREP include EXTENSION");
		printf("%d %d\n",rrep_orig.s_addr, DEV_IFINDEX(ifindex).ipaddr.s_addr);
		if(!(rrep_orig.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr)) {
			printf("on the way to origin, add route to dest :%d\n", ud->dest_addr);
			struct in_addr ud_dest;
			ud_dest.s_addr = ud->dest_addr;
			fwd_rt = rt_table_find(ud_dest);
			if (!fwd_rt) {
				fwd_rt = rt_table_insert(ud_dest, ip_src, rrep_new_hcnt, rrep_seqno,
				 rrep_lifetime, VALID, 0, ifindex);
				rt_table_update(fwd_rt, ip_src, rrep_new_hcnt, rrep_seqno,
				 	rrep_lifetime, VALID,
				 	(fwd_rt->flags)&(!RT_REPAIR));
			} else {
				fwd_rt = rt_table_update(fwd_rt, ip_src, rrep_new_hcnt, rrep_seqno,
				 rrep_lifetime, VALID,
				 0 | fwd_rt->flags);
			}
		}
	    /* Do something here */
	    break;
#ifdef CONFIG_GATEWAY
	case RREP_INET_DEST_EXT:
	    if (ext->length == sizeof(u_int32_t)) {

		/* Destination address in RREP is the gateway address, while the
		 * extension holds the real destination */
		memcpy(&inet_dest_addr, AODV_EXT_DATA(ext), ext->length);

		DEBUG(LOG_DEBUG, 0, "RREP_INET_DEST_EXT: <%s>",
		      ip_to_str(inet_dest_addr));
		/* This was a RREP from a gateway */
		rt_flags |= RT_GATEWAY;
		inet_rrep = 1;
		break;
	    }
#endif
	default:
	    alog(LOG_WARNING, 0, __FUNCTION__, "Unknown or bad extension %d",
		 ext->type);
	    break;
	}
	extlen += AODV_EXT_SIZE(ext);
	ext = AODV_EXT_NEXT(ext);
    }

    /* ---------- CHECK IF WE SHOULD MAKE A FORWARD ROUTE ------------ */

    fwd_rt = rt_table_find(rrep_dest);
    rev_rt = rt_table_find(rrep_orig);
	
	//modifiedy by mjw
	if (rrep_orig.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr) {
		goto deal_equal;
	}


    if (!fwd_rt) {
	#ifdef MJW_DEBUG
		printf("修复rrep在路上的节点 插入新路由表项 dest:%d, next:%d\n",rrep_dest.s_addr, ip_src.s_addr);
	#endif
	/* We didn't have an existing entry, so we insert a new one. */
	fwd_rt = rt_table_insert(rrep_dest, ip_src, rrep_new_hcnt, rrep_seqno,
				 rrep_lifetime, VALID, 0, ifindex);
	rt_table_update(fwd_rt, ip_src, rrep_new_hcnt, rrep_seqno,
				 	rrep_lifetime, VALID,
				 	(fwd_rt->flags)&(!RT_REPAIR));
    } else if (1/*fwd_rt->dest_seqno == 0 ||
	       (int32_t) rrep_seqno > (int32_t) fwd_rt->dest_seqno ||
	       (rrep_seqno == fwd_rt->dest_seqno &&
		(fwd_rt->state == INVALID || fwd_rt->flags & RT_UNIDIR ||
		 rrep_new_hcnt < fwd_rt->hcnt))*/) {
	#ifdef MJW_DEBUG
		printf("修复rrep在路上的节点 更新路由表项 dest:%d, next:%d\n",fwd_rt->dest_addr.s_addr, ip_src.s_addr);
	#endif
	pre_repair_hcnt = fwd_rt->hcnt;
	pre_repair_flags = fwd_rt->flags;

	fwd_rt = rt_table_update(fwd_rt, ip_src, rrep_new_hcnt, rrep_seqno,
				 rrep_lifetime, VALID,
				 0 | fwd_rt->flags);
    } else {
	if (fwd_rt->hcnt > 1) {
	    DEBUG(LOG_DEBUG, 0,
		  "Dropping RREP, fwd_rt->hcnt=%d fwd_rt->seqno=%ld",
		  fwd_rt->hcnt, fwd_rt->dest_seqno);
	}
	return;
    }


    /* If the RREP_ACK flag is set we must send a RREP
       acknowledgement to the destination that replied... */
    if (rrep->a) {
	RREP_ack *rrep_ack;

	rrep_ack = rrep_ack_create();
	aodv_socket_send((AODV_msg *) rrep_ack, fwd_rt->next_hop,
			 NEXT_HOP_WAIT, MAXTTL, &DEV_IFINDEX(fwd_rt->ifindex));
	/* Remove RREP_ACK flag... */
	rrep->a = 0;
    }

	//modified by mjw 
	deal_equal:

    /* Check if this RREP was for us (i.e. we previously made a RREQ
       for this host). */
    if (rrep_orig.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr) {
#ifdef CONFIG_GATEWAY
	if (inet_rrep) {
	    rt_table_t *inet_rt;
	    inet_rt = rt_table_find(inet_dest_addr);

	    /* Add a "fake" route indicating that this is an Internet
	     * destination, thus should be encapsulated and routed through a
	     * gateway... */
	    if (!inet_rt)
		rt_table_insert(inet_dest_addr, rrep_dest, rrep_new_hcnt, 0,
				rrep_lifetime, VALID, RT_INET_DEST, ifindex);
	    else if (inet_rt->state == INVALID || rrep_new_hcnt < inet_rt->hcnt) {
		rt_table_update(inet_rt, rrep_dest, rrep_new_hcnt, 0,
				rrep_lifetime, VALID, RT_INET_DEST |
				inet_rt->flags);
	    } else {
		DEBUG(LOG_DEBUG, 0, "INET Response, but no update %s",
		      ip_to_str(inet_dest_addr));
	    }
	}
#endif				/* CONFIG_GATEWAY */

	/* If the route was previously in repair, a NO DELETE RERR should be
	   sent to the source of the route, so that it may choose to reinitiate
	   route discovery for the destination. Fixed a bug here that caused the
	   repair flag to be unset and the RERR never being sent. Thanks to
	   McWood <hjw_5@hotmail.com> for discovering this. */

	// modified by mjw
	#ifdef MJW_DEBUG
	printf("修复rrep 到达源节点\n");
	#endif

	#ifdef MJW_DEBUG
					{int ii;
					for (ii = 0; ii < RT_TABLESIZE; ii++) {
						list_t *pos1;
						list_foreach(pos1, &rt_tbl.tbl[ii]) {
	    					rt_table_t *rt_u1 = (rt_table_t *) pos1;
							printf("更新之前当前路由表项 dest:%d next:%d valid:%d flags:%d\n",
									rt_u1->dest_addr.s_addr,rt_u1->next_hop.s_addr, rt_u1->state, rt_u1->flags);
						}
					}}
	#endif
	int i;
	RERR* rerr = NULL;
	struct in_addr rerr_unicast_dest;
	printf("fuck!!!!!! %d\n", ip_src.s_addr);
	for (i = 0; i < RT_TABLESIZE; i++) {
		list_t *pos;
		list_foreach(pos, &rt_tbl.tbl[i]) {
	    	rt_table_t *rt_u = (rt_table_t *) pos;
			
			if(rt_u->flags & RT_REPAIR)
			{
				//要不要考虑序列号呢
				if((rt_u->dest_addr).s_addr == rrep_dest.s_addr && (rt_u->flags&RT_REPAIR)/*&& 
				(rt_u->dest_seqno == 0 ||
	       		(int32_t) rrep_seqno > (int32_t) rt_u->dest_seqno ||
	       		(rrep_seqno == rt_u->dest_seqno && rrep_new_hcnt < fwd_rt->hcnt))*/) {
					printf("fuck1!!!!!! %d\n", ip_src.s_addr);

					rt_table_update(rt_u, ip_src, rrep_new_hcnt, rrep_seqno,
				 	rrep_lifetime, VALID,
				 	0);
					#ifdef MJW_DEBUG
					printf("更新路由表项 dest:%d new_hcnt:%d, next:%d\n"
							,rt_u->dest_addr.s_addr,rrep_new_hcnt,ip_src.s_addr);
					#endif
				} 
				else if((rt_u->next_hop).s_addr == rrep_dest.s_addr && (rt_u->flags&RT_REPAIR)) {
					printf("fuck2!!!!!! %d\n", ip_src.s_addr);

					rt_table_update(rt_u, ip_src, rrep_new_hcnt-1+rt_u->hcnt, rt_u->dest_seqno,
				 	rrep_lifetime, VALID,
				 	0);
					printf("fuck3!!!!!! addr: %d ttl: %d\n", ip_src.s_addr, rrep_new_hcnt-1+rt_u->hcnt);

					#ifdef MJW_DEBUG
					printf("更新路由表项 dest:%d new_hcnt:%d, next:%d\n"
							,rt_u->dest_addr.s_addr,rrep_new_hcnt-1+rt_u->hcnt,ip_src.s_addr);
					#endif
				} else{
					continue;
				}
				#ifdef MJW_DEBUG
					int ii;
					for (ii = 0; ii < RT_TABLESIZE; ii++) {
						list_t *pos1;
						list_foreach(pos1, &rt_tbl.tbl[ii]) {
	    					rt_table_t *rt_u1 = (rt_table_t *) pos1;
							printf("当前路由表项 ip:%d, dest:%d next:%d valid:%d flags:%d\n",
									DEV_NR(0).ipaddr.s_addr,rt_u1->dest_addr.s_addr,rt_u1->next_hop.s_addr, rt_u1->state, rt_u1->flags);
						}
					}
				#endif
				u_int8_t rerr_flags = RERR_NODELETE | RERR_LOCAL_REPAIR;
				// 准备重发现
				if (rt_u->nprec) {

					if (!rerr) {
						rerr =rerr_create(rerr_flags, rt_u->dest_addr, rt_u->dest_seqno);
						//rerr mjw
						rerr->rerr_id = this_host.rerr_id++;
						rerr->rerr_origin_addr = this_host.devs[0].ipaddr.s_addr;
						if (rt_u->nprec == 1)
							rerr_unicast_dest =
							FIRST_PREC(rt_u->precursors)->neighbor;
					} else {
						rerr_add_udest(rerr, rt_u->dest_addr, rt_u->dest_seqno);

						if (rerr_unicast_dest.s_addr) {
							list_t *pos2;
							list_foreach(pos2, &rt_u->precursors) {
								precursor_t *pr = (precursor_t *) pos2;
								if (pr->neighbor.s_addr != rerr_unicast_dest.s_addr) {
									rerr_unicast_dest.s_addr = 0;
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	if (rerr) {

		rt_table_t *rt_u = rt_table_find(rerr_unicast_dest);

		if (rt_u && rerr->dest_count == 1 && rerr_unicast_dest.s_addr) {
			printf("rerr dest: %d\n", rerr_unicast_dest);
			aodv_socket_send((AODV_msg *) rerr,
					rerr_unicast_dest,
					RERR_CALC_SIZE(rerr), 1,
					&DEV_IFINDEX(rt_u->ifindex));
		}
		else if (rerr->dest_count > 0) {
			for (i = 0; i < MAX_NR_INTERFACES; i++) {
				struct in_addr dest;

				if (!DEV_NR(i).enabled)
					continue;
				dest.s_addr = AODV_BROADCAST;
				aodv_socket_send((AODV_msg *) rerr, dest,
						RERR_CALC_SIZE(rerr), 1, &DEV_NR(i));
			}
		}
    }

	if (pre_repair_flags & RT_REPAIR) {
	    /*if (fwd_rt->hcnt > pre_repair_hcnt) {
		RERR *rerr;
		u_int8_t rerr_flags = 0;
		struct in_addr dest;

		dest.s_addr = AODV_BROADCAST;

		rerr_flags |= RERR_NODELETE;
		rerr = rerr_create(rerr_flags, fwd_rt->dest_addr,
				   fwd_rt->dest_seqno);

		if (fwd_rt->nprec)
		    aodv_socket_send((AODV_msg *) rerr, dest,
				     RERR_CALC_SIZE(rerr), 1,
				     &DEV_IFINDEX(fwd_rt->ifindex));
	    }*/
	}
    } else {
	/* --- Here we FORWARD the RREP on the REVERSE route --- */
	if (rev_rt && rev_rt->state == VALID) {
	    rrep_forward(rrep, rreplen, rev_rt, fwd_rt, --ip_ttl);
	} else {
	    DEBUG(LOG_DEBUG, 0, "Could not forward RREP - NO ROUTE!!!");
	}
    }

    if (!llfeedback && optimized_hellos)
	hello_start();
}			   


void NS_CLASS rrep_process(RREP * rrep, int rreplen, struct in_addr ip_src,
			   struct in_addr ip_dst, int ip_ttl,
			   unsigned int ifindex)
{
	struct in_addr now_addr = this_host.devs[0].ipaddr;

    if(rrep->lr) {
    	printf("[%.9f RREP_LOCAL]now the address is: %d, source is : %d , target is: %d, ttl: %d, channelNum : %d, origin: %d, dest: %d\n", 
            Scheduler::instance().clock(), now_addr, ip_src, ip_dst, ip_ttl, rrep->channel, rrep->orig_addr, rrep->dest_addr);
		rrep_process_lr(rrep,rreplen,ip_src,ip_dst,ip_ttl,ifindex);
		return;
	}
    //struct in_addr now_addr = this_host.devs[0].ipaddr;
    printf("[%.9f RREP_NORMAL]now the address is: %d, source is : %d , target is: %d, ttl: %d, channelNum : %d, origin: %d, dest: %d\n", 
            Scheduler::instance().clock(), now_addr, ip_src, ip_dst, ip_ttl, rrep->channel, rrep->orig_addr, rrep->dest_addr);

    u_int32_t rrep_lifetime, rrep_seqno, rrep_new_hcnt;
    u_int8_t pre_repair_hcnt = 0, pre_repair_flags = 0;
    rt_table_t *fwd_rt, *rev_rt;
    AODV_ext *ext;
    unsigned int extlen = 0;
    int rt_flags = 0;

    /* modified by chenjiyuan 11.30 */
    int channel = rrep->channel; //todo: set channel
    //printf("set channel in rrep process : %d\n", channel);
    /* end modified at 11.30 */

    struct in_addr rrep_dest, rrep_orig;
#ifdef CONFIG_GATEWAY
    struct in_addr inet_dest_addr;
    int inet_rrep = 0;
#endif

    /* Convert to correct byte order on affeected fields: */
    rrep_dest.s_addr = rrep->dest_addr;
    rrep_orig.s_addr = rrep->orig_addr;
    rrep_seqno = ntohl(rrep->dest_seqno);
    rrep_lifetime = ntohl(rrep->lifetime);
    /* Increment RREP hop count to account for intermediate node... */
    rrep_new_hcnt = rrep->hcnt + 1;

    if (rreplen < (int) RREP_SIZE) {
	alog(LOG_WARNING, 0, __FUNCTION__,
	     "IP data field too short (%u bytes)"
	     " from %s to %s", rreplen, ip_to_str(ip_src), ip_to_str(ip_dst));
	return;
    }

    /* Ignore messages which aim to a create a route to one self */
    if (rrep_dest.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr) {
	    printf("[RREP-ABORT]\n");
        return;
    }

    DEBUG(LOG_DEBUG, 0, "from %s about %s->%s",
	  ip_to_str(ip_src), ip_to_str(rrep_orig), ip_to_str(rrep_dest));
#ifdef DEBUG_OUTPUT
    log_pkt_fields((AODV_msg *) rrep);
#endif
	/*@ modify by chenjiyuan */
	double cost = log(MAX_NUM);
	/*@ end modify*/

    /* Determine whether there are any extensions */
    ext = (AODV_ext *) ((char *) rrep + RREP_SIZE);

    while ((rreplen - extlen) > RREP_SIZE) {
	switch (ext->type) {
		
	/*@ modify by chenjiyuan at 11.30 */
	case RREP_COST_EXT:
	    //DEBUG(LOG_INFO, 0, "RREP include EXTENSION");
	    cost = *(double*)((char *) AODV_EXT_DATA(ext)) + getCost(ip_src, now_addr, channel);
        *(double*)((char *) AODV_EXT_DATA(ext)) = cost;
        printf("[RREP] get cost %lf\n", cost);
        break;
	/*@ end modify at 11.30 */

	case RREP_EXT:
	    DEBUG(LOG_INFO, 0, "RREP include EXTENSION");
	    /* Do something here */
	    break;
#ifdef CONFIG_GATEWAY
	case RREP_INET_DEST_EXT:
	    if (ext->length == sizeof(u_int32_t)) {

		/* Destination address in RREP is the gateway address, while the
		 * extension holds the real destination */
		memcpy(&inet_dest_addr, AODV_EXT_DATA(ext), ext->length);

		DEBUG(LOG_DEBUG, 0, "RREP_INET_DEST_EXT: <%s>",
		      ip_to_str(inet_dest_addr));
		/* This was a RREP from a gateway */
		rt_flags |= RT_GATEWAY;
		inet_rrep = 1;
		break;
	    }
#endif
	default:
	    alog(LOG_WARNING, 0, __FUNCTION__, "Unknown or bad extension %d",
		 ext->type);
	    break;
	}
	extlen += AODV_EXT_SIZE(ext);
	ext = AODV_EXT_NEXT(ext);
    }
        printf("[RREP] get cost %lf\n", cost);
    /* ---------- CHECK IF WE SHOULD MAKE A FORWARD ROUTE ------------ */

    /*@ modify by chenjiyuan at 11.30 */
    fwd_rt = rt_table_find(rrep_dest);
    rev_rt = rt_table_find(rrep_orig);
    //fwd_rt = rt_table_find(rrep_dest);
    //rev_rt = rt_table_find(rrep_orig);
    // printf("[RREP]find the fwd (%d) & rev (%d)?\n", fwd_rt, rev_rt);
    // if (rev_rt) {
    //     printf("[ROUNTING_TABLE] the rev (%d) of dest is %d, next hop is %d?\n", rev_rt, rev_rt -> dest_addr, rev_rt -> next_hop);
    // }
    /* end modify at 11.30 */

    if (!fwd_rt) {
	/* We didn't have an existing entry, so we insert a new one. */
    /*@ modify by chenjiyuan at 11.30 */
        /*fwd_rt = rt_table_insert(rrep_dest, ip_src, rrep_new_hcnt, rrep_seqno,
				 rrep_lifetime, VALID, rt_flags, ifindex);*/
        fwd_rt = rt_table_insert_with_channel(rrep_dest, ip_src, rrep_new_hcnt, rrep_seqno,
                                 rrep_lifetime, VALID, rt_flags, ifindex, channel, cost);
    /* end modify at 11.30 */
    } else if (fwd_rt->dest_seqno == 0 ||
	       (int32_t) rrep_seqno > (int32_t) fwd_rt->dest_seqno ||
	       (rrep_seqno == fwd_rt->dest_seqno &&
		(fwd_rt->state == INVALID || fwd_rt->flags & RT_UNIDIR ||
		 cost < fwd_rt->cost))) {
            printf("only for test: dest_seqno: %d, rrep_seqno: %d, state: %d, cost: %lf\n", fwd_rt->dest_seqno, rrep_seqno, fwd_rt->state, cost);
			printf("channel: %d, cost: %lf", channel, cost);
	pre_repair_hcnt = fwd_rt->hcnt;
	pre_repair_flags = fwd_rt->flags;

    /*@ modify by chenjiyuan at 11.30 */
    /*
    fwd_rt = rt_table_update(fwd_rt, ip_src, rrep_new_hcnt, rrep_seqno,
				 rrep_lifetime, VALID,
				 rt_flags | fwd_rt->flags); */
    fwd_rt = rt_table_update_with_channel(fwd_rt, ip_src, rrep_new_hcnt, rrep_seqno,
                             rrep_lifetime, VALID,
                             rt_flags | fwd_rt->flags, channel, cost);
    } else {
	if (fwd_rt->hcnt > 1) {
	    DEBUG(LOG_DEBUG, 0,
		  "Dropping RREP, fwd_rt->hcnt=%d fwd_rt->seqno=%ld",
		  fwd_rt->hcnt, fwd_rt->dest_seqno);
	}
	return;
    }


    /* If the RREP_ACK flag is set we must send a RREP
       acknowledgement to the destination that replied... */
    if (rrep->a) {
	RREP_ack *rrep_ack;

	rrep_ack = rrep_ack_create();
	aodv_socket_send((AODV_msg *) rrep_ack, fwd_rt->next_hop,
			 NEXT_HOP_WAIT, MAXTTL, &DEV_IFINDEX(fwd_rt->ifindex));
	/* Remove RREP_ACK flag... */
	rrep->a = 0;
    }

    /* Check if this RREP was for us (i.e. we previously made a RREQ
       for this host). */
    if (rrep_orig.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr) {
#ifdef CONFIG_GATEWAY
	if (inet_rrep) {
	    rt_table_t *inet_rt;
	    inet_rt = rt_table_find(inet_dest_addr);

	    /* Add a "fake" route indicating that this is an Internet
	     * destination, thus should be encapsulated and routed through a
	     * gateway... */
	    if (!inet_rt)
		rt_table_insert(inet_dest_addr, rrep_dest, rrep_new_hcnt, 0,
				rrep_lifetime, VALID, RT_INET_DEST, ifindex);
	    else if (inet_rt->state == INVALID || rrep_new_hcnt < inet_rt->hcnt) {
		rt_table_update(inet_rt, rrep_dest, rrep_new_hcnt, 0,
				rrep_lifetime, VALID, RT_INET_DEST |
				inet_rt->flags);
	    } else {
		DEBUG(LOG_DEBUG, 0, "INET Response, but no update %s",
		      ip_to_str(inet_dest_addr));
	    }
	}
#endif				/* CONFIG_GATEWAY */

	/* If the route was previously in repair, a NO DELETE RERR should be
	   sent to the source of the route, so that it may choose to reinitiate
	   route discovery for the destination. Fixed a bug here that caused the
	   repair flag to be unset and the RERR never being sent. Thanks to
	   McWood <hjw_5@hotmail.com> for discovering this. */
	if (pre_repair_flags & RT_REPAIR) {
	    if (fwd_rt->hcnt > pre_repair_hcnt) {
		RERR *rerr;
		u_int8_t rerr_flags = 0;
		struct in_addr dest;

		dest.s_addr = AODV_BROADCAST;

		rerr_flags |= RERR_NODELETE;
		rerr = rerr_create(rerr_flags, fwd_rt->dest_addr,
				   fwd_rt->dest_seqno);

		if (fwd_rt->nprec)
		    aodv_socket_send((AODV_msg *) rerr, dest,
				     RERR_CALC_SIZE(rerr), 1,
				     &DEV_IFINDEX(fwd_rt->ifindex));
	    }
	}
    } else {
	/* --- Here we FORWARD the RREP on the REVERSE route --- */
	if (rev_rt && rev_rt->state == VALID) {
        /*@ modify by chenjiyuan at 11.30 */
        //rrep_forward(rrep, rreplen, rev_rt, fwd_rt, --ip_ttl);
        channelNum = rev_rt->channel;
		printf("[RREP-FORWARD]cost: %lf, channel: %d, target: %d\n", rev_rt->cost, rev_rt->channel, rev_rt->dest_addr);
        rrep_forward(rrep, RREP_COST_SIZE, rev_rt, fwd_rt, --ip_ttl);
        /* end modified at 11.30 */
	} else {
	    DEBUG(LOG_DEBUG, 0, "Could not forward RREP - NO ROUTE!!!");
	}
    }

    if (!llfeedback && optimized_hellos)
	hello_start();
}

/************************************************************************/

/* Include a Hello Interval Extension on the RREP and return new offset */

int rrep_add_hello_ext(RREP * rrep, int offset, u_int32_t interval)
{
    AODV_ext *ext;

    ext = (AODV_ext *) ((char *) rrep + RREP_SIZE + offset);
    ext->type = RREP_HELLO_INTERVAL_EXT;
    ext->length = sizeof(interval);
    memcpy(AODV_EXT_DATA(ext), &interval, sizeof(interval));

    return (offset + AODV_EXT_SIZE(ext));
}
