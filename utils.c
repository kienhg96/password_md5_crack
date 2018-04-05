#include <stdio.h>
#include <stdint.h>
#include "utils.h"

char * CHAR_MAP = "abcdefghijklmnopqrstuvwxyz";

int gen_next_str(char * msg, int * msg_indices, size_t len) {
	int i = len - 1;
	while (i >= 0) {
		if (msg_indices[i] == CHAR_MAP_SIZE - 1) {
			msg_indices[i] = 0;
			msg[i] = CHAR_MAP[msg_indices[i]];
			i--;
		} else {
			msg_indices[i]++;
			msg[i] = CHAR_MAP[msg_indices[i]];
			if ((len - i) == 5) {
				printf("Header: %s\n", msg);			
			}	
			return 1;
		}
	}
	return 0;
}

char * gen_init_msg(char * msg, int * msg_indices, int start_index, int pass_length) {
	// Gen msg
	int i;
	msg[0] = CHAR_MAP[start_index];
	msg_indices[0] = start_index;

	for (i = 1; i < pass_length; i++) {
		msg[i] = CHAR_MAP[0];
		msg_indices[i] = 0;
	}
	
	msg[pass_length] = 0;
	return msg;
}

int check_msg_in_domain(char * msg, int start_index, int domain_width) {
	if (msg[0] == CHAR_MAP[start_index + domain_width]) {
		return 0;
	}
	return 1;
}