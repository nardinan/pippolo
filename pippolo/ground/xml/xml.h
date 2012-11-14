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
#ifndef pippolo_xml_h
#define pippolo_xml_h
#include <regex.h>
#include "network.h"
#define pippolo_data 256
#define p_check_consistence(chr)\
    (((chr)!=' ')&&((chr)!='\t')&&((chr)!='\n'))
#define pippolo_null_regex 0
enum enum_xml_action {
    EXML_ACTIONS_READ,
    EXML_ACTIONS_TITLE,
    EXML_ACTIONS_KEY,
    EXML_ACTIONS_KVALUE,
    EXML_ACTIONS_CVALUE,
    EXML_ACTIONS_JUMP
};
typedef struct str_token_flags {
    struct pippolo_bool symbol, special, character, safe, interrupt;
} str_token_flags;
typedef struct str_token_controller {
    struct pippolo_bool opened, closed;
} str_token_controller;
typedef struct str_token_node {
    char token[pippolo_data];
    unsigned int length;
    struct str_token_flags flags;
    struct str_token_node *next;
} str_token_node;
typedef struct str_xml_key {
    char *key, *value;
    struct str_xml_key *next;
} str_xml_key;
typedef struct str_xml_node {
    regex_t regex;
    struct pippolo_bool compiled;
    char *label, *value;
    struct str_xml_key *keys;
    struct str_xml_node *father, *children, *next;
} str_xml_node;
str_token_node *p_xml_tokenizer (char *source);
int p_xml_key_append (struct str_xml_key **record, char *key, char *value);
int p_xml_analyze (struct str_xml_node **root, str_token_node *node);
int _p_xml_analyze (struct str_xml_node **root, str_token_node *node, enum enum_xml_action action);
int p_xml_duplicate_tree (struct str_xml_node **hook, struct str_xml_node *father, struct str_xml_node *source);
int p_xml_node_allocate (struct str_xml_node **hook);
int p_xml_data_trimming (str_xml_node *root);
int p_xml_draw (FILE *stream, str_xml_node *root);
int _p_xml_draw (FILE *stream, str_xml_node *root, int deep);
int p_xml_string (char **string, str_xml_node *root);
int _p_xml_string (char **string, str_xml_node *root);
void p_tokens_free (struct str_token_node **root);
void p_xml_free (struct str_xml_node **root);
void p_xml_node_free (struct str_xml_node **root);
void p_xml_key_free (struct str_xml_key **root);
#endif
