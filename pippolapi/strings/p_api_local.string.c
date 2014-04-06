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
#include "p_api_local.string.h"
void p_string_trim (char *string) {
	char *pointer[2] = {string, NULL};
	pointer[1] = (pointer[0]+p_string_len(pointer[0]))-1;
	while (((spacechar(*(pointer[0]))) || (finalchar(*(pointer[1]))) || (spacechar(*(pointer[1])))) && (pointer[0] <= pointer[1])) {
		if (spacechar(*(pointer[0])))
			pointer[0]++;
		if ((finalchar(*(pointer[1]))) || (spacechar(*(pointer[1])))) {
			*(pointer[1]) = '\0';
			pointer[1]--;
		}
	}
	if (pointer[0] > string)
		memmove(string, pointer[0], (p_string_len(pointer[0])+1));
}
