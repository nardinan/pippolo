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
#include "p_api_definitions.h"
#define pippolo_malloc(a) malloc(a)
#define pippolo_realloc(a,b) realloc((a),(b))
#define pippolo_free(a) free(a)
#define pippolo_null_free(a)\
	do{\
		pippolo_free(a);\
		(a)=NULL;\
	}while(0)
#endif
