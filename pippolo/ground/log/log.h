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
#ifndef pippolo_log_h
#define pippolo_log_h
#include "memory.h"
#include "definitions.h"
#define pippolo_log(lvl,str,...)\
	do{\
		if(lvl<=pippolo_log_level){\
			pthread_mutex_lock(&mutex_log);\
			switch(lvl){\
				case ELOG_DEBUGGING:fprintf(stdout,"[pippolo::debug] ");break;\
				case ELOG_DEEP_DEBUGGING:fprintf(stdout,"[pippolo::debug] (%s|%d) ",__FILE__,__LINE__);break;\
				case ELOG_COMMUNICATIONS:fprintf(stdout,"[pippolo::comm] ");break;\
				default:\
					break;\
			}\
			fprintf(stdout,str,##__VA_ARGS__);\
			fprintf(stdout,"\n");\
			fflush(stdout);\
			pthread_mutex_unlock(&mutex_log);\
		}\
	}while(0)
enum enum_log_level {
	ELOG_NO=0,
	ELOG_TALKING,
	ELOG_STANDARD,
	ELOG_DEBUGGING,
	ELOG_DEEP_DEBUGGING,
	ELOG_COMMUNICATIONS,
	ELOG_NULL
};
extern char log_description[][16];
extern pthread_mutex_t mutex_log;
extern enum enum_log_level pippolo_log_level;
void p_log_clean (void);
#endif
