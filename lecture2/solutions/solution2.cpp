#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <time.h>

using namespace std;

int id, p;

const int num_send = 10000;

void Do_Work(void)	 //Some (not very useful) work
{
	int sum = 0;
	for (int i = 0; i < 100; i++) sum = sum + 10;
}


int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(time(NULL) + id * 10);

	int tag_num = 1;

	MPI_Request* request = new MPI_Request[(p - 1)*2];
	double **send_data = new double*[p];
	double **recv_data = new double*[p];

    for(int i = 0; i < p; i++)
    {
        send_data[i] = new double[num_send];
        recv_data[i] = new double[num_send];
    }

	int cnt = 0;

	for (int i=0;i<p;i++)
		if (i != id)
		{
			MPI_Irecv(recv_data[i], num_send, MPI_DOUBLE, i, tag_num, MPI_COMM_WORLD, &request[cnt]);
			cnt++;
		}

	for (int i = 0; i<p; i++)
    {
		
        for(int j=0;j<num_send;j++)
            send_data[i][j] = (double)id / (double)p;
        if (i != id)
		{
			MPI_Isend(send_data[i], num_send, MPI_DOUBLE, i, tag_num, MPI_COMM_WORLD, &request[cnt]);
			cnt++;
		}
		else 
        {
            for(int j=0;j<num_send;j++)
            {
                recv_data[i][j] = send_data[i][j];
            }
        }
    }

    int flag;
    int num_cycles = 0;


	while (MPI_Testall(cnt,request,&flag,MPI_STATUS_IGNORE) == MPI_SUCCESS, flag==0)
    {
        Do_Work();
        num_cycles++;
    }

	cout << "Processor " << id << " did " << num_cycles << " cycles of work in the background " << endl;

	delete[] request;
    for(int i = 0; i < p; i++)
    {
        delete[] send_data[i];
        delete[] recv_data[i];
    }
	delete[] recv_data;
	delete[] send_data;

	MPI_Finalize();
}