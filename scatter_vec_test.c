#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

#define MCW MPI_COMM_WORLD

int main(int argc, char *argv[])
{
    int rank, commsize, count, rcount;
    double t, t_g;
    FILE *f;

    MPI_Aint lb, extent;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MCW, &commsize);
    MPI_Comm_rank(MCW, &rank);

    int m = (argc > 1) ? atoi(argv[1]) : 4;
    int n = (argc > 2) ? atoi(argv[2]) : commsize * 2;

    if (n % commsize != 0) {
	perror("n % commsize != 0");
	exit(EXIT_FAILURE);
    }

    if (rank == 0 && argc > 3)
	if ((f = fopen("series_vector.txt", "a")) != NULL)
	    fprintf(f, "\n=======%s=======\n", argv[3]);

    int ncols_per_proc = n / commsize;
    count = m * n;
    rcount = ncols_per_proc * m;

    double *matrix = NULL;
    double *recv_cols = malloc(count * sizeof(*recv_cols));
    if (rank == 0) {
	matrix = malloc(count * sizeof(*matrix));
	for (int i = 0; i < m; i++) {
	    for (int j = 0; j < n; j++) {
		matrix[i * n + j] = i * n + j;
		printf("%4.0f", matrix[i * n + j]);
	    }
	    printf("\n");
	}
	
	if (matrix == NULL) {
	    fprintf(stderr, "Error: malloc failed\n");
	    MPI_Abort(MCW, 1);
	}
    }

    if (recv_cols == NULL) {
	fprintf(stderr, "Error: malloc failed\n");
	MPI_Abort(MCW, 1);
    }

    MPI_Datatype vector, vector_resized;

    MPI_Type_vector(m, 1, n, MPI_DOUBLE, &vector);
    MPI_Type_commit(&vector);
    //MPI_Type_get_extent(vector, &lb, &extent);
    //printf("vector lb=%d, extent=%d\n", lb, extent);
    MPI_Type_create_resized(vector, 0, sizeof(*matrix), &vector_resized);
    MPI_Type_commit(&vector_resized);
    //MPI_Type_get_extent(vector_resized, &lb, &extent);
    //printf("vector_resized lb=%d, extent=%d\n", lb, extent);
    MPI_Type_free(&vector);

    t = MPI_Wtime();

    MPI_Scatter(matrix, ncols_per_proc, vector_resized, recv_cols,
               ncols_per_proc, vector_resized, 0, MCW);
    t = MPI_Wtime() - t;

    MPI_Reduce(&t, &t_g, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);


    /*
    for (int i = 0; i < rcount; i++)
	if (recv_cols[i] != (rank * rcount + i)) {
	    printf("error i %d expected %d rbuf[i] %f rank %d\n", i, rank * rcount + i,
		  recv_cols[i], rank);
	    perror("incorrect completion of exchange");
	    exit(EXIT_FAILURE);
	}
    */
    
    //printf("m %d, n %d, ncols_per_proc %d, rcount = %d\n", m, n, ncols_per_proc, rcount);
    for (int r = 0; r < commsize; r++)
	if (rank == r) {
	    printf("rank %d\n", rank);
	    for (int i = 0; i < m; i++) {
		for (int j = 0; j < ncols_per_proc; j++) {
		    printf("%3.0f (rank %d)  ", recv_cols[i * n + j], rank);
		}
		printf("\n");
	    }
	}
    
/*
    printf("%d %f\n", rank, t);
    if (rank == 0) {
	printf("\ntime: %f. count: %d. rcount: %d. commsize: %d.\n", t_g, count, rcount, commsize);
	fprintf(f, "%d ", count);
	fprintf(f, "%f\n", t_g);
    }
*/
    MPI_Type_free(&vector_resized);
    if (matrix != NULL)
	free(matrix);
    if (recv_cols != NULL)
	free(recv_cols);

    if (rank == 0 && argc > 3)
	fclose(f);
    MPI_Finalize();
    return 0;
}