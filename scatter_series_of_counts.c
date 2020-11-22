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

    //count = (argc > 1) ? atoi(argv[1]) : 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MCW, &commsize);
    MPI_Comm_rank(MCW, &rank);
    
    /*
    char name[256];
    int len;
    MPI_Get_processor_name(name, &len);
    */
    
    if (argc != 2) {
	printf("alg = ?\n");
	exit(1);
    }
    
    if (rank == 0)
	if ((f = fopen("series_of_counts.txt", "a")) != NULL)
	    fprintf(f, "\n=======%s=======\n", argv[1]);
    /*
    if (count % commsize != 0) {
	perror("count % commsize != 0");
	exit(EXIT_FAILURE);
    }
    */
    MPI_Barrier(MCW);
    for (int count = commsize * 1024; count <= commsize * 1024 * 1024; count *= 32) {

	rcount = count / commsize;
	double *sbuf = NULL;
	double *rbuf = malloc(sizeof(double) * rcount);
	if (rank == 0) {
	    sbuf = malloc(sizeof(double) * count);
	    for (int i = 0; i < count; i++)
		sbuf[i] = i;
	}

	t = MPI_Wtime();

	MPI_Scatter(sbuf, rcount, MPI_DOUBLE, rbuf, rcount, MPI_DOUBLE, 0, MCW);

	t = MPI_Wtime() - t;

	MPI_Reduce(&t, &t_g, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);


	for (int i = 0; i < rcount; i++)
	    if (rbuf[i] != (rank * rcount + i)) {
		printf("error i %d rbuf[i] %f rank %d\n", i, rbuf[i], rank);
		perror("incorrect completion of exchange");
		exit(EXIT_FAILURE);
	    }

	printf("%d %f\n", rank, t);
	if (rank == 0) {
	    printf("\ntime: %f. count: %d. rcount: %d. commsize: %d.\n", t_g, count, rcount, commsize);
	    fprintf(f, "%d ", count);
	    fprintf(f, "%f\n", t_g);
	}
	
	MPI_Barrier(MCW);
	free(sbuf);
	free(rbuf);

    }
    if (rank == 0)
	fclose(f);
    MPI_Finalize();
    return 0;
}