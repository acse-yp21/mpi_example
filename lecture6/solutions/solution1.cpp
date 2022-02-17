#include <mpi.h>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <vector>

using namespace std;

int id, p, tag_num = 1;

const int imax = 400;
const int jmax = 300;
const int kmax = 500;


class CMatrix
{
public: 
    CMatrix()
    {
        data_1D = nullptr;
        data_2D = nullptr;
    }

    CMatrix(int n_i, int n_j)
    {
        allocate_data(n_i, n_j);
    }
    ~CMatrix()
    {
        delete[] data_1D;
        delete[] data_2D;
    }
    double *data_1D;
    double **data_2D;
    int max_i,max_j;

    void allocate_data(int n_i, int n_j); 
    void fill_random(double max);

    void send_data(int row_start, int row_end, int dest, MPI_Request *request); //inclusive, row_end will be sent
    void recv_data(int row_start, int row_end, int source, MPI_Request *request); //inclusive, row_end will be sent
};

int* start_row;
int* num_rows;

void CMatrix::allocate_data(int n_i, int n_j)
{
    max_i = n_i;
    max_j = n_j;
    data_1D = new double[max_i*max_j];
    data_2D = new double* [max_i];

    for(int i=0;i<max_i;i++)
    {
        data_2D[i] = &data_1D[i*max_j];
    }
}

void CMatrix::fill_random(double max)
{
    for(int i=0;i<max_i;i++)
    {
        for(int j=0;j<max_j;j++)
        {
            data_2D[i][j] = (double)rand() / RAND_MAX * max;
        }
    }
}


void CMatrix::send_data(int row_start, int row_end, int dest, MPI_Request *request)
{
    MPI_Isend(data_2D[row_start],(row_end-row_start+1)*max_j,MPI_DOUBLE,dest,tag_num,MPI_COMM_WORLD, request);
}


void CMatrix::recv_data(int row_start, int row_end, int source, MPI_Request *request)
{
    MPI_Irecv(data_2D[row_start],(row_end-row_start+1)*max_j,MPI_DOUBLE,source,tag_num,MPI_COMM_WORLD, request);
}


CMatrix A, B, C;    //A = B*C

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(time(NULL) + id * 10);

    start_row = new int[p];
    num_rows = new int[p];

    int start = 0;
    int tot_rows = imax;

    for(int i=0;i<p;i++)
    {
        num_rows[i] = tot_rows / (p-i); //evenly allocate
        start_row[i] = start;
        start += num_rows[i];
        tot_rows -= num_rows[i];
    }

    MPI_Request* request_list_send = nullptr;
    MPI_Request* request_list_recv = nullptr;
    int cnt_recv = 0, cnt_send = 0;

    if(id == 0)
    {
        A.allocate_data(imax,jmax);
        B.allocate_data(imax,kmax);
        C.allocate_data(kmax,jmax);

        B.fill_random(10.0);
        C.fill_random(10.0);

        request_list_send = new MPI_Request[(p-1)*2];
        request_list_recv = new MPI_Request[p-1];

        for(int i=1;i<p;i++)
        {
            B.send_data(start_row[i],start_row[i]+num_rows[i]-1,i,&request_list_send[cnt_send]);
            cnt_send++;

            C.send_data(0,kmax-1,i,&request_list_send[cnt_send]); // send all C
            cnt_send++;

            A.recv_data(start_row[i],start_row[i]+num_rows[i]-1,i,&request_list_recv[cnt_recv]);
            cnt_recv++;
        }

        for(int i=0;i<num_rows[i];i++)
        {
            for(int j=0;j<jmax;j++)
            {
                A.data_2D[i][j] = 0.0;
                for(int k=0;k<kmax;k++)
                {
                    A.data_2D[i][j] += B.data_2D[i][k] * C.data_2D[k][j];
                }
            }
        }

        MPI_Waitall(cnt_recv, request_list_recv,MPI_STATUSES_IGNORE);


    }
    else
    {
        A.allocate_data(num_rows[id],jmax);
        B.allocate_data(num_rows[id],kmax);
        C.allocate_data(kmax,jmax);

        request_list_send = new MPI_Request[1];
        request_list_recv = new MPI_Request[2];


        B.recv_data(0,num_rows[id]-1,0,&request_list_recv[cnt_recv]);
        cnt_recv++;

        C.recv_data(0,kmax-1,0,&request_list_recv[cnt_recv]); // send all C
        cnt_recv++;

        MPI_Waitall(cnt_recv, request_list_recv,MPI_STATUSES_IGNORE);

        for(int i=0;i<num_rows[id];i++)
        {
            for(int j=0;j<jmax;j++)
            {
                A.data_2D[i][j] = 0.0;
                for(int k=0;k<kmax;k++)
                {
                    A.data_2D[i][j] += B.data_2D[i][k] * C.data_2D[k][j];
                }
            }
        }

        A.send_data(0,num_rows[id]-1,0,&request_list_recv[cnt_send]);
        cnt_send++;
        MPI_Wait(request_list_send,MPI_STATUSES_IGNORE);

    }

    delete[] request_list_send;
    delete[] request_list_recv;

	MPI_Finalize();
}