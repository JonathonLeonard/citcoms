# Process this file with Pyrex to produce mpi.c


cimport cmpi


cdef extern from "stdlib.h":
    void *malloc(int)
    void free(void *)


cdef class MPI_Comm:

    cdef cmpi.MPI_Comm comm

    def __init__(MPI_Comm self):
        self.comm = cmpi.MPI_COMM_WORLD


MPI_COMM_WORLD = MPI_Comm()


class MPI_Error(EnvironmentError):
    pass


def MPI_Init(argv):
    cdef int error, cargc, i
    cdef char **cargv, **mycargv
    myargv = []

    # Construct a C char** argument vector from 'argv'.
    cargc = len(argv)
    cargv = <char **>malloc((cargc + 1) * sizeof(char *))
    for i from 0 <= i < cargc:
        arg = argv[i]
        myargv.append(arg) # add ref
        cargv[i] = arg
    cargv[cargc] = NULL

    # Call MPI_Init().
    mycargv = cargv; # MPI might allocate & return its own.
    error = cmpi.MPI_Init(&cargc, &cargv)
    if error != cmpi.MPI_SUCCESS:
        free(mycargv)
        raise MPI_Error, error

    # Reconstruct Python's 'argv' from the modified 'cargv'.
    del argv[:]
    for i from 0 <= i < cargc:
        argv.append(cargv[i])
    free(mycargv)
    
    return


def MPI_Finalize():
    cdef int error
    error = cmpi.MPI_Finalize()
    if error != cmpi.MPI_SUCCESS:
        raise MPI_Error, error
    return


def MPI_Comm_rank(comm):
    cdef int error
    cdef int rank
    cdef MPI_Comm c_comm
    c_comm = comm
    error = cmpi.MPI_Comm_rank(c_comm.comm, &rank)
    if error != cmpi.MPI_SUCCESS:
        raise MPI_Error, error
    return rank
    

# end of file