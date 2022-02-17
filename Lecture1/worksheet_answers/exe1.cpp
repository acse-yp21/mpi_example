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
	srand(time(NULL)+id*10);

	int tag_num = 1;

    int* data;
    if(id != 0)
    {
        int source_id = id - 1;
        int dest_id = (id + 1) % p;
        data = new int[id+1];
        MPI_Recv(data, id, MPI_INT, source_id, tag_num, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i=0; i<id;i++)
        {
            cout << data[i] << " ";
        }
        cout << "received on processor " << id << endl;
        
        data[id] = rand();

        MPI_Send(data, id+1, MPI_INT, dest_id, tag_num, MPI_COMM_WORLD);
        
        for(int i=0; i<id+1;i++)
        {
            cout << data[i] << " ";
        }
        cout << "sent to processor " << dest_id << endl;
    }
    else
    {
        data = new int[1];
        data[0] = rand();

        MPI_Send(data, 1, MPI_INT, 1, tag_num, MPI_COMM_WORLD);
        cout << data[0] << " sent to processor 1" << endl;

        delete[] data;
        data = new int[p];
        MPI_Recv(data, p, MPI_INT, p-1, tag_num, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i=0; i<p;i++)
        {
            cout << data[i] << " ";
        }
        cout << "received on processor 0" << endl;     
    }
    
    delete[] data;
	MPI_Finalize();
}