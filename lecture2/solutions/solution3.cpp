#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <cmath>
#include <vector>
using namespace std;

int id, p;

int imax, jmax;
int id_i, id_j;
vector<int> neighbor_ids;
const bool periodic = true;
int tag_num = 1;


void find_domain(void)
{
	imax = 1;
    for(int i = round(sqrt(p));i >= 1;i--)
    {
        if(p%i==0)
        {
            imax = i;
            break;
        }
    }
    jmax = p / imax;

    if(id == 0)
    {
        cout << "domain: " << imax << " " << jmax << endl;
    }
}

void find_neighbors(void)
{
    id_i = id%imax;
    id_j = id/imax;

    for(int i = -1; i<=1; i++)
    {
        for(int j = -1; j<=1;j++)
        {
            if(periodic)
            {
                int neigh_i = (id_i + i + imax) % imax, neigh_j = (id_j + j + jmax) % jmax;
                int neigh_id = neigh_i + neigh_j * imax;
                if(neigh_id != id)
                {
                    neighbor_ids.push_back(neigh_id);
                }
            }
            else
            {
                int neigh_i = id_i + i, neigh_j = id_j + j;

                if(neigh_i>=0 && neigh_i <imax && neigh_j>=0 && neigh_j<jmax)
                {
                    int neigh_id = neigh_i + neigh_j * imax;
                    if(neigh_id != id)
                    {
                        neighbor_ids.push_back(neigh_id);
                    }
                }
            }
            
        }
    }

    cout << "neigbor list for process" << id << ": ";
    for(int neigh_id:neighbor_ids)
        cout << "\t" << neigh_id;
    cout << endl;
}

void do_communication(void)
{
    int *data_to_send = new int[neighbor_ids.size()];
    int *data_to_recv = new int[neighbor_ids.size()];

    tag_num++;
    for(int i=0; i<neighbor_ids.size();i++)
    {
        data_to_send[i] = neighbor_ids[i];
    }


    MPI_Request *request = new MPI_Request[neighbor_ids.size()*2];
    int cnt = 0;

    for(int i=0; i <neighbor_ids.size();i++)
    {
        MPI_Irecv(&data_to_recv[i],1,MPI_INT,neighbor_ids[i],tag_num,MPI_COMM_WORLD,&request[cnt]);
        cnt++;
        MPI_Isend(&data_to_send[i],1,MPI_INT,neighbor_ids[i],tag_num,MPI_COMM_WORLD,&request[cnt]);
        cnt++;

    }

    MPI_Waitall(cnt,request,MPI_STATUSES_IGNORE);

    bool test = false;
    for(int i=0; i <neighbor_ids.size();i++)
    {
        if(data_to_recv[i] != id)
        {
            test=true;
            break;
        }
    }
    if(test)
        cout << "COMM ERROR" << id << endl;
    else
        cout << "Comm on " << id << " went correctly" << endl;

    delete[] request;
    delete[] data_to_send;
    delete[] data_to_recv;
    
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(time(NULL) + id * 10);

	

	find_domain();
    find_neighbors();
    do_communication();

	MPI_Finalize();
}