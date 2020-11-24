#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

#define MCW MPI_COMM_WORLD

int main(int argc, char *argv[])
{
    int rank, commsize, count, rcount;
    double t, t_g;
    
    //rcount = (argc > 1) ? atoi(argv[1]) : 1;
    count = (argc > 1) ? atoi(argv[1]) : 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MCW, &commsize);
    MPI_Comm_rank(MCW, &rank);
    char name[256];
    int len;
    MPI_Get_processor_name(name, &len);
    
    if (count % commsize != 0) {
	perror("count % commsize != 0");
	exit(EXIT_FAILURE);
    }
    
    rcount = count / commsize;
    //count = rcount * commsize;
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
	    printf("error i %d rbuf[i] %f rank %d name %s\n", i, rbuf[i], rank, name);
	    perror("incorrect completion of exchange");
	    exit(EXIT_FAILURE);
	}

    //printf("%d %f %f\n", rank, t, rbuf[0]);
    printf("%d %f\n", rank, t);
    if (rank == 0) {
	printf("\ntime: %f. count: %d. rcount: %d. commsize: %d.\n", t_g, count, rcount, commsize);
	FILE *f;
	if (argc > 1) {
	    char *name = argv[1];
	    strcat(name, "_count.txt");
	    if ((f = fopen(name, "a")) != NULL)
		fprintf(f, "%f\n", t_g);
	}
    }

    free(sbuf);
    free(rbuf);
    MPI_Finalize();
    return 0;
}