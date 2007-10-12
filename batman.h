/*
 * Copyright (C) 2006 B.A.T.M.A.N. contributors:
 * Thomas Lopatic, Corinna 'Elektra' Aichele, Axel Neumann, Marek Lindner
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */



#ifndef _BATMAN_BATMAN_H
#define _BATMAN_BATMAN_H

#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/un.h>
#include <stdint.h>

#include "list-batman.h"
#include "bitarray.h"
#include "hash.h"
#include "allocate.h"
#include "profile.h"
#include "vis-types.h"
#include "ring_buffer.h"



#define SOURCE_VERSION "0.3-beta" //put exactly one distinct word inside the string like "0.3-pre-alpha" or "0.3-rc1" or "0.3"
#define COMPAT_VERSION 4
#define PORT 4305
#define UNIDIRECTIONAL 0x80
#define DIRECTLINK 0x40
#define ADDR_STR_LEN 16
#define TQ_MAX_VALUE 255

#define UNIX_PATH "/var/run/batmand.socket"

#define VIS_COMPAT_VERSION 21



/***
 *
 * Things you should enable via your make file:
 *
 * DEBUG_MALLOC   enables malloc() / free() wrapper functions to detect memory leaks / buffer overflows / etc
 * MEMORY_USAGE   allows you to monitor the internal memory usage (needs DEBUG_MALLOC to work)
 * PROFILE_DATA   allows you to monitor the cpu usage for each function
 *
 ***/


#ifndef REVISION_VERSION
#define REVISION_VERSION "0"
#endif



/*
 * No configuration files or fancy command line switches yet
 * To experiment with B.A.T.M.A.N. settings change them here
 * and recompile the code
 * Here is the stuff you may want to play with:
 */

#define JITTER 100
#define TTL 50                /* Time To Live of broadcast messages */
#define PURGE_TIMEOUT 200000  /* purge originators after time in ms if no valid packet comes in -> TODO: check influence on TQ_LOCAL_WINDOW_SIZE */
#define TQ_LOCAL_WINDOW_SIZE 64     /* sliding packet range of received originator messages in squence numbers (should be a multiple of our word size) */
#define TQ_TOTAL_WINDOW_SIZE 10
#define TQ_LOCAL_BIDRECT_SEND_MINIMUM TQ_LOCAL_WINDOW_SIZE / 8
#define TQ_LOCAL_BIDRECT_RECV_MINIMUM TQ_LOCAL_WINDOW_SIZE / 8
#define TQ_TOTAL_BIDRECT_LIMIT TQ_MAX_VALUE / 10


#define NUM_WORDS ( TQ_LOCAL_WINDOW_SIZE / WORD_BIT_SIZE )



/***
 *
 * Things you should leave as is unless your know what you are doing !
 *
 * BATMAN_RT_TABLE_NETWORKS	routing table for announced networks
 * BATMAN_RT_TABLE_HOSTS	routing table for routes towards originators
 * BATMAN_RT_TABLE_UNREACH	routing table for unreachable routing entry
 * BATMAN_RT_TABLE_TUNNEL	routing table for the tunnel towards the internet gateway
 * BATMAN_RT_PRIO_DEFAULT	standard priority for routing rules
 * BATMAN_RT_PRIO_UNREACH	standard priority for unreachable rules
 * BATMAN_RT_PRIO_TUNNEL	standard priority for tunnel routing rules
 *
 ***/


#define BATMAN_RT_TABLE_NETWORKS 65
#define BATMAN_RT_TABLE_HOSTS 66
#define BATMAN_RT_TABLE_UNREACH 67
#define BATMAN_RT_TABLE_TUNNEL 68

#define BATMAN_RT_PRIO_DEFAULT 6600
#define BATMAN_RT_PRIO_UNREACH BATMAN_RT_PRIO_DEFAULT + 100
#define BATMAN_RT_PRIO_TUNNEL BATMAN_RT_PRIO_UNREACH + 100









extern char *prog_name;
extern uint8_t debug_level;
extern uint8_t debug_level_max;
extern uint8_t gateway_class;
extern uint8_t routing_class;
extern uint8_t num_hna;
extern int16_t originator_interval;
extern uint32_t pref_gateway;
extern uint8_t no_policy_routing;

extern int8_t stop;

extern unsigned char *hna_buff;

extern struct gw_node *curr_gateway;
extern pthread_t curr_gateway_thread_id;

extern uint8_t found_ifs;
extern int32_t receive_max_sock;
extern fd_set receive_wait_set;

extern uint8_t unix_client;

extern struct hashtable_t *orig_hash;

extern struct list_head_first if_list;
extern struct list_head_first hna_list;
extern struct list_head_first gw_list;
extern struct list_head_first forw_list;
extern struct vis_if vis_if;
extern struct unix_if unix_if;
extern struct debug_clients debug_clients;


struct bat_packet
{
	uint32_t orig;
	uint8_t  flags;    /* 0x80: UNIDIRECTIONAL link, 0x40: DIRECTLINK flag, ... */
	uint8_t  ttl;
	uint16_t seqno;
	uint8_t  gwflags;  /* flags related to gateway functions: gateway class */
	uint8_t  version;  /* batman version field */
	uint8_t  tq;
} __attribute__((packed));

struct orig_node                 /* structure for orig_list maintaining nodes of mesh */
{
	uint32_t orig;
	struct neigh_node *router;
	struct batman_if *batman_if;
	TYPE_OF_WORD *bcast_own;
	uint8_t *bcast_own_sum;
	uint8_t tq_own;
	uint32_t last_valid;        /* when last packet from this node was received */
	uint8_t  gwflags;      /* flags related to gateway functions: gateway class */
	unsigned char *hna_buff;
	int16_t  hna_buff_len;
	uint16_t last_real_seqno;   /* last and best known squence number */
	uint8_t last_ttl;         /* ttl of last received packet */
	struct list_head_first neigh_list;
};

struct neigh_node
{
	struct list_head list;
	uint32_t addr;
	uint8_t real_packet_count;
	uint8_t tq_recv[TQ_TOTAL_WINDOW_SIZE];
	uint8_t tq_index;
	uint8_t tq_avg;
	uint32_t last_valid;            /* when last packet via this neighbour was received */
	TYPE_OF_WORD real_bits[NUM_WORDS];
	struct orig_node *orig_node;
	struct batman_if *if_incoming;
};

struct hna_node
{
	struct list_head list;
	uint32_t addr;
	uint8_t netmask;
};

struct forw_node                 /* structure for forw_list maintaining packets to be send/forwarded */
{
	struct list_head list;
	uint32_t send_time;
	uint8_t  own;
	unsigned char *pack_buff;
	int32_t  pack_buff_len;
	struct batman_if *if_outgoing;
};

struct gw_node
{
	struct list_head list;
	struct orig_node *orig_node;
	uint16_t unavail_factor;
	uint32_t last_failure;
	uint32_t deleted;
};

struct batman_if
{
	struct list_head list;
	char *dev;
	int32_t udp_send_sock;
	int32_t udp_recv_sock;
	int32_t udp_tunnel_sock;
	int16_t if_num;
	int32_t if_index;
	uint8_t if_rp_filter_old;
	uint8_t if_send_redirects_old;
	pthread_t listen_thread_id;
	struct sockaddr_in addr;
	struct sockaddr_in broad;
	uint32_t netaddr;
	uint8_t netmask;
	struct bat_packet out;
// 	struct batgat_ioc_args *gate_params;
};

struct gw_client
{
	uint32_t addr;
	uint32_t last_keep_alive;
};

struct vis_if {
	int32_t sock;
	struct sockaddr_in addr;
};

struct unix_if {
	int32_t unix_sock;
	pthread_t listen_thread_id;
	struct sockaddr_un addr;
	struct list_head_first client_list;
};

struct unix_client {
	struct list_head list;
	int32_t sock;
	uint8_t debug_level;
};

struct debug_clients {
	void **fd_list;
	int16_t *clients_num;
	pthread_mutex_t **mutex;
};

struct debug_level_info {
	struct list_head list;
	int32_t fd;
};

struct curr_gw_data {
	unsigned int orig;
	struct gw_node *gw_node;
	struct batman_if *batman_if;
};

struct batgat_ioc_args {
	char dev_name[16];
	uint32_t universal;
	uint32_t ifindex;
};


int8_t batman( void );
void usage( void );
void verbose_usage( void );
int is_batman_if( char *dev, struct batman_if **batman_if );
void update_routes( struct orig_node *orig_node, struct neigh_node *neigh_node, unsigned char *hna_recv_buff, int16_t hna_buff_len );
void update_gw_list( struct orig_node *orig_node, uint8_t new_gwflags );
void get_gw_speeds( unsigned char class, int *down, int *up );
unsigned char get_gw_class( int down, int up );
void choose_gw();


#endif
