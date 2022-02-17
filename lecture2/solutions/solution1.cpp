#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <vector>

using namespace std;

int id, p;
vector<int> send_list, receive_list;
MPI_Request* request_list;
int tag_num = 1;

void setup_comms(void)
{
    bool *send_bool_list = new bool[p];
    bool *recv_bool_list = new bool[p];
    request_list = new MPI_Request[(p - 1)*2];
    for(int i = 0; i < p; i++)
    {
        send_bool_list[i] = false;
    }

    for(int i = 0; i < 2; i++)
    {
        int proc;
        do
        {
            proc = rand() % p;
        } while (proc == id || send_bool_list[proc]);
        
        send_list.push_back(proc);
        send_bool_list[proc] = true;
    }

    int cnt = 0;
    for(int i = 0; i < p; i++)
    {
        if(i != id)
        {
            MPI_Irecv(&recv_bool_list[i],1,MPI_C_BOOL,i, tag_num,MPI_COMM_WORLD, &request_list[cnt]);
            cnt++;
            MPI_Isend(&send_bool_list[i],1,MPI_C_BOOL,i, tag_num,MPI_COMM_WORLD, &request_list[cnt]);
            cnt++;
        }
        else
        {
            recv_bool_list[i] = false;
        }
        
    }

    MPI_Waitall(cnt,request_list,MPI_STATUSES_IGNORE);

    for(int i = 0; i < p; i++)
    {
        if(recv_bool_list[i])
            receive_list.push_back(i);
        
    }

    cout << "Process" << id << ": sending to";
    for(int send_id: send_list)
        cout << "\t" << send_id;
    cout << endl;

    cout << "Process" << id << ": receiving from";
    for(int recv_id: receive_list)
        cout << "\t" << recv_id;
    cout << endl;

    delete[] recv_bool_list;
    delete[] send_bool_list;
    delete[] request_list;
}


void communicate(void)
{

    vector<double> data_to_send(send_list.size());
    vector<double> data_to_recv(receive_list.size());

    for(int i = 0; i < send_list.size(); i++)
    {
        data_to_send[i] = (double)rand() / RAND_MAX;
    }

    for(int i = 0; i < receive_list.size(); i++)
    {
        data_to_recv[i] = (double)rand() / RAND_MAX;
    }

    tag_num++;
    request_list = new MPI_Request[send_list.size() + receive_list.size()];
    int cnt = 0;

    for(int i = 0; i < send_list.size(); i++)
    {
        MPI_Isend(&data_to_send[i],1,MPI_DOUBLE,send_list[i],tag_num,MPI_COMM_WORLD, &request_list[cnt]);
        cnt++;
    }

    for(int i = 0; i < receive_list.size(); i++)
    {
        MPI_Irecv(&data_to_recv[i],1,MPI_DOUBLE,receive_list[i],tag_num,MPI_COMM_WORLD, &request_list[cnt]);
        cnt++;
    }

    MPI_Waitall(cnt,request_list,MPI_STATUSES_IGNORE);

    delete[] request_list;
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(time(NULL) + id * 10);

	setup_comms();


    for(int i = 0; i<10;i++)
    {
        if(id == 0)
            cout << "Doing communication cycle" << i << endl;
        communicate();
    }
	MPI_Finalize();
}