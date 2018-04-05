#include <math.h>
#include <stdint.h>
#include <mpi.h>
#include "md5.h"
#include "rank0.h"
#include "utils.h"
#include "tag.h"

void send_stop_signal_to_all(int world_size, int * alives) {
	int i;
	int stop_code = 0;
	for (i = 1; i < world_size; i++) {
		if (alives[i] == 1) {
			MPI_Send(&stop_code, 1, MPI_INTEGER, i, TAG_STOP_SIGNAL, MPI_COMM_WORLD);
		}
	}
}

int count_alives(int world_size, int * alives) {
	int count = 0;
	int i;
	for (i = 1; i < world_size; i++) {
		if (alives[i]) {
			count++;
		}
	}
	return count;
}

void rank0(char * hashed_password, int pass_length) {
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	int domain_width = (int)ceil(1.0 * CHAR_MAP_SIZE / world_size);
	int meta_buffer[2]; // [0]: char_width, [1]: start_index
	int * alives = (int *)malloc(world_size * sizeof(int)); // Check process working

	// Send to each other rank
	int i;
	meta_buffer[0] = domain_width;
	for (i = 1; i < world_size; i++) {
		meta_buffer[1] = domain_width * i;
		MPI_Send(meta_buffer, 2, MPI_INTEGER, i, TAG_META_DATA, MPI_COMM_WORLD);
		alives[i] = 1;
	}

	// Handler found password signal
	char * recv_msg = (char *)malloc((pass_length + 1) * sizeof(char));
	MPI_Status found_pass_status;
	int found_pass_flag;
	MPI_Request found_pass_request;
	MPI_Irecv(recv_msg, pass_length + 1, MPI_CHARACTER, MPI_ANY_SOURCE, 
			TAG_FOUND_PASSWORD, MPI_COMM_WORLD, &found_pass_request);

	// Handler terminated signal
	int terminated_code;
	MPI_Status terminated_status;
	int terminated_flag;
	MPI_Request terminated_request;
	MPI_Irecv(&terminated_code, 1, MPI_INTEGER, MPI_ANY_SOURCE, 
			TAG_PROCESS_TERMINATED, MPI_COMM_WORLD, &terminated_request);

	// Init msg for itself
	int start_index = 0;
	char * msg = (char *)malloc((pass_length + 1) * sizeof(char));
	int * msg_indices = (int *)malloc(pass_length * sizeof(int));
	gen_init_msg(msg, msg_indices, start_index, pass_length);

	// Convert hashed password to buffer
	uint32_t hashed_password_buffer[4];
	string_to_md5_buffer(hashed_password, hashed_password_buffer);

	// Buffer storing processing password
	uint32_t pass_buffer[4];
	// Main loop
	do {
		// Stop when out of domain
		if (!check_msg_in_domain(msg, start_index, domain_width)) {
			break;
		}

		// Check if child found password
		MPI_Test(&found_pass_request, &found_pass_flag, &found_pass_status);
		if (found_pass_flag) {
			printf("Receive password: %s\n", recv_msg);
			alives[found_pass_status.MPI_SOURCE] = 0; // found password process ended
			send_stop_signal_to_all(world_size, alives);
			break;
		}

		// Check if child terminated
		if (count_alives(world_size, alives) > 0) {
			MPI_Test(&terminated_request, &terminated_flag, &terminated_status);
			if (terminated_flag) {
				printf("Child %d terminated %d\n", terminated_status.MPI_SOURCE, terminated_flag);
				terminated_flag = 0;
				alives[terminated_status.MPI_SOURCE] = 0;
				// Continue listening
				if (count_alives(world_size, alives) > 0) {
					MPI_Irecv(&terminated_code, 1, MPI_INTEGER, MPI_ANY_SOURCE, 
						TAG_PROCESS_TERMINATED, MPI_COMM_WORLD, &terminated_request);
				}
			}
		}

		// process itself
		md5(msg, pass_buffer);
		if (compare_md5_result(pass_buffer, hashed_password_buffer)) {
			// Found password, stop
			printf("Rank 0 Found password: %s\n", msg);
			send_stop_signal_to_all(world_size, alives);
			break;
		}
	} while (gen_next_str(msg, msg_indices, pass_length));

	// When master finish job, but child not
	while (count_alives(world_size, alives) > 0) {
		// Check if child terminated
		MPI_Test(&terminated_request, &terminated_flag, &terminated_status);
		if (terminated_flag) {
			terminated_flag = 0;
			printf("Child %d terminated\n", terminated_status.MPI_SOURCE);
			alives[terminated_status.MPI_SOURCE] = 0;
			// Continue listening
			if (count_alives(world_size, alives) > 0) {
				MPI_Irecv(&terminated_code, 1, MPI_INTEGER, MPI_ANY_SOURCE, 
					TAG_PROCESS_TERMINATED, MPI_COMM_WORLD, &terminated_request);
			}
		}
	}

	if (!found_pass_flag) {
		printf("Password not found\n");
	}

	// Free
	free(msg);
	free(msg_indices);
	free(recv_msg);
}
