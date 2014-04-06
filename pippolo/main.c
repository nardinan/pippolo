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
#include "node.h"
#define pippolo_main_wait_sec 30
#define pippolo_main_wait_usec 0
int main (int argc, char *argv[]) {
	FILE *stream = NULL;
	srand((unsigned int)time(NULL));
	if (argc == 2) {
		p_memory_clean();
		p_log_clean();
		pippolo_log_level = ELOG_NO; /* no output (after startup)! This must be a deamon process */
		p_node_clean();
		p_data_clean();
		if ((stream = fopen(argv[1], "r"))) {
			p_node_setup(stream);
			fclose(stream);
			while (pippolo_true)
				p_wait(pippolo_main_wait_sec, pippolo_main_wait_usec);
		} else
			pippolo_log(ELOG_TALKING, "what ... sorry master, you give me an unreadable file (or it not exists)");
	} else
		pippolo_log(ELOG_TALKING, "please give me a configuration file, master");
	return 0;
}
