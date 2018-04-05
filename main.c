#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "rank0.h"
#include "ranki.h"
#include "mpi.h"

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if (argc < 3) {
		if (rank == 0) {
			printf("Usage: crack <password length> <hashed password>\n");
		}
		MPI_Finalize();
		return 0;
	}

	int pass_length = (int)strtol(argv[1], 0, 10);
	char * hashed_password = argv[2];

	if (rank == 0) {
		rank0(hashed_password, pass_length);
	} else {
		ranki(hashed_password, pass_length);
	}

	MPI_Finalize();
	return 0;
}
