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
#include "p_api_network.h"
#include "p_api_xml.h"
#define pippolo_address_size 16
#define pippolo_key 8
#define pippolo_token 8
#define pippolo_default_timeout_sec 5
#define pippolo_default_timeout_usec 0
#define pippolo_default_retry_sec 0
#define pippolo_default_retry_usec 5000
#define pippolo_default_size 1024
#define pippolo_xml_node "node"
#define pippolo_xml_key_token "token"
#define pippolo_xml_key_live "live"
#define pippolo_xml_key_action "action"
#define pippolo_xml_record "record"
#define pippolo_xml_key_time "time"
#define pippolo_xml_key_hash "hash"
#define pippolo_xml_value "value"
#define pippolo_xml_key_primary "primary"
#define pippolo_xml_key_key "key"
#define pippolo_xml_value_true "true"
typedef struct str_key {
	char key[(pippolo_key+1)], *value;
	struct pippolo_bool primary;
	struct str_key *next;
} str_key;
typedef struct str_record {
	time_t timestamp;
	unsigned int hash;
	struct str_key *data;
	struct str_record *next;
} str_record;
enum enum_data_action {
	EDATA_ACTIONS_ADD,
	EDATA_ACTIONS_DELETE,
	EDATA_ACTIONS_GET
};
typedef void (*p_hook)(const char *ID, str_xml_node *records);
typedef int (*p_discretize)(const char *value);
typedef struct str_node {
	char address[pippolo_address_size];
	struct str_socket connection;
	struct str_node *next;
} str_node;
typedef struct str_parameter {
	char *ID, *records, action;
	struct str_node *node;
	unsigned int ttl;
	p_hook hooker;
} str_parameter;
extern struct str_node *pippolo_neighbours;
extern char *pippolo_nomenclature;
extern p_discretize pippolo_discretizer;
int p_node_pippolo_init (const char *nomenclature);
void p_node_pippolo_quit (void);
int p_node_pippolo_add (const char *address, unsigned short port);
int p_node_action (const char *ID, enum enum_data_action action, struct str_record *records, unsigned int ttl, p_hook hooker);
void *_p_node_action (void *parameter);
char *_p_node_action_convert (struct str_record *records);
void *_p_node_action_build (void *parameter);
void *_p_node_action_run (struct str_parameter *input, const char *request, const char *token);
int p_node_record_add (struct str_record **records);
int p_node_record_keys_add (struct str_record **records, const char *key, const char *value, int primary);
void p_node_records_destroy (struct str_record **records);
void _p_node_records_destroy_key (struct str_key **key);
#endif
