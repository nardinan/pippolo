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
#ifndef pippolo_memory_h
#define pippolo_memory_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "log.h"
#include "definitions.h"
#define pippolo_checksum 0xfacefeed
#define pippolo_overflow 20
#define pippolo_malloc(a) malloc(a)/*_p_malloc((a),__FILE__,__LINE__)*/
#define pippolo_realloc(a,b) realloc((a),(b))/*_p_realloc((a),(b),__FILE__,__LINE__)*/
#define pippolo_free(a) free(a)/*_p_free((a),__FILE__,__LINE__)*/
#define pippolo_null_free(a)\
    do{\
        pippolo_free(a);\
        (a)=NULL;\
    }while(0)
typedef struct str_pointer_coords {
    const char *file;
    unsigned int line;
} str_pointer_coords;
typedef struct str_pointer_node {
    void *pointer;
    size_t size;
    struct str_pointer_node *next;
} str_pointer_node;
extern struct str_pointer_node *memory;
extern size_t filled_memory;
extern pthread_mutex_t mutex_memory;
void p_memory_clean (void);
void *_p_malloc (size_t size, const char *file, unsigned int line);
void *_p_realloc (void *pointer, size_t size, const char *file, unsigned int line);
void _p_free (void *pointer, const char *file, unsigned int line);
void _p_free_check (struct str_pointer_node *singleton, const char *file, unsigned int line);
#endif
