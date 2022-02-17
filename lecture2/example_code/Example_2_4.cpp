#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <vector>

using namespace std;

int id, p;

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(time(NULL) + id * 10);

	int tag_num = 1;

	if (id == 0)
	{
		vector<MPI_Request> request;
		vector<int> send_data;

		request.resize(p - 1);
		send_data.resize(p - 1);

		for (int i = 1; i < p; i++)
		{
			send_data[i - 1] = rand();
			MPI_Isend(&send_data[i - 1], 1, MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[i - 1]);			
			cout << send_data[i - 1] << " sent to processor " << i << endl;
			cout.flush();
		}

		MPI_Waitall(p - 1, request.data(), MPI_STATUS_IGNORE);
		//Note the use of request.data() rather than request � I could also use &request[0] instead
	}
	else
	{
		int recv_data;
		MPI_Request request;
		MPI_Irecv(&recv_data, 1, MPI_INT, 0, tag_num, MPI_COMM_WORLD, &request);

		MPI_Wait(&request, MPI_STATUS_IGNORE);

		cout << recv_data << " received on processor " << id << endl;
		cout.flush();
	}
	MPI_Finalize();
}
