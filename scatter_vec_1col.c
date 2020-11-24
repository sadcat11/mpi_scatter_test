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
    double *recv_cols = malloc(rcount * sizeof(*recv_cols));

    if (rank == 0) {
	matrix = malloc(count * sizeof(*matrix));
#if 0
	for (int i = 0; i < m; i++) {
	    for (int j = 0; j < n; j++) {
		matrix[i * n + j] = i * n + j;
		printf("%4.0f", matrix[i * n + j]);
	    }
	    printf("\n");
	}
#else
	for (int i = 0; i < count; i++)
	    matrix[i] = i;
#endif

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
    MPI_Datatype vector1, vector_resized1;

    MPI_Type_vector(m, ncols_per_proc, n, MPI_DOUBLE, &vector);
    MPI_Type_commit(&vector);
    //MPI_Type_get_extent(vector, &lb, &extent);
    //printf("vector lb=%d, extent=%d\n", lb, extent);
    MPI_Type_create_resized(vector, 0, ncols_per_proc * sizeof(*matrix), 
                           &vector_resized);
    MPI_Type_commit(&vector_resized);
    //MPI_Type_get_extent(vector_resized, &lb, &extent);
    //printf("vector_resized lb=%d, extent=%d\n", lb, extent);
    MPI_Type_free(&vector);

    MPI_Type_vector(m, ncols_per_proc, ncols_per_proc, MPI_DOUBLE, &vector1);
    MPI_Type_commit(&vector1);
    //MPI_Type_get_extent(vector1, &lb, &extent);
    //printf("vector1 lb=%d, extent=%d\n", lb, extent);
    MPI_Type_create_resized(vector1, 0, ncols_per_proc * sizeof(*recv_cols), 
                           &vector_resized1);
    MPI_Type_commit(&vector_resized1);
    //MPI_Type_get_extent(vector_resized1, &lb, &extent);
    //printf("vector_resized1 lb=%d, extent=%d\n", lb, extent);
    MPI_Type_free(&vector1);


    t = MPI_Wtime();

    MPI_Scatter(matrix, 1, vector_resized, recv_cols, 1, vector_resized1, 0, MCW);
    t = MPI_Wtime() - t;

    MPI_Reduce(&t, &t_g, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);


#if 1
    for (int r = 0; r < commsize; r++)
	if (rank == r) {
	    for (int i = 0; i < m; i++)
		for (int j = 0; j < ncols_per_proc; j++)
		    if (recv_cols[i * ncols_per_proc + j] != 
		       (j + rank * ncols_per_proc + i * n)) {
			printf("rank %d error. real %2.0f. expected %d\n",
			     rank, recv_cols[i * ncols_per_proc + j],
			     j + rank * ncols_per_proc + i * n);
			//perror("incorrect completion of exchange");
			//exit(EXIT_FAILURE);
		    }
	    }
#endif

#if 0
    for (int r = 0; r < commsize; r++)
	if (rank == r) {
	    printf("rank %d\n", rank);
	    for (int i = 0; i < m; i++) {
		for (int j = 0; j < ncols_per_proc; j++) {
		    printf("%3.0f (rank %d)", recv_cols[i * ncols_per_proc + j],
		          rank);
		}
		printf("\n");
	    }
	}
#endif

#if 0
    printf("%d %f\n", rank, t);
    if (rank == 0) {
	printf("\ntime: %f. count: %d. rcount: %d. commsize: %d.\n", t_g, count, rcount, commsize);
	fprintf(f, "%d ", count);
	fprintf(f, "%f\n", t_g);
    }
#endif

    MPI_Type_free(&vector_resized);
    MPI_Type_free(&vector_resized1);
    free(matrix);
    free(recv_cols);

    if (rank == 0 && argc > 3)
	fclose(f);
    MPI_Finalize();
    return 0;
}