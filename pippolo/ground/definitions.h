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
#ifndef pippolo_definitions_h
#define pippolo_definitions_h
#define pippolo_false 0
#define pippolo_true 1
#define pippolo_hash_elements 10
#define pippolo_hash_cover "0123456789"
#define pippolo_version "0.15a"
typedef struct pippolo_bool {
	int value:1;
} pippolo_bool;
void p_wait (int sec, int usec);
#define pippolo_lock(a)\
	pthread_mutex_lock(&(a))
#define pippolo_lock_action(a,action)\
	do{\
		pippolo_lock(a);\
		(action);\
		pippolo_unlock(a);\
	}while(0)
#define pippolo_unlock(a)\
	pthread_mutex_unlock(&(a))
#define pippolo_trylock(a)\
	pthread_mutex_trylock(&(a))
#define p_max(a,b)\
	(((a)>(b))?a:b)
#define p_min(a,b)\
	(((a)<(b))?a:b)
#endif
