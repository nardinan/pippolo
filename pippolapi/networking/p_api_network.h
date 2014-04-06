/*
   Pippolo - a nosql distributed database.
   Copyright (C) 2012 Andrea Nardinocchi (nardinocchi@psychogames.net)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
   */
#ifndef pippolo_network_h
#define pippolo_network_h
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "p_api_endian.h"
#include "p_api_local.string.h"
#define pippolo_intro sizeof(int)
#define pippolo_max_queue 8
#define pippolo_port_size 6
#define pippolo_null_socket -1
#define pippolo_ack_request "ACKREQ"
#define pippolo_ack_response "ACKRES"
#define pippolo_ack_len 6
#define pippolo_step_len 512
#ifndef pippolo_again
#define pippolo_again EAGAIN
#endif
typedef struct str_socket {
	char port[pippolo_port_size];
	struct pippolo_bool established;
	int hook;
	pthread_mutex_t mutex;
} str_socket;
int p_network_serverize (const char *address, const char *port, unsigned int queue);
int p_network_connect (const char *address, const char *port);
int p_network_accept (int hook);
int p_network_receive (int hook, char **buffer, int sec, int usec);
size_t p_network_write (int hook, char *buffer);
#endif
