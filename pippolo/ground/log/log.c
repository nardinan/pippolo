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
#include "log.h"
char log_description[][16] = {
	"silent",
	"talking",
	"standard",
	"debugging",
	"deep debugging",
	"communications"
};
pthread_mutex_t mutex_log;
enum enum_log_level pippolo_log_level = ELOG_TALKING;
void p_log_clean (void) {
	pthread_mutex_init(&mutex_log, NULL);
}
