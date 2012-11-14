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
#include "p_api_network.h"
int p_network_serverize (const char *address, const char *port, unsigned int queue) {
    struct addrinfo hints = {AI_PASSIVE, AF_INET, SOCK_STREAM}, *response = NULL;
	int result = pippolo_null_socket, value = 1;
	if (getaddrinfo(address, port, &hints, &response) == 0) {
		if ((result = socket(response->ai_family, response->ai_socktype, response->ai_protocol)) != pippolo_null_socket) {
			if (setsockopt(result, SOL_SOCKET, SO_REUSEADDR, (char *)&value, sizeof(int)) != -1) {
				if (bind(result, response->ai_addr, response->ai_addrlen) == 0) {
					if (listen(result, queue) != 0) {
                        freeaddrinfo(response);
                        result = pippolo_null_socket;
                    } else
                        setsockopt(result, SOL_SOCKET, SO_NOSIGPIPE, (char *)&value, sizeof(int));
				} else if (result != pippolo_null_socket) {
                    close(result);
                    result = pippolo_null_socket;
                }
			} else if (result != pippolo_null_socket) {
                close(result);
                result = pippolo_null_socket;
            }
		}
		freeaddrinfo(response);
	}
	return result;
}

int p_network_connect (const char *address, const char *port) {
    struct addrinfo hints = {AI_PASSIVE, AF_INET, SOCK_STREAM}, *response = NULL;
	int result = pippolo_null_socket, flags, value = 1;
	if (getaddrinfo(address, port, &hints, &response) == 0) {
		if ((result = socket(response->ai_family, response->ai_socktype, response->ai_protocol)) != pippolo_null_socket) {
            if (connect(result, response->ai_addr, response->ai_addrlen) == 0) {
                if ((flags = fcntl(result, F_GETFL)) != -1)
                    fcntl(result, F_SETFL, flags|O_NONBLOCK);
                setsockopt(result, SOL_SOCKET, SO_NOSIGPIPE, (char *)&value, sizeof(int));
            } else if (result != pippolo_null_socket) {
                close(result);
                result = pippolo_null_socket;
            }
		}
		freeaddrinfo(response);
	}
	return result;
} 

int p_network_accept (int hook) {
    fd_set rdset;
    int flags, result = pippolo_null_socket;
    FD_ZERO(&rdset);
	FD_SET(hook,&rdset);
    if ((result = accept(hook, NULL, NULL)) != pippolo_null_socket)
        if ((flags = fcntl(result, F_GETFL)) != -1)
            fcntl(result, F_SETFL, flags|O_NONBLOCK);
    return result;
}

int p_network_receive (int hook, char **buffer, int sec, int usec) {
    char *pointer;
    fd_set rdset;
    struct timeval timeout = {sec, usec};
    int descriptors, length, result = pippolo_true;
    ssize_t readed = pippolo_again;
    if (hook != pippolo_null_socket) {
        FD_ZERO(&rdset);
        FD_SET(hook,&rdset);
        if ((descriptors = select(hook+1, &rdset, NULL, NULL, &timeout)) > 0) {
            if ((pippolo_intro == read(hook, &length, pippolo_intro))) {
                if (_p_endian_check() != pippolo_default_endian)
                    pippolo_swap(length);
                if (((*buffer) = (char *) pippolo_malloc(length+1))) {
                    memset((*buffer), '\0', (length+1));
                    pointer = (*buffer);
                    while ((length > 0) && (readed == pippolo_again)) {
                        timeout.tv_sec = sec;
                        timeout.tv_usec = usec;
                        if ((descriptors = select(hook+1, &rdset, NULL, NULL, &timeout)) > 0) {
                            while ((readed = read(hook, pointer, length)) > 0) {
                                length -= readed;
                                pointer += readed;
                            }
                        } else {
                            if (descriptors < 0)
                                result = pippolo_false;
                            break;
                        }
                    }
                    if (length)
                        result = pippolo_false;
                } else
                    result = pippolo_false;
            } else
                result = pippolo_false;
        } else if (descriptors < 0)
            result = pippolo_false;
    } else
        result = pippolo_false;
    return result;
}

size_t p_network_write (int hook, char *buffer) {
    size_t result = 0, size = p_string_len(buffer);
    int length = (int)size;
    if (hook != pippolo_null_socket) {
        if (_p_endian_check() != pippolo_default_endian)
            pippolo_swap(length);
        if ((pippolo_intro == write(hook, &length, pippolo_intro)))
            result = write(hook, buffer, size);
    }
    return result;
}
