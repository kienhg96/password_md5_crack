#include <math.h>
#include <stdint.h>
#include <mpi.h>
#include "md5.h"
#include "ranki.h"
#include "utils.h"
#include "tag.h"

void ranki(char * hashed_password, int pass_length) {
	int meta_buffer[2];
	MPI_Status meta_status;

	// Receive meta data from rank0
	MPI_Recv(meta_buffer, 2, MPI_INTEGER, 0, TAG_META_DATA, MPI_COMM_WORLD, &meta_status);
	int domain_width = meta_buffer[0];
	int start_index = meta_buffer[1];

	// printf("Receive meta_buffer %d %d\n", domain_width, start_index);

	// Get rank
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	// Handle stop signal
	int stop_status_code;
	int stop_flag;
	MPI_Status stop_status;
	MPI_Request stop_request;
	MPI_Irecv(&stop_status_code, 1, MPI_INTEGER, 0, TAG_STOP_SIGNAL, MPI_COMM_WORLD, &stop_request);

	// Init msg
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

		// Check stop signal
		MPI_Test(&stop_request, &stop_flag, &stop_status);
		if (stop_flag) {
			printf("Rank %d Request stop with code %d\n", rank, stop_status_code);
			break;
		}

		// process itself
		md5(msg, pass_buffer);
		if (compare_md5_result(pass_buffer, hashed_password_buffer)) {
			// Found password, stop
			printf("Rank %d Found password: %s\n", rank, msg);

			// Send to master
			MPI_Send(msg, pass_length + 1, MPI_CHARACTER, 0, TAG_FOUND_PASSWORD, MPI_COMM_WORLD);
			break;
		}
	} while (gen_next_str(msg, msg_indices, pass_length));

	int terminated_code = 0;
	MPI_Send(&terminated_code, 1, MPI_INTEGER, 0, TAG_PROCESS_TERMINATED, MPI_COMM_WORLD);

	// Free
	free(msg);
	free(msg_indices);
}
