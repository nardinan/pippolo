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
#include "endian.h"
int _p_endian_check (void) {
    unsigned int value = 0xAABBCCDD;
    return (*((unsigned char *)&value)==0xAA)?pippolo_big_endian:pippolo_little_endian;
}

void _p_endian_swap (void *data, size_t length) {
    char *pointer = (char *)data, byte;
    size_t tail, head;
    for (head = 0, tail = (length-1); tail > head; head++, tail--) {
        byte = pointer[head];
        pointer[head] = pointer[tail];
        pointer[tail] = byte;
    }
}
