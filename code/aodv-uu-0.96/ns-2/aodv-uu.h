/*****************************************************************************
 *
 * Copyright (C) 2002 Uppsala University.
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
 * Authors: Bj�rn Wiberg <bjorn.wiberg@home.se>
 *
 *****************************************************************************/

#ifndef _AODV_UU_H
#define _AODV_UU_H

/* Number of channel */
#define Channel_Count 3

/* Constants for interface queue packet buffering/dropping */
#define IFQ_BUFFER 0
#define IFQ_DROP 1
#define IFQ_DROP_BY_DEST 2

/* This is a C++ port of AODV-UU for ns-2 */
#ifndef NS_PORT
#error "To compile the ported version, NS_PORT must be defined!"
#endif /* NS_PORT */


/* System-dependent datatypes */
/* Needed by some network-related datatypes */
#include <sys/types.h>

/* Network-related datatypes */
/* Needed by <netinet/ip.h> on Sun Solaris */
#include <netinet/in_systm.h>

/* Header files from ns-2 */
#include "../../common/packet.h"
#include "./../common/timer-handler.h"
#include "./../tools/random.h"
#include "./../trace/cmu-trace.h"
#include "./../queue/priqueue.h"
/* Added by buaa g410 */
#include "../../common/agent.h"
#include "../../config.h"
#include "../../mac/mac-802_11.h"
/* End buaa g410 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Forward declaration needed to be able to reference the class */
class AODVUU;

/* Global definitions and lib functions */
#include "../params.h"
#include "../defs.h"
#include "../list.h"

/* Extract global data types, defines and global declarations */
#undef NS_NO_GLOBALS
#define NS_NO_DECLARATIONS

#include "../timer_queue.h"
#include "../aodv_hello.h"
#include "../aodv_rerr.h"
#include "../aodv_rrep.h"
#include "../aodv_rreq.h"
#include "../aodv_socket.h"
#include "../aodv_timeout.h"
#include "../debug.h"
#include "../routing_table.h"
#include "../seek_list.h"
#include "../locality.h"

#include "packet_queue.h"
#include "packet_input.h"

#undef NS_NO_DECLARATIONS

/* In ns-2 we don't care about byte order */
#undef ntohl
#undef htonl
#undef htons
#undef ntohs

#define ntohl(x) x
#define htonl(x) x
#define htons(x) x
#define ntohs(x) x

#define MAX_IF 11
/* Timer class for managing the queue of timers */
class TimerQueueTimer : public TimerHandler {
public:
	TimerQueueTimer(AODVUU *a) : TimerHandler() { agent_ = a; }
	virtual void expire(Event *e);

protected:
	AODVUU *agent_;
};


/* The AODV-UU routing agent class */
class AODVUU : public Agent {

	friend class TimerQueueTimer;

	unsigned int ifindex; /* Always use ns interface */
public:
	AODVUU(nsaddr_t id);
	~AODVUU();

	void recv(Packet *p, Handler *);
	int command(int argc, const char *const *argv);
	void packetFailed(Packet *p);

protected:
	void interfaceQueue(nsaddr_t next_hop, int action);
	void sendPacket(Packet *p, struct in_addr next_hop, double delay);
	int startAODVUUAgent();
	void scheduleNextEvent();
	int gettimeofday(struct timeval *tv, struct timezone *tz);
	char *if_indextoname(int ifindex, char *ifname);
	Packet *pkt_encapsulate(Packet *p, struct in_addr gw_addr);
	Packet *pkt_decapsulate(Packet *p);

	int initialized;
	TimerQueueTimer tqtimer;
	PriQueue *ifqueue;
	NsObject *ll; /* Pointer to link layer */
	int  node_id;
	MobileNode *node_;
	NsObject *port_dmux_;
	NsObject *wired_target_;

	/* Added by buaa g410 */
	int nIfaces;
	NsObject *lllist[MAX_IF];
	PriQueue *ifqueuelist[MAX_IF];
	Mac802_11 *maclist[MAX_IF];
	int fixed_interface;
	/* End buaa g410 */
	/* Added by MSQ */
	int channelNum;
	/* End MSQ */
/*
  Extract method declarations (and occasionally, variables)
  from header files
*/
#define NS_NO_GLOBALS
#undef NS_NO_DECLARATIONS

#undef _AODV_NEIGHBOR_H
#include "../aodv_neighbor.h"

#undef _AODV_HELLO_H
#include "../aodv_hello.h"

#undef _AODV_RERR_H
#include "../aodv_rerr.h"

#undef _AODV_RREP_H
#include "../aodv_rrep.h"

#undef _AODV_RREQ_H
#include "../aodv_rreq.h"

#undef _AODV_SOCKET_H
#include "../aodv_socket.h"

#undef _AODV_TIMEOUT_H
#include "../aodv_timeout.h"

#undef _DEBUG_H
#include "../debug.h"

#undef _ROUTING_TABLE_H
#include "../routing_table.h"

#undef _SEEK_LIST_H
#include "../seek_list.h"

#undef _TIMER_QUEUE_H
#include "../timer_queue.h"

#undef _LOCALITY_H
#include "../locality.h"

#undef _PACKET_INPUT_H
#include "packet_input.h"

#undef _PACKET_QUEUE_H
#include "packet_queue.h"

#undef NS_NO_GLOBALS

	/* (Previously global) variables from main.c */
	int log_to_file;
	int rt_log_interval;
	int unidir_hack;
	int rreq_gratuitous;
	int expanding_ring_search;
	int internet_gw_mode;
	int local_repair;
	int receive_n_hellos;
	int hello_jittering;
	int optimized_hellos;
	int ratelimit;
	int llfeedback;
	char *progname;
	int wait_on_reboot;
	struct timer worb_timer;

	/* Parameters that are dynamic configuration values: */
	int active_route_timeout;
	int ttl_start;
	int delete_period;

	/* From aodv_hello.c */
	struct timer hello_timer;

	/* From aodv_rreq.c */
	list_t rreq_records;
	list_t rreq_blacklist;

	/* From seek_list.c */
	list_t seekhead;

	/* From aodv_socket.c */
	char recv_buf[RECV_BUF_SIZE];
	char send_buf[SEND_BUF_SIZE];

	/* From debug.c */
	int log_file_fd;
	int log_rt_fd;
	int log_nmsgs;
	int debug;
	struct timer rt_log_timer;

	/* From defs.h */
	struct host_info this_host;
	unsigned int dev_indices[MAX_NR_INTERFACES];
	inline int ifindex2devindex(unsigned int ifindex);
	inline double getCost(struct in_addr src_addr, struct in_addr dest_addr, int channel);

/* From timer_queue.c */
	list_t TQ;


};


/* From defs.h (needs the AODVUU class declaration) */
inline int NS_CLASS ifindex2devindex(unsigned int ifindex)
{
	int i;

	for (i = 0; i < this_host.nif; i++)
		if (dev_indices[i] == ifindex)
			return i;

	return -1;
}

/*@ modified by chenjiyuan:11.23*/
inline double NS_CLASS getCost(struct in_addr src_addr, struct in_addr dest_addr, int channel) {
	// printf("get in abcdefg!!\n");
	printf("this host is %d, wanna find cost to dest: %d through channel:%d\n",this_host.devs[0].ipaddr.s_addr, src_addr.s_addr, channel);
	int src = src_addr.s_addr;
	int dst = dest_addr.s_addr;

	printf("[FROM %d TO %d]\n", src_addr.s_addr, dest_addr.s_addr);
// #if 0
    //if(0)
    {//modified by XY
        int index=0;
        for(index = 0;index < this_host.neighbor_num; ++index)
        {
            if(this_host.neighbors[index].ipaddr.s_addr == src){
                return min(100000.0, -log(max(0.000001, this_host.neighbors[index].channel_cost[channel])));
            }else{
				printf("has neighbor %d in this host, his cost through channel is %f\n",this_host.neighbors[index].ipaddr.s_addr, this_host.neighbors[index].channel_cost[channel]);
			}
        }
		printf("error! can't find dest!\n");
		return 100000.0;
    }
// #endif

	//return 1.0;
}
/* end modified*/

#endif /* AODV_UU_H */
