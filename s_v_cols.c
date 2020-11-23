#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

#define MCW MPI_COMM_WORLD

int main(int argc, char *argv[])
{
    int rank, commsize, count, rcount;
    double t, t_g;
    //FILE *f;

    MPI_Aint lb, extent;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MCW, &commsize);
    MPI_Comm_rank(MCW, &rank);

    int m = (argc > 1) ? atoi(argv[1]) : 4;
    int n = (argc > 2) ? atoi(argv[2]) : commsize * 2;
/*
    if (argc > 3)
	if (rank == 0)
	    if ((f = fopen("series_vector.txt", "a")) != NULL)
		fprintf(f, "\n=======%s=======\n", argv[3]);
*/
    int ncols_per_proc = n / commsize;
    count = m * n;
    rcount = ncols_per_proc * m;

    double *matrix = NULL;
    double *recv_cols = malloc(rcount * sizeof(double));

    //if (rank == 0) {
	matrix = malloc(count * sizeof(double));

	/*
	for (int i = 0; i < m; i++) {
	    for (int j = 0; j < n; j++) {
		matrix[i * n + j] = i * n + j;
		printf("%4.0f", matrix[i * n + j]);
	    }
	    printf("\n");
	}
	*/

	for (int i = 0; i < count; i++)
	    matrix[i] = i;


	/*
	for (int i = 0; i < ; i++) { //commsize
	    for (int j = 0; j < m; j++) {
		for (int k = 0; k < m; k++) {
		    matrix[i * m + j * m + k] = j * m + j * m + k;
		    printf("%4.0f", matrix[i * m + j * m + k]);
		}
		printf("\n");
	    }
	//    printf("\n");
	}
	*/
	
	
	
	if (matrix == NULL) {
	    fprintf(stderr, "Error: malloc failed\n");
	    MPI_Abort(MCW, 1);
	}
	
    //}

    /*
    double matrix[m][n], recv_cols[m][ncols_per_proc];

    if (rank == 0)
	for (int i = 0; i < m; i++) {
	    for (int j = 0; j < n; j++) {
		matrix[i][j] = i * n + j;
		printf("%4.0f", matrix[i][j]);
	    }
	    printf("\n");
	}
    */

    if (recv_cols == NULL) {
	fprintf(stderr, "Error: malloc failed\n");
	MPI_Abort(MCW, 1);
    }
    
    //MPI_Barrier(MCW);

    MPI_Datatype vector, vector_resized;
    MPI_Datatype vector1, vector_resized1;

    MPI_Type_vector(m, ncols_per_proc, n, MPI_DOUBLE, &vector);
    MPI_Type_commit(&vector);
    MPI_Type_get_extent(vector, &lb, &extent);
    //printf("vector lb=%d, extent=%d\n", lb, extent);
    MPI_Type_create_resized(vector, 0, ncols_per_proc * sizeof(*matrix), &vector_resized);
    MPI_Type_commit(&vector_resized);
    //MPI_Type_get_extent(vector_resized, &lb, &extent);
    //printf("vector_resized lb=%d, extent=%d\n", lb, extent);
    MPI_Type_free(&vector);

    MPI_Type_vector(m, ncols_per_proc, ncols_per_proc, MPI_DOUBLE, &vector1);
    MPI_Type_commit(&vector1);
    //MPI_Type_get_extent(vector1, &lb, &extent);
    //printf("vector1 lb=%d, extent=%d\n", lb, extent);
    MPI_Type_create_resized(vector1, 0, ncols_per_proc * sizeof(*recv_cols), &vector_resized1);
    MPI_Type_commit(&vector_resized1);
    //MPI_Type_get_extent(vector_resized1, &lb, &extent);
    //printf("vector_resized1 lb=%d, extent=%d\n", lb, extent);
    MPI_Type_free(&vector1);


    t = MPI_Wtime();

//    MPI_Scatter(&matrix, ncols_per_proc, vector_resized, &recv_cols,
//               ncols_per_proc, vector_resized, 0, MCW);


    MPI_Scatter(matrix, 1, vector_resized, recv_cols,
               1, vector_resized1, 0, MCW);
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
    
    for (int r = 0; r < commsize; r++)
	if (rank == r) {
	    printf("rank %d\n", rank);
	    for (int i = 0; i < m; i++) {
		for (int j = 0; j < ncols_per_proc; j++) {
		    printf("%3.0f (rank %d)", recv_cols[i * ncols_per_proc + j], rank);
		    //printf("%3.0f (rank %d)", recv_cols[i * commsize][j], rank);
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

    MPI_Type_free(&vector_resized1);
    MPI_Type_free(&vector_resized);
    free(matrix);
    free(recv_cols);


    //MPI_Type_free(&vector1);

    //if (rank == 0)
	//fclose(f);
    MPI_Finalize();
    return 0;
}