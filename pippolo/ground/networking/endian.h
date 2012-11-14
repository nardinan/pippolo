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
#ifndef pippolo_endian_h
#define pippolo_endian_h
#include <string.h>
#define pippolo_big_endian 1
#define pippolo_little_endian 0
#define pippolo_default_endian pippolo_big_endian
#define pippolo_swap(val) _p_endian_swap(&val,sizeof(val))
extern int _p_endian_check (void);
extern void _p_endian_swap (void *data, size_t length);
#endif
