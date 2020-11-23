# mpi_scatter_test
=====================

s_v_cols.c:

send vector:
block = m, blocklen = n / commsize (ncols_per_proc), stride = n

recv vector:
block = m, blocklen = n / commsize, stride = n / commsize

resize extent:
n / commsize * sizeof(type)

scatter:
send(and recv)count = 1;

=====================

scatter_vec_ncols.c:

send vector:
block = m, blocklen = 1, stride = n

recv vector:
block = m, blocklen = 1, stride = n / commsize (ncols_per_proc)

resize extent:
sizeof(type)

scatter:
send(and recv)count = n / commsize (ncols_per_proc)

=====================

