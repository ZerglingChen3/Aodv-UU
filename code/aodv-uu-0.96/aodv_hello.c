/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University and Ericsson AB.
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
#else
#include <netinet/in.h>
#include "aodv_hello.h"
#include "aodv_timeout.h"
#include "aodv_rrep.h"
#include "aodv_rreq.h"
#include "routing_table.h"
#include "timer_queue.h"
#include "params.h"
#include "aodv_socket.h"
#include "defs.h"
#include "debug.h"

extern int unidir_hack, receive_n_hellos, hello_jittering, optimized_hellos;
static struct timer hello_timer;

#endif

/* #define DEBUG_HELLO */


long NS_CLASS hello_jitter()
{
    if (hello_jittering) {
#ifdef NS_PORT
	return (long) (((float) Random::integer(RAND_MAX + 1) / RAND_MAX - 0.5)
		       * JITTER_INTERVAL);
#else
	return (long) (((float) random() / RAND_MAX - 0.5) * JITTER_INTERVAL);
#endif
    } else
	return 0;
}

void NS_CLASS hello_start()
{
    if (hello_timer.used)
	return;

    gettimeofday(&this_host.fwd_time, NULL);

    DEBUG(LOG_DEBUG, 0, "Starting to send HELLOs!");
    timer_init(&hello_timer, &NS_CLASS hello_send, NULL);

    hello_send(NULL);
}

void NS_CLASS hello_stop()
{
    DEBUG(LOG_DEBUG, 0,
	  "No active forwarding routes - stopped sending HELLOs!");
    timer_remove(&hello_timer);
}

void NS_CLASS hello_send(void *arg)
{	
	printf("[HELLO_SEND_IN]\n");
    RREP *rrep;
    AODV_ext *ext = NULL;
    u_int8_t flags = 0;
    struct in_addr dest;
    long time_diff, jitter;
    struct timeval now;
    int msg_size = RREP_SIZE;
    int i;

    gettimeofday(&now, NULL);

    if (optimized_hellos &&
	timeval_diff(&now, &this_host.fwd_time) > ACTIVE_ROUTE_TIMEOUT) {
	printf("[HELLO_SEND_OUT]\n");
	hello_stop();
	return;
    }

    time_diff = timeval_diff(&now, &this_host.bcast_time);
    jitter = hello_jitter();

    /* This check will ensure we don't send unnecessary hello msgs, in case
       we have sent other bcast msgs within HELLO_INTERVAL */
    if (time_diff >= HELLO_INTERVAL) {
	for (i = 0; i < MAX_NR_INTERFACES; i++) {
	    if (!DEV_NR(i).enabled)
		continue;
#ifdef DEBUG_HELLO
	    DEBUG(LOG_DEBUG, 0, "sending Hello to 255.255.255.255");
#endif
	    rrep = rrep_create(flags, 0, 0, DEV_NR(i).ipaddr,
			       this_host.seqno,
			       DEV_NR(i).ipaddr,
			       ALLOWED_HELLO_LOSS * HELLO_INTERVAL);
//		if (0) //switch to use this feature or not
			{ //modified by XY
                // update stability sequence
				this_host.hello_sent++;
                this_host.stability_sequence[this_host.hello_sent % MAX_SEQUENCE_LEN]=this_host.stability.isStable;

                // move head and tail pointer
                int head=this_host.hello_head;
                int tail=this_host.hello_tail;
                if (head % MAX_SEQUENCE_LEN == (tail + 1) % MAX_SEQUENCE_LEN)
                {
                    this_host.hello_head++;
                    head++;
                }
                this_host.hello_tail++;
                tail++;

                // update every channel cost
				printf("------------info update begin\n");
				for(int neib_index=0;neib_index<20;++neib_index){
					if(this_host.neighbors[neib_index].ipaddr.s_addr==0)
						break;
					for (int channel = 0; channel < 5; ++channel)
					{
						printf("neib_index:%d\n",neib_index);
						if (head % MAX_SEQUENCE_LEN == (tail + 1) % MAX_SEQUENCE_LEN)
						{
							double deliver_rate = 0;
							double expect_connected_possibility = 0;
							double status_change_possibility = 0;
							{ // deliver rate
								printf("caculating deliver rate\n");
								printf("this_host.hello_sent:%d\n",this_host.hello_sent);
                                double self_sent=this_host.hello_sent;
                                double self_received=this_host.neighbors[neib_index].channel_hello_self_received[channel];
                                double remote_sent=this_host.neighbors[neib_index].channel_hello_remote_sent;
                                double remote_received=this_host.neighbors[neib_index].channel_hello_remote_received[channel];
								double positive_rate = remote_received / self_sent;
								double negative_rate = self_received / remote_sent;
                                deliver_rate = positive_rate * negative_rate;
							}
							{ // arma model
								// use time interval every two hello msg
								//todo: get p, q from model
								1;
							}
							{ //
                                int stable_count = 0;
                                for(int now_index = head; now_index < tail; ++now_index)
								{
                                    stable_count += this_host.stability_sequence[now_index] && this_host.neighbors[neib_index].host_stability_sequence[now_index];
								}
								status_change_possibility = 1.0 * stable_count / MAX_SEQUENCE_LEN;
							}
							// use status change possibility temporarily
							this_host.neighbors[neib_index].channel_cost[channel] = 0.33 * deliver_rate + 0.66 * status_change_possibility;
						}
						else
						{
							printf("before reaching 10\n");
							int success_count = 0;
							int count = 0;
                            for(int now_index = head; now_index < tail; ++now_index)
							{
								success_count += this_host.neighbors[neib_index].channel_hello_sequence[channel][now_index % MAX_SEQUENCE_LEN];
								count++;
							}
							printf("count:%d\n",count);
							this_host.neighbors[neib_index].channel_cost[channel] = success_count / count;
						}
					}
				}
				printf("---------info update end\n");
			}
	    /* Assemble a RREP extension which contain our neighbor set... */
	    if (unidir_hack) {
		int i;

		if (ext)
		    ext = AODV_EXT_NEXT(ext);
		else
		    ext = (AODV_ext *) ((char *) rrep + RREP_SIZE);

		ext->type = RREP_HELLO_NEIGHBOR_SET_EXT;
		ext->length = 0;

		for (i = 0; i < RT_TABLESIZE; i++) {
		    list_t *pos;
		    list_foreach(pos, &rt_tbl.tbl[i]) {
			rt_table_t *rt = (rt_table_t *) pos;
			/* If an entry has an active hello timer, we assume
			   that we are receiving hello messages from that
			   node... */
			if (rt->hello_timer.used) {
#ifdef DEBUG_HELLO
			    DEBUG(LOG_INFO, 0,
				  "Adding %s to hello neighbor set ext",
				  ip_to_str(rt->dest_addr));
#endif
			    memcpy(AODV_EXT_DATA(ext), &rt->dest_addr,
				   sizeof(struct in_addr));
			    ext->length += sizeof(struct in_addr);
			}
		    }
		}
		if (ext->length)
		    msg_size = RREP_SIZE + AODV_EXT_SIZE(ext);
	    }
	    dest.s_addr = AODV_BROADCAST;
		/* Added by MSQ */
		update_sta_info();
		update_neighbor_info();
		rrep->res1 = this_host.stability.isStable;
		/* End MSQ */
        //if(0)
        {//modified by XY
            rrep->hello_sent = this_host.hello_sent;
        }
	    aodv_socket_send((AODV_msg *) rrep, dest, msg_size, 1, &DEV_NR(i));
	}

	timer_set_timeout(&hello_timer, HELLO_INTERVAL + jitter);
    } else {
	if (HELLO_INTERVAL - time_diff + jitter < 0)
	    timer_set_timeout(&hello_timer,
			      HELLO_INTERVAL - time_diff - jitter);
	else
	    timer_set_timeout(&hello_timer,
			      HELLO_INTERVAL - time_diff + jitter);
    }

	printf("[HELLO_SEND_OUT]\n");
}


/* Process a hello message */
void NS_CLASS hello_process(RREP * hello, int rreplen, unsigned int ifindex)
{

	struct in_addr now_addr = this_host.devs[0].ipaddr;
    printf("[%.9f HELLO]now the address is: %d\n",Scheduler::instance().clock(), now_addr);

	printf("[HELLO_PROCESS]\n");
    u_int32_t hello_seqno, timeout, hello_interval = HELLO_INTERVAL;
    u_int8_t state, flags = 0;
    struct in_addr ext_neighbor, hello_dest;
    rt_table_t *rt;
    AODV_ext *ext = NULL;
    int i;
    struct timeval now;

    gettimeofday(&now, NULL);

    hello_dest.s_addr = hello->dest_addr;
    hello_seqno = ntohl(hello->dest_seqno);

//	if (0) //switch to use this feature or not
	{ // modified by XY
		int neib_index=0;
		for(neib_index=0;neib_index<20;++neib_index){
			if(this_host.neighbors[neib_index].ipaddr.s_addr == hello_dest.s_addr){
				// printf("~~~~~~~~~~~find record in the neighbor table, the ip is:%d~~~~~~~~~~~~~~~~\n", hello_dest.s_addr);
				break;
			}else if(this_host.neighbors[neib_index].ipaddr.s_addr==0){
				// printf("~~~~~~~~~~~~~didt find ip, create a new record, the ip is:%d~~~~~~~~~~~~~~\n", hello_dest.s_addr);
				this_host.neighbors[neib_index].ipaddr.s_addr=hello_dest.s_addr;
				break;
			}
		}
		
		printf("[HELLO_XY] from: %d channel:%d\n",hello->dest_addr, hello->channel);
		int channel = hello->channel;

		this_host.neighbors[neib_index].channel_hello_self_received[channel]++;
		this_host.neighbors[neib_index].channel_hello_sequence[channel][this_host.hello_tail] = 1;
        this_host.neighbors[neib_index].channel_hello_remote_sent=hello->hello_sent;

        // send hello_ack
		RREP_ack *hello_ack;
		hello_ack = rrep_ack_create();
		hello_ack->is_hello_ack = 1;
		hello_ack->channel = hello->channel;
        hello_ack->hello_index = hello->hello_sent;
		hello_ack->channel_hello_received = 
            this_host.neighbors[neib_index].channel_hello_self_received[channel];
		aodv_socket_send((AODV_msg *)hello_ack, hello_dest, RREP_ACK_SIZE, MAXTTL, &DEV_IFINDEX(ifindex));
		printf("send hello_ack:%x to dest:%d, through channel:%d\n", hello_ack, hello_dest, hello->channel);
		printf("[HELLO_XY_END]\n");
	}

	//printf("the hello comes from channel: %d\n", hello->channel);
	/* Added by MSQ */
	int flag = 0;
	for (i = 0; i < 20; i++) {
		if (this_host.neighbors[i].ipaddr.s_addr == hello_dest.s_addr) {
			//this_host.neighbors[i].isValid = 0;
			this_host.neighbors[i].neighbor_sta = hello->res1;
			flag = 1;
			break;
		}
	}
	if (flag == 0) {
		for (i = 0; i < 20; i++) {
			if (this_host.neighbors[i].ipaddr.s_addr == 0) {
				this_host.neighbors[i].ipaddr.s_addr = hello_dest.s_addr;
				//this_host.neighbors[i].isValid = 0;
				this_host.neighbors[i].neighbor_sta = hello->res1;
				break;
			}
		}	
	}
	/* End MSQ */

    rt = rt_table_find(hello_dest); //todo: set channel

    if (rt)
	flags = rt->flags;

    if (unidir_hack)
	flags |= RT_UNIDIR;

    /* Check for hello interval extension: */
    ext = (AODV_ext *) ((char *) hello + RREP_SIZE);

    while (rreplen > (int) RREP_SIZE) {
	switch (ext->type) {
	case RREP_HELLO_INTERVAL_EXT:
	    if (ext->length == 4) {
		memcpy(&hello_interval, AODV_EXT_DATA(ext), 4);
		hello_interval = ntohl(hello_interval);
#ifdef DEBUG_HELLO
		DEBUG(LOG_INFO, 0, "Hello extension interval=%lu!",
		      hello_interval);
#endif

	    } else
		alog(LOG_WARNING, 0,
		     __FUNCTION__, "Bad hello interval extension!");
	    break;
	case RREP_HELLO_NEIGHBOR_SET_EXT:

#ifdef DEBUG_HELLO
	    DEBUG(LOG_INFO, 0, "RREP_HELLO_NEIGHBOR_SET_EXT");
#endif
	    for (i = 0; i < ext->length; i = i + 4) {
		ext_neighbor.s_addr =
		    *(in_addr_t *) ((char *) AODV_EXT_DATA(ext) + i);

		if (ext_neighbor.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr)
		    flags &= ~RT_UNIDIR;
	    }
	    break;
	default:
	    alog(LOG_WARNING, 0, __FUNCTION__,
		 "Bad extension!! type=%d, length=%d", ext->type, ext->length);
	    ext = NULL;
	    break;
	}
	if (ext == NULL)
	    break;

	rreplen -= AODV_EXT_SIZE(ext);
	ext = AODV_EXT_NEXT(ext);
    }

#ifdef DEBUG_HELLO
    DEBUG(LOG_DEBUG, 0, "rcvd HELLO from %s, seqno %lu",
	  ip_to_str(hello_dest), hello_seqno);
#endif
    /* This neighbor should only be valid after receiving 3
       consecutive hello messages... */
    if (receive_n_hellos)
	state = INVALID;
    else
	state = VALID;

    timeout = ALLOWED_HELLO_LOSS * hello_interval + ROUTE_TIMEOUT_SLACK;

    if (!rt) {
	/* No active or expired route in the routing table. So we add a
	   new entry... */

	rt = rt_table_insert(hello_dest, hello_dest, 1,
			     hello_seqno, timeout, state, flags, ifindex); //todo: set channel

	if (flags & RT_UNIDIR) {
	    DEBUG(LOG_INFO, 0, "%s new NEIGHBOR, link UNI-DIR",
		  ip_to_str(rt->dest_addr));
	} else {
	    DEBUG(LOG_INFO, 0, "%s new NEIGHBOR!", ip_to_str(rt->dest_addr));
	}
	rt->hello_cnt = 1;

    } else {

	if ((flags & RT_UNIDIR) && rt->state == VALID && rt->hcnt > 1) {
	    goto hello_update;
	}

	if (receive_n_hellos && rt->hello_cnt < (receive_n_hellos - 1)) {
	    if (timeval_diff(&now, &rt->last_hello_time) <
		(long) (hello_interval + hello_interval / 2))
		rt->hello_cnt++;
	    else
		rt->hello_cnt = 1;

	    memcpy(&rt->last_hello_time, &now, sizeof(struct timeval));
	    return;
	}
	rt_table_update(rt, hello_dest, 1, hello_seqno, timeout, VALID, flags); //todo: set channel
    }

  hello_update:

    hello_update_timeout(rt, &now, ALLOWED_HELLO_LOSS * hello_interval);
    return;
}


#define HELLO_DELAY 50		/* The extra time we should allow an hello
				   message to take (due to processing) before
				   assuming lost . */

NS_INLINE void NS_CLASS hello_update_timeout(rt_table_t * rt,
					     struct timeval *now, long time)
{
    timer_set_timeout(&rt->hello_timer, time + HELLO_DELAY);
    memcpy(&rt->last_hello_time, now, sizeof(struct timeval));
}

/* Added by MSQ */
void NS_CLASS update_sta_info()
{
	int i, j;
	int flag;
	int best_SNR;
	this_host.stability.neighbor_num = 0;
	this_host.stability.neighbor_change = 0;
	this_host.stability.ava_channel_num = 0;
	this_host.stability.best_channel_SNR = 0;
	for (i = 0; i < RT_TABLESIZE; i++) {
		list_t *pos;
		list_foreach(pos, &rt_tbl.tbl[i]) {
			rt_table_t *rt = (rt_table_t *) pos;
			/* If an entry has an active hello timer, we assume
				that we are receiving hello messages from that
				node... */
			if (rt->hello_timer.used) {
				this_host.stability.neighbor_num++;
				flag = 0;
				for (j = 0; j < 20; j++) {
					if (this_host.neighbors[j].ipaddr.s_addr == rt->dest_addr.s_addr
						&& this_host.neighbors[j].isValid == 1) {
						flag = 1;	
						break;
					}
				}
				if (flag == 0) {
					this_host.stability.neighbor_change++;
				}
			}
		}
	}
	for (j = 0; j < 20; j++) {
		if (this_host.neighbors[j].isValid == 0 || this_host.neighbors[j].ipaddr.s_addr == 0) {
			continue;
		}
		flag = 0;
		for (i = 0; i < RT_TABLESIZE; i++) {
			list_t *pos;
			list_foreach(pos, &rt_tbl.tbl[i]) {
				rt_table_t *rt = (rt_table_t *) pos;
				if (rt->hello_timer.used) {
					if (rt->dest_addr.s_addr == this_host.neighbors[j].ipaddr.s_addr) {
						flag = 1;
						break;
					}
				}
			}
		}
		if (flag == 0) {
			this_host.stability.neighbor_change++;
		}
	}
	for (i = 0; i < 3; i++) {
		if (maclist[i]->getState() == 1) {
			this_host.stability.ava_channel_num++;
		}
		if (maclist[i]->getSNR() > this_host.stability.best_channel_SNR) {
			this_host.stability.best_channel_SNR = maclist[i]->getSNR();
		}
	}
	//printf("%d %d %d %f\n", this_host.stability.neighbor_num, this_host.stability.neighbor_change, this_host.stability.ava_channel_num, this_host.stability.best_channel_SNR);
	double svmParams[5] = {0.76960165, -0.39568155, 1.98846181, 4.02854831, -13.08570038};
	double svmResult = this_host.stability.neighbor_num * svmParams[0] + this_host.stability.neighbor_change * svmParams[1] + 
						this_host.stability.ava_channel_num * svmParams[2] + this_host.stability.best_channel_SNR * svmParams[3] + svmParams[4];
	if (svmResult < 0 || this_host.stability.neighbor_num == 0 || this_host.stability.ava_channel_num == 0) {
		this_host.stability.isStable = 0;
	}
	else {
		this_host.stability.isStable = 1;
	}
	//printf("%d %d %d %f %d\n", this_host.stability.neighbor_num, this_host.stability.neighbor_change, this_host.stability.ava_channel_num, this_host.stability.best_channel_SNR, this_host.stability.isStable);
	
}

void NS_CLASS update_neighbor_info()
{
	int i, j;
	int flag;
	for (i = 0; i < RT_TABLESIZE; i++) {
		list_t *pos;
		list_foreach(pos, &rt_tbl.tbl[i]) {
			rt_table_t *rt = (rt_table_t *) pos;
			if (rt->hello_timer.used) {
				flag = 0;
				for (j = 0; j < 20; j++) {
					if (rt->dest_addr.s_addr == this_host.neighbors[j].ipaddr.s_addr) {
						this_host.neighbors[j].isValid = 1;
						flag = 1;
						break;
					}
				}
				if (flag == 0) {
					for (j = 0; j < 20; j++) {
						if (this_host.neighbors[j].ipaddr.s_addr == 0) {
							this_host.neighbors[j].ipaddr.s_addr = rt->dest_addr.s_addr;
							this_host.neighbors[j].neighbor_sta = 0;
							this_host.neighbors[j].isValid = 1;
							break;
						}
					}
				}
			}
		}
	}
	for (j = 0; j < 20; j++) {
		if (this_host.neighbors[j].isValid == 0) {
			continue;
		}
		flag = 0;
		for (i = 0; i < RT_TABLESIZE; i++) {
			list_t *pos;
			list_foreach(pos, &rt_tbl.tbl[i]) {
				rt_table_t *rt = (rt_table_t *) pos;
				if (rt->hello_timer.used) {
					if (rt->dest_addr.s_addr == this_host.neighbors[j].ipaddr.s_addr) {
						flag = 1;
						break;
					}
				}
			}
		}
		if (flag == 0) {
			this_host.neighbors[j].isValid = 0;
		}
	}

}
/* End MSQ */
