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
#ifndef pippolo_data_h
#define pippolo_data_h
#include <ctype.h>
#include "network.h"
#include "xml.h"
#define pippolo_max_neighbours 10
#define pippolo_key 8
#define pippolo_token pippolo_key
#define pippolo_default_expiration 1
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
#define pippolo_key_append_head(rt,sn)\
	(((!(rt))||((sn)->primary.value)||((!(rt)->primary.value)&&(p_string_case_cmp((sn)->key,(rt)->key)<0))))
enum enum_data_action {
	EDATA_ACTIONS_ADD,
	EDATA_ACTIONS_DELETE,
	EDATA_ACTIONS_GET,
	EDATA_ACTIONS_RESPONSE,
	EDATA_ACTIONS_NULL
};
enum enum_hook_action {
	EHOOK_ACTIONS_FORWARD = 0,
	EHOOK_ACTIONS_REPLY,
	EHOOK_ACTIONS_NULL
};
typedef struct str_key {
	char key[(pippolo_key+1)], *value;
	struct pippolo_bool primary;
	struct str_key *next;
} str_key;
typedef struct str_record {
	time_t time;
	int hash;
	struct str_key *keys;
	struct str_record *next;
} str_record;
typedef struct str_level {
	pthread_mutex_t mutex;
	char key[(pippolo_key+1)];
	struct str_record *values;
	struct str_level *childrens, *next;
} str_level;
typedef struct str_primary {
	char *value;
	struct str_level *level;
} str_primary;
typedef struct str_action {
	pthread_mutex_t mutex;
	time_t live, last;
	unsigned int owner, hooks[EHOOK_ACTIONS_NULL];
	char token[(pippolo_token+1)];
	enum enum_data_action kind;
	struct {
		struct pippolo_bool hash[pippolo_hash_elements];
	} range, covered;
	struct pippolo_bool terminated, duplicated, executed[pippolo_max_neighbours], completed[pippolo_max_neighbours];
	struct str_xml_node *original, *records, *forward, *reply;
	struct str_action *next;
} str_action;
extern pthread_mutex_t mutex_actions;
extern struct str_action *actions;
extern struct str_level *root;
void p_data_clean (void);
int p_data_key_append (struct str_key **record, const char *key, char *value);
int _p_data_key_create (struct str_key **record, const char *key, char *value, int primary);
int _p_data_key_allocate (struct str_key **record, size_t length);
int _p_data_key_append (struct str_key **record, struct str_key *singleton);
int p_data_record_insert (struct str_record **level, unsigned int hash, time_t timestamp, struct str_key *record);
int p_data_level_append (struct str_level **hook, unsigned int hash, time_t timestamp, struct str_key *record);
int _p_data_level_create (struct str_level **hook, const char *key);
int _p_data_level_allocate (struct str_level **hook);
int _p_data_level_append (struct str_level **hook, unsigned int hash, time_t timestamp, struct str_key *record, struct str_key *pointer);
int p_data_action (char *data, unsigned int owner);
int _p_data_action (struct str_xml_node *nodes, unsigned int owner);
int _p_data_action_create (struct str_xml_node *nodes, struct str_xml_node *singleton, unsigned int owner);
int _p_data_action_allocate (struct str_action **action, unsigned int owner);
int _p_data_action_insert (struct str_action *action);
int _p_data_action_insert_duplicate (struct str_action *action, struct str_xml_node **hook, struct str_xml_node *source);
struct str_xml_key *_p_data_action_insert_duplicate_hash (struct str_xml_node *record);
int _p_data_action_execute (struct str_action *action);
int _p_data_action_execute_add (struct str_action *action);
int _p_data_action_execute_del (struct str_action *action);
int _p_data_action_execute_get (struct str_action *action);
int p_data_insert (struct str_xml_node *hook);
int p_data_pruning (struct str_level *root, const char *primary, time_t timestamp);
int _p_data_pruning (struct str_key *root, const char *primary);
int p_data_delete (struct str_action *action, struct str_xml_node *nodes, struct str_level *hook);
int _p_data_delete_level (struct str_action *action, struct str_xml_node *nodes, struct str_record **value);
int p_data_match (struct str_action *action, struct str_xml_node *nodes, struct str_level *hook);
int _p_data_match_level (struct str_action *action, struct str_xml_node *nodes, struct str_record *values);
int _p_data_match_record (struct str_xml_node *nodes, struct str_key *values);
int _p_data_match_conversion (struct str_xml_node **hook, unsigned int hash, time_t timestamp, struct str_key *values);
int _p_data_match_conversion_keys (struct str_xml_node **hook, struct str_xml_node *father, struct str_key *values);
int p_data_reply_insert (struct str_action *action, struct str_xml_node *record);
int p_data_draw (FILE *stream, struct str_level *root);
int _p_data_draw (FILE *stream, struct str_level *root, unsigned int level);
int p_data_records_draw (FILE *stream, struct str_record *record, unsigned int level);
int p_data_keys_draw (FILE *stream, time_t timestamp, unsigned int hash, struct str_key *record, unsigned int level);
void p_data_free (struct str_level **root);
void p_data_free_records (struct str_record **root);
void p_data_free_keys (struct str_key **root);
#endif
