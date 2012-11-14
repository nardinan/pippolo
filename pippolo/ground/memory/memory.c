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
#include "memory.h"
struct str_pointer_node *memory = NULL;
size_t filled_memory = 0;
pthread_mutex_t mutex_memory;
void p_memory_clean (void) {
    pthread_mutex_init(&mutex_memory, NULL);
}

void *_p_malloc (size_t size, const char *file, unsigned int line) {
    struct str_pointer_coords *coords = NULL;
    struct str_pointer_node *singleton = NULL;
    void *result = NULL;
    unsigned char *checksum = NULL;
    size_t length = (size+sizeof(str_pointer_coords))+pippolo_overflow;
    if ((result = malloc(length))) {
        checksum = (unsigned char *)((char *)result+size);
        coords = (str_pointer_coords *)(checksum+pippolo_overflow);
        coords->file = file;
        coords->line = line;
        memset(result, 0, size);
        *((unsigned int *)checksum) = pippolo_checksum;
        if ((singleton = (struct str_pointer_node *) malloc(sizeof(str_pointer_node)))) {
            singleton->pointer = result;
            singleton->size = size;
            pippolo_lock(mutex_memory);
            singleton->next = memory;
            memory = singleton;
            filled_memory += length;
            pippolo_unlock(mutex_memory);
        } else
            pippolo_log(ELOG_DEBUGGING, "memory in overallocation at %s (line %d)\n", file, line);
    } else
        pippolo_log(ELOG_DEBUGGING, "memory in overallocation at %s (line %d)\n", file, line);
    return result;
}

void *_p_realloc (void *pointer, size_t size, const char *file, unsigned int line) {
    struct str_pointer_node *singleton = memory;
    void *result = NULL;
    if ((result = _p_malloc(size, file, line))) {
        pippolo_lock(mutex_memory);
        singleton = memory;
        while (singleton) {
            if (singleton->pointer == pointer) {
                memcpy(result, pointer, p_min(singleton->size, size));
                pippolo_unlock(mutex_memory);
                _p_free(pointer, file, line);
                pippolo_lock(mutex_memory);
                break;
            }
            singleton = singleton->next;
        }
        pippolo_unlock(mutex_memory);
    }
    return result;
}

void _p_free (void *pointer, const char *file, unsigned int line) {
    struct str_pointer_node *singleton = memory, *backup;
    pippolo_bool done;
    done.value = pippolo_false;
    pippolo_lock(mutex_memory);
    if (memory) {
        if (singleton->pointer == pointer) {
            memory = singleton->next;
            _p_free_check(singleton, file, line);
            done.value = pippolo_true;
        } else {
            while (singleton->next) {
                backup = singleton->next;
                if (backup->pointer == pointer) {
                    singleton->next = backup->next;
                    _p_free_check(backup, file, line);
                    done.value = pippolo_true;
                    break;
                }
                singleton = singleton->next;
            }
        }
    }
    pippolo_unlock(mutex_memory);
    if (!done.value)
        pippolo_log(ELOG_DEBUGGING, "suspicious of double pippolo_free called at %s (line %d)\n", file, line);
    free(pointer);
}

void _p_free_check (struct str_pointer_node *singleton, const char *file, unsigned int line) {
    struct str_pointer_coords *coords = NULL;
    size_t length = (singleton->size+sizeof(str_pointer_coords))+pippolo_overflow;
    unsigned char *checksum = NULL;
    checksum = (unsigned char *)(((char *)singleton->pointer)+singleton->size);
    if (*((unsigned int *)checksum) != pippolo_checksum) {
        coords = (struct str_pointer_coords *)(checksum+pippolo_overflow);
        pippolo_log(ELOG_DEBUGGING, "mistaken pointer allocated at %s (line %d) and pippolo_free'd at %s (line %d)\n", coords->file, coords->line, file, line);
    }
    filled_memory -= length;
    free(singleton);
}
