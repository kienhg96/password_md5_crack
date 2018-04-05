#ifndef __MD5__
#define __MD5__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint32_t * md5(uint8_t *initial_msg, uint32_t * result);
uint8_t compare_md5_result(uint32_t * src, uint32_t * dst);
uint32_t * string_to_md5_buffer(char * str, uint32_t * result);

#endif