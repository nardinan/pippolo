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
#include "data.h"
#include "node.h"
pthread_mutex_t mutex_actions;
struct str_action *actions = NULL;
struct str_level *root = NULL;
void p_data_clean (void) {
    pthread_mutex_init(&mutex_actions, NULL);
}

int p_data_key_append (struct str_key **record, const char *key, char *value) {
    struct str_key *singleton = NULL;
    int result = pippolo_true;
    if (p_string_len(key) < pippolo_key) {
        if ((result = _p_data_key_create(&singleton, key, value, (*record)?pippolo_false:pippolo_true)))
            result = _p_data_key_append(record, singleton);
    } else
        result = pippolo_false;
    return result;
}

int _p_data_key_create (struct str_key **record, const char *key, char *value, int primary) {
    int result = pippolo_true;
    size_t length = p_string_len(value);
    if ((result = _p_data_key_allocate(record, (length+1)))) {
        memcpy((*record)->key, key, pippolo_key);
        memcpy((*record)->value, value, length);
        pippolo_capitalize((*record)->key);
        (*record)->primary.value = primary;
    }
    return result;
}

int _p_data_key_allocate (struct str_key **record, size_t length) {
    int result = pippolo_true;
    if ((*record = (struct str_key *) pippolo_malloc(sizeof(str_key)))) {
        memset((*record)->key, '\0', (pippolo_key+1));
        (*record)->next = NULL;
        if (((*record)->value = (char *) pippolo_malloc(length)))
            memset((*record)->value, '\0', length);
        else
            pippolo_kill("out of memory, I'm dying bleaaargh"); 
    } else
        pippolo_kill("out of memory, I'm dying bleaaargh");
    return result;
}

int _p_data_key_append (struct str_key **record, struct str_key *singleton) {
    int result = pippolo_true;
    if (!pippolo_key_append_head((*record),singleton)) {
        if ((*record)->next) {
            if (((p_string_case_cmp(singleton->key, (*record)->next->key) < 0))) {
                singleton->next = (*record)->next;
                (*record)->next = singleton;
            } else
                result = _p_data_key_append(&((*record)->next), singleton);
        } else
            (*record)->next = singleton;
    } else {
        singleton->next = *record;
        *record = singleton;
    }
    return result;
}

int p_data_record_insert (struct str_record **level, unsigned int hash, time_t timestamp, struct str_key *record) {
    struct str_record *singleton;
    int result = pippolo_true;
    if ((singleton = (struct str_record *) pippolo_malloc(sizeof(str_record)))) {
        singleton->keys = record;
        singleton->hash = hash;
        singleton->time = timestamp;
        singleton->next = *level;
        *level = singleton;
    } else
        pippolo_kill("out of memory, I'm dying bleaaargh");
    return result;
}

int p_data_level_append (struct str_level **hook, unsigned int hash, time_t timestamp, struct str_key *record) {
    return _p_data_level_append(hook, hash, timestamp, record, record);
}

int _p_data_level_create (struct str_level **hook, const char *key) {
    int result = pippolo_true;
    if ((result = _p_data_level_allocate(hook))) {
        snprintf((*hook)->key, (pippolo_key+1), "%s", key);
        pippolo_capitalize((*hook)->key);
    }
    return result;
}

int _p_data_level_allocate (struct str_level **hook) {
    int result = pippolo_true;
    if (((*hook) = (struct str_level *) pippolo_malloc(sizeof(str_level)))) {
        memset((*hook)->key, '\0', (pippolo_key+1));
        (*hook)->childrens = (*hook)->next = NULL;
        (*hook)->values = NULL;
        pthread_mutex_init(&((*hook)->mutex), NULL);
    } else
        pippolo_kill("out of memory, I'm dying bleaaargh");
    return result;
}

int _p_data_level_append (struct str_level **hook, unsigned int hash, time_t timestamp, struct str_key *record, struct str_key *pointer) {
    int result = pippolo_true;
    if ((*hook)) {
        if (p_string_case_cmp((*hook)->key, pointer->key) == 0) {
            if (pointer->next)
                result = _p_data_level_append(&((*hook)->childrens), hash, timestamp, record, pointer->next);
            else {
                pippolo_lock((*hook)->mutex);
                result = p_data_record_insert(&((*hook)->values), hash, timestamp, record); /* the function is thread unsafe, so we have to lock it */
                pippolo_unlock((*hook)->mutex);
            }
        } else
            result = _p_data_level_append(&((*hook)->next), hash, timestamp, record, pointer);
    } else if ((result = _p_data_level_create(hook, pointer->key)))
        result = _p_data_level_append(hook, hash, timestamp, record, pointer);
    return result;
}

int p_data_action (char *data, unsigned int owner) {
    int result = pippolo_true;
    struct str_token_node *structure = NULL;
    struct str_xml_node *nodes = NULL;
    if (data) {
        if ((structure = p_xml_tokenizer(data))) {
            if ((result = p_xml_analyze(&nodes, structure))) {
                if (!_p_data_action(nodes, owner))
                    p_xml_free(&nodes);
            } else
                result = pippolo_false;
            p_tokens_free(&structure);
        } else
            result = pippolo_false;
    } else
        result = pippolo_false;
    return result;
}

int _p_data_action (struct str_xml_node *nodes, unsigned int owner) {
    int result = pippolo_false;
    struct str_xml_node *singleton = nodes;
    while (singleton) {
        if (p_string_cmp(singleton->label, pippolo_xml_node) == 0) { /* manage a single request for time (break at end) */
            /*
                TODO: fast XML generation (without recalculating every time the forward/reply string)
             */
            result = _p_data_action_create(nodes, singleton, owner);
            break;
        }
        singleton = singleton->next;
    }
    return result;
}

int _p_data_action_create (struct str_xml_node *nodes, struct str_xml_node *singleton, unsigned int owner) {
    int result = pippolo_true, hashes;
    struct str_xml_key *data = singleton->keys;
    struct str_action *action = NULL;
    struct str_token_node *tokens;
    char reply[pippolo_default_size], *pointer;
    if ((result = _p_data_action_allocate(&action, owner))) {
        action->original =  nodes;
        action->records = singleton->children;
        while (data) {
            if (p_string_cmp(data->key, pippolo_xml_key_token) == 0)
                snprintf(action->token, (pippolo_token+1), "%s", data->value);
            else if (p_string_cmp(data->key, pippolo_xml_key_live) == 0)
                action->live = atoi(data->value); /* number of seconds of waiting for a reply */
            else if (p_string_cmp(data->key, pippolo_xml_key_hash) == 0) { /* covered hashes */
                pointer = data->value;
                while (*pointer) {
                    action->range.hash[(*pointer)-48].value = pippolo_true;
                    pointer++;
                }
            } else if (p_string_cmp(data->key, pippolo_xml_key_action) == 0)
                switch (data->value[0]) {
                    case 'A':
                        action->kind = EDATA_ACTIONS_ADD;
                        break;
                    case 'D':
                        action->kind = EDATA_ACTIONS_DELETE;
                        break;
                    case 'G':
                        action->kind = EDATA_ACTIONS_GET;
                        break;
                    case 'R':
                        action->kind = EDATA_ACTIONS_RESPONSE;
                        break;
                    default:
                        action->kind = EDATA_ACTIONS_NULL;
                        break;
                }
            data = data->next;
        }
        if (action->kind == EDATA_ACTIONS_RESPONSE) {
            action->forward = action->reply = NULL;
            action->hooks[EHOOK_ACTIONS_FORWARD] = action->hooks[EHOOK_ACTIONS_REPLY] = 0;
        } else {
            /* maybe add, delete or get */
            action->forward = action->original;
            snprintf(reply, pippolo_default_size, "<%s %s=R %s=%s %s=%ld %s=%s></%s>", pippolo_xml_node, pippolo_xml_key_action, pippolo_xml_key_token, action->token, pippolo_xml_key_live, action->live, pippolo_xml_key_hash, pippolo_hash_cover, pippolo_xml_node);
            if ((tokens = p_xml_tokenizer(reply))) {
                p_xml_analyze(&(action->reply), tokens);
                p_tokens_free(&tokens);
                if ((action->kind == EDATA_ACTIONS_GET) &&
                        (action->forward) &&
                            (action->forward->keys)) {
                    /* get action */
                    data = action->forward->keys;
                    while (data) {
                        if (p_string_cmp(data->key, pippolo_xml_key_hash) == 0) {
                            if (data->value)
                                pippolo_null_free(data->value);
                            if ((data->value = (char *) pippolo_malloc((pippolo_hash_elements+1)))) {
                                memset(data->value, '\0', (pippolo_hash_elements+1));
                                pointer = data->value;
                                pippolo_lock(server.mutex);
                                for (hashes = 0; hashes < pippolo_hash_elements; hashes++)
                                    if ((action->range.hash[hashes].value) && (!server.range.hash[hashes].value)) {
                                        *pointer = (char)(48+hashes);
                                        pointer++;
                                    }
                                pippolo_unlock(server.mutex);
                                if (p_string_len(data->value) == 0)
                                    action->forward = NULL;
                            } else
                                pippolo_kill("out of memory, I'm dying bleaaargh");
                            break;
                        }
                        data = data->next;  
                    }
                }
            } else
                result = pippolo_false;
        }
        if (!(result = _p_data_action_insert(action))) {
            if (action->reply)
                p_xml_free(&(action->reply));
            pippolo_free(action);
        } else {
            action->last = time(NULL);
            pippolo_log(ELOG_DEBUGGING, "forwards for action %s %d, and reply %d", action->token, action->hooks[EHOOK_ACTIONS_FORWARD], action->hooks[EHOOK_ACTIONS_REPLY]);
        }
    }
    return result;
}

int _p_data_action_allocate (struct str_action **action, unsigned int owner) {
    int result = pippolo_true, index, hashes, alive;
    if (((*action) = (struct str_action *) pippolo_malloc(sizeof(str_action)))) {
        memset((*action)->token, '\0', (pippolo_token+1));
        (*action)->terminated.value = (*action)->duplicated.value = pippolo_false;
        (*action)->owner = owner;
        (*action)->hooks[EHOOK_ACTIONS_FORWARD] = (*action)->hooks[EHOOK_ACTIONS_REPLY] = 0;
        for (index = 0; index < pippolo_max_neighbours; index++) {
            pippolo_lock(mutex_nodes);
            alive = alives[index].value;
            pippolo_unlock(mutex_nodes);
            (*action)->executed[index].value = (*action)->completed[index].value = (!alive);
            if (alive)
                if (index != (*action)->owner)
                    (*action)->hooks[EHOOK_ACTIONS_FORWARD]++;
        }
        (*action)->hooks[EHOOK_ACTIONS_REPLY] = (*action)->hooks[EHOOK_ACTIONS_FORWARD];
        for (hashes = 0; hashes < pippolo_hash_elements; hashes++) {
            (*action)->range.hash[hashes].value = pippolo_false;
            pippolo_lock(server.mutex);
            (*action)->covered.hash[hashes].value = server.range.hash[hashes].value;
            pippolo_unlock(server.mutex);
        }
        (*action)->live = pippolo_default_expiration;
        (*action)->reply = (*action)->forward = (*action)->original = (*action)->records = NULL;
        pthread_mutex_init(&((*action)->mutex), NULL);
    } else
        pippolo_kill("out of memory, I'm dying bleaaargh");
    return result;
}

int _p_data_action_insert (struct str_action *action) {
    int result = (action->kind == EDATA_ACTIONS_RESPONSE)?pippolo_false:pippolo_true, index, founded = pippolo_false;
    struct str_action *pointer;
    struct str_xml_node *nodes = NULL, *singleton, *next;
    pippolo_lock(mutex_actions);
    pointer = actions;
    pippolo_unlock(mutex_actions);
    while (pointer) {
        /* we can lock the pointer->mutex here because the action 'action' didn't exists in the actions' list (so we haven't locked anything) */
        pippolo_lock(pointer->mutex);
        if (!pointer->duplicated.value) /* is not a symbolic action */
            if (p_string_cmp(action->token, pointer->token) == 0) {
                if (action->kind == EDATA_ACTIONS_RESPONSE) {
                    /* if the incoming action contains records, we have to append them into the original action response */
                    if (action->records)
                        if (_p_data_action_insert_duplicate(pointer, &nodes, action->records)) {
                            singleton = nodes;
                            while (singleton) {
                                next = singleton->next; /* after the insertion the pointer to 'next' change */
                                if (!(p_data_reply_insert(pointer, singleton)))
                                    p_xml_node_free(&singleton);
                                singleton = next;
                            }
                        }
                    pippolo_log(ELOG_DEBUGGING, "there is a reply (so we can decrement the hooker) for the action %s", pointer->token);
                    if (!pointer->completed[action->owner].value) {
                        pointer->completed[action->owner].value = pippolo_true;
                        pointer->hooks[EHOOK_ACTIONS_REPLY] = p_max((pointer->hooks[EHOOK_ACTIONS_REPLY]-1),0);
                    }
                } else {
                    action->hooks[EHOOK_ACTIONS_FORWARD] = action->hooks[EHOOK_ACTIONS_REPLY] = 0;
                    action->duplicated.value = pippolo_true;
                    for (index = 0; index < pippolo_max_neighbours; index++)
                        action->executed[index].value = action->completed[index].value = pippolo_true;
                }
                founded = pippolo_true;
            }
        pippolo_unlock(pointer->mutex);
        if (founded)
            break;
        pointer = pointer->next;
    }
    if (result) {
        /* add action into queue */
        pippolo_lock(mutex_actions);
        action->next = actions;
        actions = action;
        pippolo_unlock(mutex_actions);
    }
    return result;
}

int _p_data_action_insert_duplicate (struct str_action *action, struct str_xml_node **hook, struct str_xml_node *source) {
    int result = pippolo_true, covered, hashes;
    struct str_xml_key *key;
    struct str_xml_node *singleton;
    struct pippolo_bool hash[pippolo_hash_elements];
    for (hashes = 0; hashes < pippolo_hash_elements; hashes++)
        hash[hashes].value = pippolo_false;
    while (source) {
        if ((key = _p_data_action_insert_duplicate_hash(source))) {
            covered = p_string_atoi_unsigned(key->value);
            if (!action->covered.hash[covered].value) {
                if ((result = p_xml_node_allocate(&singleton))) {
                    if (source->label)
                        pippolo_append(singleton->label, source->label);
                    if (source->value)
                        pippolo_append(singleton->value, source->value);
                    key = source->keys;
                    while(key) {
                        if (!(result = p_xml_key_append(&(singleton->keys), key->key, key->value)))
                            break;
                        key = key->next;
                    }
                    /* pure copy of each children value */
                    p_xml_duplicate_tree(&(singleton->children), singleton, source->children);
                    singleton->next = (*hook);
                    *hook = singleton;
                    singleton = NULL;
                }
                hash[covered].value = pippolo_true;
            }
        }
        source = source->next;
    }
    for (hashes = 0; hashes < pippolo_hash_elements; hashes++)
        action->covered.hash[hashes].value = (action->covered.hash[hashes].value||hash[hashes].value);
    return result;
}

struct str_xml_key *_p_data_action_insert_duplicate_hash (struct str_xml_node *record) {
    struct str_xml_key *result;
    int founded = pippolo_false;
    if (p_string_cmp(record->label, pippolo_xml_record) == 0) {
        result = record->keys;
        while (result) {
            if (p_string_cmp(result->key, pippolo_xml_key_hash) == 0) {
                founded = pippolo_true;
                break;
            }
            result = result->next;
        }
    }
    return result;
}

int _p_data_action_execute (struct str_action *action) {
    int result = pippolo_true, execute = pippolo_false, hashes;
    for (hashes = 0; hashes < pippolo_hash_elements; hashes++)
        if (action->range.hash[hashes].value) {
            pippolo_lock(server.mutex);
            execute = server.range.hash[hashes].value;
            pippolo_unlock(server.mutex);
            if (execute)
                break;
        }
    if (execute) {
        switch (action->kind) {
            case EDATA_ACTIONS_ADD:
                pippolo_log(ELOG_DEEP_DEBUGGING, "executing an 'ADD' action");
                result = _p_data_action_execute_add(action);
                break;
            case EDATA_ACTIONS_DELETE:
                pippolo_log(ELOG_DEEP_DEBUGGING, "executing a 'DELETE' action");
                result = _p_data_action_execute_del(action);
                break;
            case EDATA_ACTIONS_GET:
                pippolo_log(ELOG_DEEP_DEBUGGING, "executing a 'GET' action");
                result = _p_data_action_execute_get(action);
                break;
            default:
                break;
        }
        pippolo_log(ELOG_DEEP_DEBUGGING, "executed");
    }
    return result;
}

int _p_data_action_execute_add (struct str_action *action) {
    return p_data_insert(action->records);
}

int _p_data_action_execute_del (struct str_action *action) {
    return p_data_delete(action, action->records, root);
}

int _p_data_action_execute_get (struct str_action *action) {
    return p_data_match(action, action->records, root);
}

int p_data_insert (struct str_xml_node *hook) {
    int result = pippolo_true, hash = (pippolo_hash_elements-1) /* default value */, insert;
    struct str_xml_node *node, *primary;
    struct str_xml_key *pointer, *key;
    struct str_key *record, *value;
    time_t timestamp = time(NULL);
    while (hook) {
        if (p_string_cmp(hook->label, pippolo_xml_record) == 0) {
            insert = pippolo_false;
            pointer = hook->keys;
            while (pointer) {
                if (p_string_cmp(pointer->key, pippolo_xml_key_hash) == 0) {
                    hash = p_string_atoi_unsigned(pointer->value) % pippolo_hash_elements; /* safe modulation */
                    pippolo_lock(server.mutex);
                    insert = server.range.hash[hash].value;
                    pippolo_unlock(server.mutex);
                } else if (p_string_cmp(pointer->key, pippolo_xml_key_time) == 0)
                    timestamp = p_string_atoi_unsigned(pointer->value);
                pointer = pointer->next;
            }
            if (insert) {
                record = NULL;
                primary = NULL;
                node = hook->children;
                while (node) {
                    if (p_string_cmp(node->label, pippolo_xml_value) == 0) {
                        pointer = key = node->keys;
                        while (pointer) {
                            if (p_string_cmp(pointer->key, pippolo_xml_key_key) == 0)
                                key = pointer;
                            else if ((p_string_cmp(pointer->key, pippolo_xml_key_primary) == 0) &&
                                        (p_string_cmp(pointer->value, pippolo_xml_value_true) == 0))
                                primary = node;
                            pointer = pointer->next;
                        }
                        if (key) {
                            value = NULL;
                            if ((result = _p_data_key_create(&value, key->value, node->value, (primary==node))))
                                result = _p_data_key_append(&record, value);
                        }
                    }
                    if (!result)
                        break;
                    node = node->next;
                }
                if ((result) && (record) && (primary)) {
                    if ((p_data_pruning(root, primary->value, timestamp)))
                        result = p_data_level_append(&root, hash, timestamp, record);
                    else
                        p_data_free_keys(&record);
                } else
                    result = pippolo_false;
            }
        }
        if (!result)
            break;
        hook = hook->next;
    }
    return result;
}

int p_data_pruning (struct str_level *root, const char *primary, time_t timestamp) {
    int result = pippolo_true, founded = pippolo_false;
    struct str_record *singleton, *backup;
    while (root) {
        pippolo_lock(root->mutex);
        if ((singleton = root->values)) {
            if ((founded = _p_data_pruning(singleton->keys, primary))) {
                pippolo_log(ELOG_DEEP_DEBUGGING, "evalutating stored record timestamp (%ld) with new entry timestamp (%ld)", singleton->time, timestamp);
                if (timestamp > singleton->time) { /* not if equal, only with a bigger timestamp */
                    root->values = singleton->next;
                    p_data_free_keys(&(singleton->keys));
                    pippolo_null_free(singleton);
                } else
                    result = pippolo_false;
            } else
                while ((backup = singleton->next)) {
                    if ((founded = _p_data_pruning(backup->keys, primary))) {
                        pippolo_log(ELOG_DEEP_DEBUGGING, "evalutating stored record timestamp (%ld) with new entry timestamp (%ld)", backup->time, timestamp);
                        if (timestamp > backup->time) { /* not if equal, only with a bigger timestamp */
                            singleton->next = backup->next;
                            p_data_free_keys(&(backup->keys));
                            pippolo_null_free(backup);
                        } else
                            result = pippolo_false;
                        break;
                    }
                    singleton = singleton->next;
                }
        }
        pippolo_unlock(root->mutex);
        if ((founded) || (!(result = p_data_pruning(root->childrens, primary, timestamp))))
            break;
        root = root->next;
    }
    return result;
}
    
int _p_data_pruning (struct str_key *root, const char *primary) {
    int result = pippolo_false;
    while (root) {
        if (root->primary.value)
            if (p_string_cmp(root->value, primary) == 0)
                result = pippolo_true;
        if (result)
            break;
        root = root->next;
    }
    return result;
}

int p_data_delete (struct str_action *action, struct str_xml_node *nodes, struct str_level *hook) { /* partially avoiding recursion */
    int result = pippolo_true;
    if (nodes)
        while (hook) {
            pippolo_lock(hook->mutex);
            result = _p_data_delete_level(action, nodes, &(hook->values));
            pippolo_unlock(hook->mutex);
            if (result) {
                pippolo_lock(hook->mutex);
                result = p_data_delete(action, nodes, hook->childrens);
                pippolo_unlock(hook->mutex);
            }
            if (!result)
                break;
            hook = hook->next;
        }
    return result;
}

int _p_data_delete_level (struct str_action *action, struct str_xml_node *nodes, struct str_record **value) { /* avoiding recursion */
    int result = pippolo_true, founded;
    struct str_xml_node *keys;
    struct str_record *backup, *singleton;
    while ((singleton = (*value))) {
        founded = pippolo_false;
        keys = nodes;
        while (keys) { /* key is record that contain the regex */
            if (p_string_cmp(keys->label, pippolo_xml_record) == 0)
                if (_p_data_match_record(keys->children, singleton->keys)) {
                    founded = pippolo_true;
                    break;
                }
            keys = keys->next;
        }
        if (founded) {
            *value = singleton->next;
            p_data_free_keys(&(singleton->keys));
            pippolo_null_free(singleton);
        } else
            break;
    }
    singleton = (*value);
    while ((singleton) && (backup = singleton->next)) {
        founded = pippolo_false;
        keys = nodes;
        while (keys) { /* key is record that contain the regex */
            if (p_string_cmp(keys->label, pippolo_xml_record) == 0)
                if (_p_data_match_record(keys->children, backup->keys)) {
                    founded = pippolo_true;
                    break;
                }
            keys = keys->next;
        }
        if (founded) {
            singleton->next = backup->next;
            p_data_free_keys(&(backup->keys));
            pippolo_null_free(backup);
        } else
            singleton = singleton->next;
    }
    return result;
}

int p_data_match (struct str_action *action, struct str_xml_node *nodes, struct str_level *hook) { /* partially avoiding recursion */
    int result = pippolo_true;
    while (hook) {
        pippolo_lock(hook->mutex);
        result = _p_data_match_level(action, nodes, hook->values);
        pippolo_unlock(hook->mutex);
        if (result) {
            pippolo_lock(hook->mutex);
            result = p_data_match(action, nodes, hook->childrens);
            pippolo_unlock(hook->mutex);
        }
        if (!result)
            break;
        hook = hook->next;
    }
    return result;
}

int _p_data_match_level (struct str_action *action, struct str_xml_node *nodes, struct str_record *values) {
    int result = pippolo_true, insert;
    struct str_xml_node *hook = NULL, *keys;
    while (values) {
        if (action->range.hash[values->hash].value) {
            insert = pippolo_true; /* without criteria the default is true */
            keys = nodes;
            while (keys) { /* key is record that contain the regex */
                insert = pippolo_false;
                if (p_string_cmp(keys->label, pippolo_xml_record) == 0)
                    if (_p_data_match_record(keys->children, values->keys)) {
                        insert = pippolo_true;
                        break;
                    }
                keys = keys->next;
            }
            if (insert)
                if ((result = _p_data_match_conversion(&hook, values->hash, values->time, values->keys)))
                    result = p_data_reply_insert(action, hook);
            if (!result)
                break;
        }
        values = values->next;
    }
    return result;
}

int _p_data_match_record (struct str_xml_node *nodes, struct str_key *values) {
    int result = pippolo_true;
    struct str_key *singleton;
    struct str_xml_key *key;
    if (nodes) {
        key = nodes->keys;
        while (key) {
            if (p_string_cmp(key->key, pippolo_xml_key_key) == 0) {
                result = pippolo_false;
                singleton = values;
                while (singleton) {
                    if (p_string_case_cmp(singleton->key, key->value) == 0) {
                        if (!nodes->compiled.value)
                            if (regcomp(&(nodes->regex), nodes->value, 0) == 0)
                                nodes->compiled.value = pippolo_true;
                        if (nodes->compiled.value)
                            if (regexec(&(nodes->regex), singleton->value, 0, NULL, 0) == 0)
                                result = pippolo_true;
                        break;
                    }
                    singleton = singleton->next;
                }
                if (!result)
                    break;
            }
            key = key->next;
        }
        if (result)
            result = _p_data_match_record(nodes->next, values);
    }
    return result;
}

int _p_data_match_conversion (struct str_xml_node **hook, unsigned int hash, time_t timestamp, struct str_key *values) {
    int result = pippolo_true;
    char value[pippolo_default_size];
    if (values)
        if ((result = p_xml_node_allocate(hook))) {
            pippolo_append((*hook)->label, pippolo_xml_record);
            snprintf(value, pippolo_default_size, "%ld", timestamp);
            if ((result = p_xml_key_append(&((*hook)->keys), pippolo_xml_key_time, value))) {
                snprintf(value, pippolo_default_size, "%d", hash);
                if ((result = p_xml_key_append(&((*hook)->keys), pippolo_xml_key_hash, value)))
                    result = _p_data_match_conversion_keys(&((*hook)->children), (*hook), values);
            }
        }
    return result;
}

int _p_data_match_conversion_keys (struct str_xml_node **hook, struct str_xml_node *father, struct str_key *values) {
    int result = pippolo_true;
    if (values)
        if ((result = p_xml_node_allocate(hook))) {
            (*hook)->father = father;            
            pippolo_append((*hook)->label, pippolo_xml_value);
            pippolo_append((*hook)->value, values->value);
            if ((result = p_xml_key_append(&((*hook)->keys), pippolo_xml_key_key, values->key)))
                if (values->primary.value)
                    result = p_xml_key_append(&((*hook)->keys), pippolo_xml_key_primary, pippolo_xml_value_true);
            if (result)
                result = _p_data_match_conversion_keys(&((*hook)->next), father, values->next);
        }
    return result;
}

int p_data_reply_insert (struct str_action *action, struct str_xml_node *record) {
    int result = pippolo_true;
    if (action->reply) {
        if (p_string_cmp(action->reply->label, pippolo_xml_node) == 0) {
            record->father = action->reply;
            record->next = action->reply->children;
            action->reply->children = record;
        } else
            result = pippolo_false;
    } else
        result = pippolo_false;
    return result;
}

int p_data_draw (FILE *stream, struct str_level *root) {
    return _p_data_draw(stream, root, 0);
}

int _p_data_draw (FILE *stream, struct str_level *root, unsigned int level) {
    int result = pippolo_true, index;
    if (root) {
        for (index = 0; index < level; index++)
            fprintf(stream, "\t");
        fprintf(stream, "<node key=%s>\n", root->key);
        if ((result = _p_data_draw(stream, root->childrens, level+1)))
            if ((result = p_data_records_draw(stream, root->values, level+1))) {
                for (index = 0; index < level; index++)
                    fprintf(stream, "\t");
                fprintf(stream, "</node>\n");
                result = _p_data_draw(stream, root->next, level);
            }
    }
    return result;
}

int p_data_records_draw (FILE *stream, struct str_record *record, unsigned int level) {
    int result = pippolo_true;
    struct str_record *singleton = record;
    while (singleton) {
        if (!(result = p_data_keys_draw(stream, singleton->time, singleton->hash, singleton->keys, level)))
            break;
        singleton = singleton->next;
    }
    return result;
}

int p_data_keys_draw (FILE *stream, time_t timestamp, unsigned int hash, struct str_key *data, unsigned int level) {
    int result = pippolo_true, index;
    struct str_key *keys = data;
    for (index = 0; index < level; index++)
        fprintf(stream, "\t");
    fprintf(stream, "<record timestamp=%lu hash=%d>\n", timestamp, hash);
    while (keys) {
        for (index = 0; index <= level; index++)
            fprintf(stream, "\t");
        fprintf(stream, "<value");
        if (keys->primary.value)
            fprintf(stream, " primary=true");
        fprintf(stream, " key=%s>%s</value>\n", keys->key, keys->value);
        keys = keys->next;
    }
    for (index = 0; index < level; index++)
        fprintf(stream, "\t");
    fprintf(stream, "</record>\n");
    return result;
}

void p_data_free (struct str_level **root) { /* partially avoiding recursivity */
    struct str_level *support;
    while ((support = *root)) {
        *root = (*root)->next;
        if (support->childrens)
            p_data_free(&(support->childrens));
        if (support->values)
            p_data_free_records(&(support->values));
        pthread_mutex_destroy(&(support->mutex));
        pippolo_null_free(support);
    }
}

void p_data_free_records (struct str_record **root) { /* avoiding recursivity */
    struct str_record *support;
    while ((support = *root)) {
        *root = (*root)->next;
        if (support->keys)
            p_data_free_keys(&(support->keys));
        pippolo_null_free(support);
    }
}

void p_data_free_keys (struct str_key **root) { /* avoiding recursivity */
    struct str_key *support;
    while ((support = *root)) {
        *root = (*root)->next;
        if (support->value)
            pippolo_free(support->value);
        pippolo_null_free(support);
    }
}
