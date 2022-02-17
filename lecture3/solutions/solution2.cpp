#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <time.h>

using namespace std;

int id, p;


int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(time(NULL) + id * 1257);

	double id_rand = (double)rand() /RAND_MAX *100.0;
    cout << "Process " << id << " produced number " << id_rand << endl;

    double min_rand;

    MPI_Reduce(&id_rand,&min_rand,1,MPI_DOUBLE,MPI_MIN,0,MPI_COMM_WORLD);

    cout << id << ": " << min_rand << endl;

    MPI_Bcast(&min_rand,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

    cout << id << ": " << min_rand << endl;
	MPI_Finalize();
}