/*
 * Copyright (C) 2006 BATMAN contributors:
 * Thomas Lopatic
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

#ifndef _BATMAN_OS_H
#define _BATMAN_OS_H

unsigned int get_time(void);
void set_forwarding(int);
int get_forwarding(void);
void output(char *format, ...);
void add_del_route(unsigned int dest, unsigned int router, int del);
int is_aborted();
void *alloc_memory(int len);
void free_memory(void *mem);
void addr_to_string(unsigned int addr, char *str, int len);
int receive_packet(unsigned char *buff, int len, unsigned int *neigh, unsigned int timeout);
int send_packet(unsigned char *buff, int len);
int rand_num(int limit);

#endif
