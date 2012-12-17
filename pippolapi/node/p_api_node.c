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
#include "p_api_node.h"
struct str_node *pippolo_neighbours = NULL;
char *pippolo_nomenclature = NULL;
p_discretize pippolo_discretizer = NULL;
int p_node_pippolo_init (const char *nomenclature) {
    int result = pippolo_true;
    srand((unsigned int)time(NULL));
    if (pippolo_nomenclature)
        pippolo_null_free(pippolo_nomenclature);
    pippolo_append(pippolo_nomenclature, nomenclature);
    return result;
}

void p_node_pippolo_quit (void) {
    struct str_node *singleton = pippolo_neighbours;
    while ((pippolo_neighbours = singleton->next)) {
        pthread_mutex_destroy(&singleton->connection.mutex);
        pippolo_null_free(singleton);
        singleton = pippolo_neighbours;
    }
}

int p_node_pippolo_add (const char *address, unsigned short port) {
    int result = pippolo_true;
    struct str_node *singleton;
    if ((singleton = (struct str_node *) pippolo_malloc(sizeof(str_node)))) {
        snprintf(singleton->address, pippolo_address_size, "%s", address);
        snprintf(singleton->connection.port, pippolo_port_size, "%hu", port);
        singleton->connection.hook = pippolo_null_socket;
        singleton->connection.established.value = pippolo_false;
        pthread_mutex_init(&singleton->connection.mutex, NULL);
        singleton->next = pippolo_neighbours;
        pippolo_neighbours = singleton;
    } else
        result = pippolo_false;
    return result;
}

int p_node_action (const char *ID, enum enum_data_action action, struct str_record *records, unsigned int ttl, p_hook hooker) {
    int result = pippolo_false;
    struct str_parameter *parameter;
    pthread_t link;
    pthread_attr_t attributes;
    if ((parameter = (struct str_parameter *) pippolo_malloc(sizeof(str_parameter)))) {
        parameter->ID = NULL;
        pippolo_append(parameter->ID, ID);
        if ((parameter->records = _p_node_action_convert(records))) {
            parameter->hooker = hooker;
            parameter->ttl = ttl;
            switch (action) {
                case EDATA_ACTIONS_ADD:
                    parameter->action = 'A';
                    break;
                case EDATA_ACTIONS_GET:
                    parameter->action = 'G';
                    break;
                case EDATA_ACTIONS_DELETE:
                    parameter->action = 'D';
                    break;
            }
            pthread_attr_init(&attributes);
            if (pthread_create(&link, &attributes, &_p_node_action, (void *)parameter) == 0) {
                pthread_detach(link);
                result = pippolo_true;
            }
        }
    }
    return result;
}

void *_p_node_action (void *parameter) {
    int result = pippolo_false;
    struct str_parameter *input = (struct str_parameter *)parameter;
    struct str_node *singleton;
    pthread_t link;
    pthread_attr_t attributes;
    while (!result) {
        singleton = pippolo_neighbours;
        while ((!result) && (singleton)) {
            if (pippolo_trylock(singleton->connection.mutex) == 0) {
                input->node = singleton;
                pthread_attr_init(&attributes);
                if (pthread_create(&link, &attributes, &_p_node_action_build, (void *)input) == 0) {
                    pthread_detach(link);
                    result = pippolo_true;
                }
            }
            singleton = singleton->next;
        }
        p_wait(pippolo_default_retry_sec, pippolo_default_retry_usec);
    }
    pthread_exit(NULL);
    return NULL;
}

char *_p_node_action_convert (struct str_record *records) {
    char header[pippolo_default_size], footer[pippolo_default_size], *result = NULL;
    struct str_key *pointer;
    while (records) {
        snprintf(header, pippolo_default_size, "<%s %s=%d %s=%ld>", pippolo_xml_record, pippolo_xml_key_hash, records->hash, pippolo_xml_key_time, records->timestamp);
        pippolo_append(result, header);
        pointer = records->data;
        while (pointer) {
            if (pointer->primary.value)
                snprintf(header, pippolo_default_size, "<%s %s=%s %s=%s>", pippolo_xml_value, pippolo_xml_key_key, pointer->key, pippolo_xml_key_primary, pippolo_xml_value_true);
            else
                snprintf(header, pippolo_default_size, "<%s %s=%s>", pippolo_xml_value, pippolo_xml_key_key, pointer->key);
            snprintf(footer, pippolo_default_size, "</%s>", pippolo_xml_value);
            pippolo_append(result, header);
            pippolo_append(result, pointer->value);
            pippolo_append(result, footer);
            pointer = pointer->next;
        }
        snprintf(footer, pippolo_default_size, "</%s>", pippolo_xml_record);
        pippolo_append(result, footer);
        records = records->next;
    }
    return result;
}

void *_p_node_action_build(void *parameter) {
    struct str_parameter *input = (struct str_parameter *)parameter;
    char header[pippolo_default_size], footer[pippolo_default_size], token[(pippolo_token+1)], *request = NULL;
    snprintf(token, (pippolo_token+1), "%d", rand());
    snprintf(header, pippolo_default_size, "<%s %s=%s %s=%c %s=%s %s=%d>", pippolo_xml_node, pippolo_xml_key_hash, pippolo_hash_cover, pippolo_xml_key_action, input->action, pippolo_xml_key_token, token, pippolo_xml_key_live, input->ttl);
    snprintf(footer, pippolo_default_size, "</%s>", pippolo_xml_node);
    pippolo_append(request, header);
    pippolo_append(request, input->records);
    pippolo_append(request, footer);
    _p_node_action_run(input, request, token);
    pippolo_null_free(request);
    pippolo_unlock(input->node->connection.mutex);
    if (input->records)
        pippolo_free(input->records);
    if (input->ID)
        pippolo_free(input->ID);
    pippolo_free(input);
    pthread_exit(NULL);
    return NULL;
}

void *_p_node_action_run (struct str_parameter *input, const char *request, const char *token) {
    int hook, syncronized;
    struct str_token_node *tokens = NULL;
    struct str_xml_node *node = NULL;
    struct str_xml_key *key = NULL;
    char *nomenclature = NULL, *buffer = NULL;
    if ((hook = p_network_connect(input->node->address, input->node->connection.port)) != pippolo_null_socket) {
        if (p_network_write(hook, pippolo_nomenclature) > 0)
            if ((p_network_receive(hook, &nomenclature, pippolo_default_timeout_sec, pippolo_default_timeout_usec))) {
                if (p_network_write(hook, (char *)request))
                    while (pippolo_true)
                        if ((p_network_receive(hook, &buffer, pippolo_default_timeout_sec, pippolo_default_timeout_usec)) && (buffer)) {
                            syncronized = pippolo_false;
                            if ((tokens = p_xml_tokenizer(buffer))) {
                                if ((p_xml_analyze(&node, tokens))) {
                                    key = node->keys;
                                    while (key) {
                                        if (p_string_cmp(key->key, pippolo_xml_key_token) == 0) {
                                            if (p_string_cmp(token, key->value) == 0)
                                                syncronized = pippolo_true;
                                            break;
                                        }
                                        key = key->next;
                                    }
                                    if ((syncronized) && (input->hooker))
                                        input->hooker(input->ID, node);
                                    p_xml_free(&node);
                                }
                                p_tokens_free(&tokens);
                            }
                            pippolo_null_free(buffer);
                            if (syncronized)
                                break;
                        }
                pippolo_null_free(nomenclature);
            }
        shutdown(hook, SHUT_RDWR);
        close(hook);
    }
    return NULL;
}

int p_node_record_add(struct str_record **records) {
    int result = pippolo_true;
    struct str_record *singleton;
    if ((singleton = (struct str_record *) pippolo_malloc(sizeof(str_record)))) {
        singleton->timestamp = time(NULL);
        singleton->data = NULL;
        singleton->next = *records; /* head insertion */
        *records = singleton;
    } else
        result = pippolo_false;
    return result;
}

int p_node_record_keys_add (struct str_record **records, const char *key, const char *value, int primary) {
    int result = pippolo_true;
    struct str_key *singleton;
    if ((*records)) {
        if ((singleton = (struct str_key *) pippolo_malloc(sizeof(str_key)))) {
            singleton->value = NULL;
            pippolo_append(singleton->value, value);
            snprintf(singleton->key, (pippolo_key+1), "%s", key);
            if ((singleton->primary.value = primary))
                if (pippolo_discretizer)
                    (*records)->hash = pippolo_discretizer(value);
            singleton->next = (*records)->data;
            (*records)->data = singleton;
        } else
            result = pippolo_false;
    } else
        result = pippolo_false;
    return result;
}

void p_node_records_destroy (struct str_record **records) {
    if ((*records)) {
        p_node_records_destroy(&((*records)->next));
        _p_node_records_destroy_key(&((*records)->data));
        pippolo_null_free(*records);
    }
}

void _p_node_records_destroy_key (struct str_key **key) {
    if ((*key)) {
        _p_node_records_destroy_key(&((*key)->next));
        if ((*key)->value)
            pippolo_null_free((*key)->value);
        pippolo_null_free(*key);
    }
}
