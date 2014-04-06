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
#ifndef pippolo_node_h
#define pippolo_node_h
#include "network.h"
#include "data.h"
#define pippolo_address_size 16
#define pippolo_queue 8
#define pippolo_configuration_line 512
#define pippolo_default_size 1024
#define pippolo_safe_token(a) ((p_string_len((a))>0)?(a):"(not named yet)")
enum enum_channel {
	ECHANNELS_MAIN=0,
	ECHANNELS_SERVICE,
	ECHANNELS_NULL
};
extern char p_channel_description[][pippolo_channel_length];
typedef struct str_node {
	char address[pippolo_address_size], token[(pippolo_token+1)];
	struct str_socket connections[ECHANNELS_NULL];
	struct {
		int sec, usec;
	} timeout, refresh;
	struct {
		struct pippolo_bool hash[pippolo_hash_elements];
	} range;
	struct pippolo_bool initialized, onetime;
	pthread_mutex_t mutex;
} str_node;
typedef struct str_parameter {
	int index;
	enum enum_channel channel;
} str_parameter;
extern pthread_mutex_t mutex_nodes;
extern struct pippolo_bool alives[pippolo_max_neighbours];
extern struct str_node neighbours[pippolo_max_neighbours], server;
void p_node_clean (void);
int p_node_setup (FILE *stream);
void p_node_configure (unsigned int index, const char *key, const char *value);
void p_server_configure (const char *key, const char *value);
void p_node_synchronize (void);
void _p_node_synchronize (unsigned int index, unsigned int identify);
void p_node_check_start (void);
void _p_node_check_start (unsigned int index);
void p_server_check_start (void);
void _p_server_check_start (void);
void *p_node_thread_check (void *parameters);
int _p_node_thread_check_read (str_parameter *value);
void _p_node_thread_check_action (str_parameter *value);
void _p_node_thread_check_action_flush (struct str_action *action);
void _p_node_thread_check_shutdown (int index, enum enum_channel channel);
void *p_server_thread_check (void *parameters);
int p_node_handshake_client (int hook, char **token, int sec, int usec);
int p_node_handshake_server (int hook, char **token, int sec, int usec);
int p_node_initialize (unsigned int index, enum enum_channel channel);
int p_node_alloc (int hook, enum enum_channel channel, char *token);
#endif
