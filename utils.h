#ifndef ___ST_UTILS_H_
#define ___ST_UTILS_H_
#define CHAR_MAP_SIZE 26

extern char * CHAR_MAP;

char * gen_init_msg(char * msg, int * msg_indices, int start_index, int pass_length);
int gen_next_str(char * msg, int * msg_indices, size_t len);
int check_msg_in_domain(char * msg, int start_index, int pass_length);

#endif
