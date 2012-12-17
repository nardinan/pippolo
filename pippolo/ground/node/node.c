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
#include "node.h"
char p_channel_description[][pippolo_channel_length] = {
    "main channel",
    "service channel"
};
pthread_mutex_t mutex_nodes;
struct pippolo_bool alives[pippolo_max_neighbours];
struct str_node neighbours[pippolo_max_neighbours], server;
void p_node_clean (void) {
    int nodes, channels, hashes;
    pthread_mutex_init(&mutex_nodes, NULL);
    for (nodes = 0; nodes < pippolo_max_neighbours; nodes++) {
        alives[nodes].value = pippolo_false;
        pthread_mutex_init(&neighbours[nodes].mutex, NULL);
        memset(neighbours[nodes].token, '\0', (pippolo_token+1));
        memset(neighbours[nodes].address, '\0', pippolo_address_size);
        neighbours[nodes].initialized.value = neighbours[nodes].onetime.value = pippolo_false;
        neighbours[nodes].timeout.sec = neighbours[nodes].timeout.usec = 0;
        for (channels = 0; channels < ECHANNELS_NULL; channels++) {
            memset(neighbours[nodes].connections[channels].port, '\0', pippolo_port_size);
            neighbours[nodes].connections[channels].hook = pippolo_null_socket;
            neighbours[nodes].connections[channels].established.value = pippolo_false;
        }
        for (hashes = 0; hashes < pippolo_hash_elements; hashes++)
            neighbours[nodes].range.hash[hashes].value = pippolo_false;
    }
    pthread_mutex_init(&server.mutex, NULL);
    memset(server.address, '\0', pippolo_address_size);
    for (nodes = 0; nodes < ECHANNELS_NULL; nodes++) {
        memset(neighbours[nodes].connections[channels].port, '\0', pippolo_port_size);
        server.connections[nodes].hook = pippolo_null_socket;
        server.connections[nodes].established.value = pippolo_false;
    }
    for (hashes = 0; hashes < pippolo_hash_elements; hashes++)
        server.range.hash[hashes].value = pippolo_false;
}

int p_node_setup (FILE *stream) {
    char singleton[pippolo_configuration_line], *ptr = NULL;
    int neighbour = pippolo_false, result = pippolo_true, index = -1;
    memset(singleton, '\0', pippolo_configuration_line);
    while ((fgets(singleton, pippolo_configuration_line, stream))) {
        p_string_trim(singleton);
        if (strncasecmp(singleton, pippolo_len("@neighbour")) == 0) {
            index++;
            if (index < pippolo_max_neighbours)
                neighbour = pippolo_true;
            else {
                neighbour = pippolo_false;
                pippolo_log(ELOG_TALKING, "there is space for at most %d friends; sorry but I have to ignore Pippolo %d", pippolo_max_neighbours, (index+1));
            }
        } else if (neighbour) {
            if ((ptr = strchr(singleton, ':'))) {
                p_string_trim(++ptr);
                p_node_configure(index, singleton, ptr);
            } else
                pippolo_log(ELOG_TALKING, "ehy master, that's embaressing for you but %s is an incorrect key:value", singleton);
        } else {
            if ((ptr = strchr(singleton, ':'))) {
                p_string_trim(++ptr);
                p_server_configure(singleton, ptr);
            } else
                pippolo_log(ELOG_TALKING, "ehy master, that's embaressing for you but %s is an incorrect key:value", singleton);
        }
        memset(singleton, '\0', p_string_len(singleton));
        if (!result)
            break;
    }
    /* before check start we need to search informations from out neighbours */
    p_node_synchronize();
    /* end */
    if (p_string_len(server.address) > 0) {
        if (p_string_len(server.token) == pippolo_token) {
            pippolo_log(ELOG_TALKING, "hello, I'm Pippolo %s (version %s|log level %s) and I can't wait to meet new friends!", pippolo_safe_token(server.token), pippolo_version, log_description[pippolo_log_level]);
            p_server_check_start();
            p_node_check_start();
        } else {
            pippolo_log(ELOG_TALKING, "'%s' is an invalid name for a Pippolo, you can't give me that name!", pippolo_safe_token(server.token));
            result = pippolo_false;
        }
    } else {
        pippolo_log(ELOG_TALKING, "ehy master, you forgot the server's address! I can't start myself");
        result = pippolo_false;
    }
    return result;
}

void p_node_configure (unsigned int index, const char *key, const char *value) {
    if (strncasecmp(key, pippolo_len("address")) == 0)
        snprintf(neighbours[index].address, pippolo_address_size, "%s", value);
    else if (strncasecmp(key, pippolo_len("mport")) == 0)
        snprintf(neighbours[index].connections[ECHANNELS_MAIN].port, pippolo_port_size, "%s", value);
    else if (strncasecmp(key, pippolo_len("sport")) == 0)
        snprintf(neighbours[index].connections[ECHANNELS_SERVICE].port, pippolo_port_size, "%s", value);
    else if (strncasecmp(key, pippolo_len("timeout.sec")) == 0)
        neighbours[index].timeout.sec = p_string_atoi_unsigned(value);
    else if (strncasecmp(key, pippolo_len("timeout.usec")) == 0)
        neighbours[index].timeout.usec = p_string_atoi_unsigned(value);
    else if (strncasecmp(key, pippolo_len("refresh.sec")) == 0)
        neighbours[index].refresh.sec = p_string_atoi_unsigned(value);
    else if (strncasecmp(key, pippolo_len("refresh.usec")) == 0)
        neighbours[index].refresh.usec = p_string_atoi_unsigned(value);
}

void p_server_configure (const char *key, const char *value) {
    char *pointer;
    if (strncasecmp(key, pippolo_len("address")) == 0)
        snprintf(server.address, pippolo_address_size, "%s", value);
    else if (strncasecmp(key, pippolo_len("mport")) == 0)
        snprintf(server.connections[ECHANNELS_MAIN].port, pippolo_port_size, "%s", value);
    else if (strncasecmp(key, pippolo_len("sport")) == 0)
        snprintf(server.connections[ECHANNELS_SERVICE].port, pippolo_port_size, "%s", value);
    else if (strncasecmp(key, pippolo_len("timeout.sec")) == 0)
        server.timeout.sec = p_string_atoi_unsigned(value);
    else if (strncasecmp(key, pippolo_len("timeout.usec")) == 0)
        server.timeout.usec = p_string_atoi_unsigned(value);
    else if (strncasecmp(key, pippolo_len("refresh.sec")) == 0)
        server.refresh.sec = p_string_atoi_unsigned(value);
    else if (strncasecmp(key, pippolo_len("refresh.usec")) == 0)
        server.refresh.usec = p_string_atoi_unsigned(value);
    else if (strncasecmp(key, pippolo_len("token")) == 0)
        snprintf(server.token, (pippolo_token+1), "%s", value);
    else if (strncasecmp(key, pippolo_len("hash")) == 0) {
        pointer = (char *)value;
        while (*pointer) {
            server.range.hash[(*pointer)-48].value = pippolo_true;
            pointer++;
        }
    }
}

void p_node_synchronize (void) {
    int index, indentify = rand();
    for (index = 0; index < pippolo_max_neighbours; index++)
        if (p_string_len(neighbours[index].address) > 0)
            _p_node_synchronize(index, indentify);
        else
            break;
}

void _p_node_synchronize (unsigned int index, unsigned int identify) {
    int hashes, hook;
    char request[pippolo_default_size], hash[(pippolo_hash_elements+1)], *pointer, *token = NULL, *buffer = NULL;
    struct str_token_node *tokens;
    struct str_xml_node *node = NULL;
    memset(request, '\0', pippolo_default_size);
    memset(hash, '\0', (pippolo_hash_elements+1));
    pointer = hash;
    for (hashes = 0; hashes < pippolo_hash_elements; hashes++)
        if (server.range.hash[hashes].value) {
            *pointer = (char)(48+hashes);
            pointer++;
        }
    if ((hook = p_network_connect(neighbours[index].address, neighbours[index].connections[ECHANNELS_MAIN].port)) != pippolo_null_socket) {
        if ((p_node_handshake_client(hook, &token, server.timeout.sec, server.timeout.usec))) {
            snprintf(request, pippolo_default_size, "<%s %s=%s %s=G %s=%d %s=10></%s>", pippolo_xml_node, pippolo_xml_key_hash, hash, pippolo_xml_key_action, pippolo_xml_key_token, identify, pippolo_xml_key_live, pippolo_xml_node);
            if (p_network_write(hook, request))
                while (pippolo_true)
                    if (p_network_receive(hook, &buffer, neighbours[index].refresh.sec, neighbours[index].refresh.usec)) {
                        if (buffer) {
                            if ((tokens = p_xml_tokenizer(buffer))) {
                                if ((p_xml_analyze(&node, tokens))) {
                                    p_data_insert(node->children);
                                    p_xml_free(&node);
                                }
                                p_tokens_free(&tokens);
                            }
                            pippolo_null_free(buffer);
                            break;
                        }
                    }
            pippolo_null_free(token);
        }
        shutdown(hook, SHUT_RDWR);
        close(hook);
    }
}

void p_node_check_start (void) {
    int index;
    for (index = 0; index < pippolo_max_neighbours; index++)
        if (p_string_len(neighbours[index].address) > 0) {
            if (!neighbours[index].initialized.value)
                _p_node_check_start(index);
        } else
            break; /* interrupt to the first empty */
}

void _p_node_check_start (unsigned int index) { /* thread unsafe (we have to use an uninitialized index) */
    struct str_parameter *parameter;
    pthread_t link;
    pthread_attr_t attributes;
    int channel;
    for (channel = 0; channel < ECHANNELS_NULL; channel++) {
        if ((parameter = (str_parameter *) pippolo_malloc(sizeof(str_parameter)))) {
            parameter->index = index;
            parameter->channel = channel;
            pthread_mutex_init(&neighbours[index].connections[channel].mutex, NULL);
            pthread_attr_init(&attributes);
            if (pthread_create(&link, &attributes, &p_node_thread_check, (void *)parameter) == 0)
                pthread_detach(link);
            else
                pippolo_log(ELOG_TALKING, "I'm very sorry but I think there are some technical difficulties on 'threads' and stuff with Pippolo %s on %s", pippolo_safe_token(neighbours[index].token), p_channel_description[channel]);
        } else
            pippolo_kill("out of memory, I'm dying bleaaargh");
    }
    neighbours[index].initialized.value = pippolo_true;
}

void p_server_check_start (void) {
    if (p_string_len(server.address) > 0)
        if (!server.initialized.value)
            _p_server_check_start();
}

void _p_server_check_start (void) {
    struct str_parameter *parameter;
    pthread_t link;
    pthread_attr_t attributes;
    int channel;
    for (channel = 0; channel < ECHANNELS_NULL; channel++)
        if ((server.connections[channel].hook = p_network_serverize(((p_string_cmp(server.address, "127.0.0.1") == 0)?NULL:server.address), server.connections[channel].port, pippolo_queue)) != pippolo_null_socket) {
            if ((parameter = (struct str_parameter *) pippolo_malloc(sizeof(str_parameter)))) {
                parameter->index = pippolo_null_socket;
                parameter->channel = channel;
                pthread_mutex_init(&server.connections[channel].mutex, NULL);
                pthread_attr_init(&attributes);
                server.initialized.value = pippolo_true;
                if (pthread_create(&link, &attributes, &p_server_thread_check, (void *)parameter) == 0)
                    pthread_detach(link);
                else
                    pippolo_log(ELOG_TALKING, "I'm very sorry but I think I've some technical difficulties on 'threads' and stuff on %s", p_channel_description[channel]);
            } else
                pippolo_kill("out of memory, I'm dying bleaaargh");
        } else
            pippolo_log(ELOG_TALKING, "oh God, God, God, God ... I'm completely deaf on %s!", p_channel_description[channel]);
    server.initialized.value = pippolo_true;
}

void *p_node_thread_check (void *parameters) {
    struct str_parameter *value = (str_parameter *)parameters;
    int result = pippolo_true, sec, usec, alive;
    while (result) {
        alive = pippolo_false;
        pippolo_lock(neighbours[value->index].connections[value->channel].mutex);
        alive = neighbours[value->index].connections[value->channel].established.value;
        pippolo_unlock(neighbours[value->index].connections[value->channel].mutex);
        if (value->channel == ECHANNELS_MAIN) {
            pippolo_lock(mutex_nodes);
            alives[value->index].value = alive;
            pippolo_unlock(mutex_nodes);
        }
        if (alive) {
            if (_p_node_thread_check_read(value))
                if (value->channel == ECHANNELS_MAIN)
                    _p_node_thread_check_action(value);
        } else {
            pippolo_lock(neighbours[value->index].mutex);
            sec = neighbours[value->index].refresh.sec;
            usec = neighbours[value->index].refresh.usec;
            pippolo_unlock(neighbours[value->index].mutex);
            p_wait(sec, usec);
            p_node_initialize(value->index, value->channel);
        }
    }
    pippolo_free(value);
    pthread_exit(NULL);
    return NULL;
}

int _p_node_thread_check_read (str_parameter *value) {
    int result = pippolo_true;
    char *incoming = NULL;
    pippolo_lock(neighbours[value->index].connections[value->channel].mutex);
    result = p_network_receive(neighbours[value->index].connections[value->channel].hook, &incoming, neighbours[value->index].refresh.sec, neighbours[value->index].refresh.usec);
    pippolo_unlock(neighbours[value->index].connections[value->channel].mutex);
    if ((result) && (incoming)) {
        if (p_string_len(incoming) > 0) {
            if ((p_string_len(incoming) == pippolo_ack_len) && (p_string_cmp(incoming, pippolo_ack_request) == 0)) {
                pippolo_lock(neighbours[value->index].connections[value->channel].mutex);
                p_network_write(neighbours[value->index].connections[value->channel].hook, pippolo_ack_response);
                pippolo_unlock(neighbours[value->index].connections[value->channel].mutex);
            } else if (p_string_cmp(incoming, pippolo_ack_response) != 0)
                if (!p_data_action(incoming, value->index))
                    pippolo_log(ELOG_TALKING, "uhm, can't analyze request '%s' from Pippolo %s. Maybe a different version?", incoming, neighbours[value->index].token);
        } else
            result = pippolo_false;
        pippolo_null_free(incoming);
    }
    if (!result) {
        pippolo_log(ELOG_TALKING, "I think my friend Pippolo %s is sick, so I kick him away from %s!", pippolo_safe_token(neighbours[value->index].token), p_channel_description[value->channel]);
        _p_node_thread_check_shutdown(value->index, value->channel);
    }
    return result;
}

void _p_node_thread_check_action (str_parameter *value) {
    struct str_action *current;
    char *message = NULL;
    pippolo_lock(mutex_actions);
    current = actions;
    pippolo_unlock(mutex_actions);
    while (current) {
        if (pippolo_trylock(current->mutex) == 0) { /* try to lock, otherwise we can jump it to execute another one */
            /* for the action management we need an atomic process */
            if (!current->terminated.value) {
                if (!(current->executed[value->index].value)) {
                    if (current->owner == value->index) {
                        if ((current->hooks[EHOOK_ACTIONS_FORWARD] == 0) || ((current->last+current->live) < time(NULL))) {
                            pippolo_log(ELOG_DEBUGGING, "awesome! There is an action that I can execute! (%s)", (current->hooks[EHOOK_ACTIONS_FORWARD]>0)?"it's for timeout":"every token has done his job");
                            _p_data_action_execute(current);
                            current->last = time(NULL);
                            current->executed[value->index].value = pippolo_true;
                        }
                    } else {
                        if (current->forward) {
                            if ((p_xml_string(&message, current->forward)) && (message)) {
                                pippolo_lock(neighbours[value->index].connections[value->channel].mutex);
                                p_network_write(neighbours[value->index].connections[value->channel].hook, message);
                                pippolo_unlock(neighbours[value->index].connections[value->channel].mutex);
                                pippolo_null_free(message);
                            }
                        } else {
                            /* we don't need to waiting for the reply */
                            current->completed[value->index].value = pippolo_true;
                            current->hooks[EHOOK_ACTIONS_REPLY] = p_max((current->hooks[EHOOK_ACTIONS_REPLY]-1), 0);
                        }
                        current->last = time(NULL);
                        current->executed[value->index].value = pippolo_true;
                        current->hooks[EHOOK_ACTIONS_FORWARD] = p_max((current->hooks[EHOOK_ACTIONS_FORWARD]-1), 0);
                    }
                } else if (current->owner == value->index)
                    if ((current->hooks[EHOOK_ACTIONS_REPLY] == 0) || ((current->last+current->live) < time(NULL))) {
                        if (current->reply)
                            pippolo_log(ELOG_DEBUGGING, "I think it's time to give a reply to my dear friend Pippolo %s for action %s with remaining hooks %d (%s)", pippolo_safe_token(neighbours[value->index].token), current->token, current->hooks[EHOOK_ACTIONS_REPLY], (current->hooks[EHOOK_ACTIONS_REPLY]>0)?"it's for timeout":"every token has done his job");
                            if ((p_xml_string(&message, current->reply)) && (message)) {
                                pippolo_lock(neighbours[value->index].connections[value->channel].mutex);
                                p_network_write(neighbours[value->index].connections[value->channel].hook, message);
                                pippolo_unlock(neighbours[value->index].connections[value->channel].mutex);
                                pippolo_null_free(message);
                            }
                        current->terminated.value = pippolo_true;
                        _p_node_thread_check_action_flush(current); /* the function is thread usafe, but here we ave current->mutex locked */
                    }
            }
            pippolo_unlock(current->mutex);
        }
        current = current->next;
    }
}

void _p_node_thread_check_action_flush (struct str_action *action) {
    if (action->reply)
        p_xml_free(&(action->reply));
    action->reply = NULL;
    if (action->original)
        p_xml_free(&(action->original));
    action->original = NULL;
    action->forward = NULL;
    action->records = NULL;
}

void _p_node_thread_check_shutdown (int index, enum enum_channel channel) {
    struct str_action *current;
    pippolo_lock(neighbours[index].connections[channel].mutex);
    shutdown(neighbours[index].connections[channel].hook, SHUT_RDWR);
    close(neighbours[index].connections[channel].hook);
    neighbours[index].connections[channel].hook = pippolo_null_socket;
    neighbours[index].connections[channel].established.value = pippolo_false;
    pippolo_unlock(neighbours[index].connections[channel].mutex);
    /* update every action in the bucket, removing himself (trying to avoid timeout) */
    if (channel == ECHANNELS_MAIN) {
        pippolo_lock(mutex_actions);
        current = actions;
        pippolo_unlock(mutex_actions);
        while (current) {
            pippolo_lock(current->mutex);
            if (!current->terminated.value)
                if (current->owner != index) {
                    if (!current->executed[index].value) {
                        current->executed[index].value = pippolo_true;
                        current->hooks[EHOOK_ACTIONS_FORWARD] = p_max((current->hooks[EHOOK_ACTIONS_FORWARD]-1), 0);
                    }
                    if (!current->completed[index].value) {
                        current->completed[index].value = pippolo_true;
                        current->hooks[EHOOK_ACTIONS_REPLY] = p_max((current->hooks[EHOOK_ACTIONS_REPLY]-1),0);
                    }
                }
            pippolo_unlock(current->mutex);
            current = current->next;
        }
    }
}

void *p_server_thread_check (void *parameters) {
    int result = pippolo_true, handshake, hook, assigned;
    struct str_parameter *value = (struct str_parameter *)parameters;
    char *token = NULL;
    while (result) {
        pippolo_lock(server.connections[value->channel].mutex);
        hook = p_network_accept(server.connections[value->channel].hook);
        pippolo_unlock(server.connections[value->channel].mutex);
        if (hook != pippolo_null_socket) {
            assigned = pippolo_false;
            pippolo_lock(server.connections[value->channel].mutex);
            handshake = p_node_handshake_server(hook, &token, server.timeout.sec, server.timeout.usec); /* the function is thread unsafe so we have to lock mutex here */
            pippolo_unlock(server.connections[value->channel].mutex);
            if (handshake) {
                if (p_string_cmp(token, server.token) != 0)
                    assigned = p_node_alloc(hook, value->channel, token);
                else
                    pippolo_log(ELOG_TALKING, "oh gosh, I'm trying to talk with myself or a clone of mine! That sounds like 2001 Space Odyssey!");
                if (token)
                    pippolo_free(token);
            } else
                pippolo_log(ELOG_DEEP_DEBUGGING, "uhm, a Pippolo try to get in but he didn't tell me his name in %d secs (I think he isn't a friend of mine)", server.timeout.sec);
            if (!assigned) {
                shutdown(hook, SHUT_RDWR);
                close(hook);
            }
        }
    }
    pippolo_free(value);
    pthread_exit(NULL);
    return NULL;
}

int p_node_handshake_client (int hook, char **token, int sec, int usec) {
    int result = pippolo_true;
    if (p_network_write(hook, server.token) > 0) {
        if ((p_network_receive(hook, token, sec, usec))) {
            if (p_string_len(*token) != pippolo_token) {
                pippolo_log(ELOG_TALKING, "'%s' is an invalid name for a Pippolo (I think he isn't a friend of mine)", pippolo_safe_token(*token));
                result = pippolo_false;
            }
        } else
            result = pippolo_false;
    } else
        result = pippolo_false;
    return result;
}

int p_node_handshake_server (int hook, char **token, int sec, int usec) {
    int result = pippolo_true;
    if ((p_network_receive(hook, token, sec, usec))) {
        if (p_string_len(*token) == pippolo_token) {
            if (p_network_write(hook, server.token) <= 0)
                result = pippolo_false;
        } else
            result = pippolo_false;
    } else
        result = pippolo_false;
    return result;
}

int p_node_initialize (unsigned int index, enum enum_channel channel) {
    int result = pippolo_true, alive, initializable, hook;
    char *token = NULL;
    pippolo_lock(neighbours[index].connections[channel].mutex);
    alive = neighbours[index].connections[channel].established.value;
    pippolo_unlock(neighbours[index].connections[channel].mutex);
    if (!alive) {
        initializable = pippolo_false;
        pippolo_lock(neighbours[index].mutex);
        initializable = (p_string_len(neighbours[index].address) > 0)?pippolo_true:pippolo_false;
        pippolo_unlock(neighbours[index].mutex);
        if (initializable) {
            pippolo_lock(neighbours[index].connections[channel].mutex);
            neighbours[index].connections[channel].hook = hook = p_network_connect(neighbours[index].address, neighbours[index].connections[channel].port);
            pippolo_unlock(neighbours[index].connections[channel].mutex);
            if (hook != pippolo_null_socket) {
                pippolo_lock(neighbours[index].connections[channel].mutex);
                result = p_node_handshake_client(neighbours[index].connections[channel].hook, &token, server.timeout.sec, server.timeout.usec);  /* the function is thread unsafe so we have to lock mutex here */
                pippolo_unlock(neighbours[index].connections[channel].mutex);
                if (result) {
                    snprintf(neighbours[index].token, (pippolo_token+1), "%s", token);
                    neighbours[index].connections[channel].established.value = pippolo_true;
                } else
                    _p_node_thread_check_shutdown(index, channel);
                if (token)
                    pippolo_free(token);
            } else
                result = pippolo_false;
            
        } else
            result = pippolo_false;
    }
    return result;
}

int p_node_alloc (int hook, enum enum_channel channel, char *token) {
    int index, assigned, founded;
    assigned = founded = pippolo_false;
    for (index = 0; index < pippolo_max_neighbours; index++) {
        pippolo_lock(neighbours[index].mutex);
        if (neighbours[index].initialized.value) {
            if (p_string_cmp(token, neighbours[index].token) == 0) {
                pippolo_lock(neighbours[index].connections[channel].mutex);
                if (!neighbours[index].connections[channel].established.value) {
                    neighbours[index].connections[channel].hook = hook;
                    neighbours[index].connections[channel].established.value = pippolo_true;
                    pippolo_log(ELOG_TALKING, "hey Pippolo %s, welcome back! I miss you so much!", pippolo_safe_token(token));
                    assigned = pippolo_true;
                } else
                    pippolo_log(ELOG_TALKING, "hey Pippolo %s, we are already in communication! Are you mad bro? ", pippolo_safe_token(token));
                pippolo_unlock(neighbours[index].connections[channel].mutex);
                founded = pippolo_true;
                /* already running thread */
            }
        } else {
            snprintf(neighbours[index].token, (pippolo_token+1), "%s", token); /* marking it */
            /* we don't need to lock (nobody can use this pippolo, we have to initialize it) */
            neighbours[index].refresh.sec = server.refresh.sec;
            neighbours[index].refresh.usec = server.refresh.usec;
            neighbours[index].timeout.sec = server.timeout.sec;
            neighbours[index].timeout.usec = server.timeout.usec;
            neighbours[index].connections[channel].hook = hook;
            neighbours[index].connections[channel].established.value = pippolo_true;
            _p_node_check_start(index);
            pippolo_log(ELOG_TALKING, "who we have here? My friend Pippolo %s! Welcome! This is your firs time here, right? I love you!", pippolo_safe_token(token));
            assigned = founded = pippolo_true;
        }
        pippolo_unlock(neighbours[index].mutex);
        if (founded)
            break;
    }
    return assigned;
}
